#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>

#include <stdio.h>

#define CHAIRS 5
#define TOTAL_CLIENTS 100

#define CLIENT_CADENCE 4
#define BARBER_SPEED 2

// Metrics
int clients = 0, lost_clients = 0, attended_clients = 0, total_cuts_recieved = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t barber_ready;
sem_t free_seats;
pthread_mutex_t accessing_free_seats = PTHREAD_MUTEX_INITIALIZER;
sem_t clients_waiting;
sem_t barber_asleep;
pthread_cond_t barber_sleep = PTHREAD_COND_INITIALIZER;
pthread_mutex_t sleep_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t checking_barber = PTHREAD_MUTEX_INITIALIZER;


pthread_mutex_t step_sync_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t step_sync_cond = PTHREAD_COND_INITIALIZER;
sem_t step_sync_sem;

void recibir_pago() {
  sem_post(&step_sync_sem);
  pthread_cond_wait(&step_sync_cond, &step_sync_mutex);
  puts("$b: Barber got paid.");
}

void pagar() {
  sem_wait(&step_sync_sem);
  pthread_cond_signal(&step_sync_cond);
  puts("$c: Client paid.");
}

void me_cortan(int client_n)
{
  sem_wait(&step_sync_sem);
  pthread_cond_signal(&step_sync_cond);
  printf("-- %d got cut. Cut n %d\n", client_n, ++total_cuts_recieved);
}

void cortando(int signal_n)
{
  sleep(rand() % BARBER_SPEED);
  sem_post(&step_sync_sem);
  pthread_cond_wait(&step_sync_cond, &step_sync_mutex);
  printf("Finished cut n %d\n", signal_n);
}

void *barber(void *arg)
{
  int total_clients = 0, waiting_clients = 0, available_seats = 0, signal_n = 0;
  puts("\t{Barber start}\n");
  int attended;
  sem_post(&barber_ready);
  while (total_clients < TOTAL_CLIENTS || waiting_clients > 0)
  // while(true)
  {
    // Si no hay clientes dormimos
    int got_client = sem_trywait(&clients_waiting);
    if (got_client == -1) {
      sem_post(&barber_asleep);
      // Esperamos que nos despierten
      pthread_cond_wait(&barber_sleep, &sleep_mutex);
      continue;
    }

    // Nos marcamos como listos y liberamos un asiento
    pthread_mutex_lock(&accessing_free_seats);
    sem_post(&barber_ready);
    sem_post(&free_seats);
    pthread_mutex_unlock(&accessing_free_seats);

    // Cortamos el pelo al cliente
    cortando(++signal_n);

    // Recibimos el pago
    recibir_pago();

    // Metricas
    attended++;
    pthread_mutex_lock(&clients_mutex);
    total_clients = clients;
    sem_getvalue(&clients_waiting, &waiting_clients);
    sem_getvalue(&free_seats, &available_seats);
    printf("\t\twait val: %d -- free val: %d\n", waiting_clients, available_seats);
    pthread_mutex_unlock(&clients_mutex);
    printf("\t\t\tCuts given: %d, recieved: %d\n", signal_n, total_cuts_recieved);
  }
  printf("\t{Barber finish}\nAttended %d clients.\n", attended);
  printf("Lost clients: %d\nAttended clients: %d\n", lost_clients, attended_clients);
  return NULL;
}

void *client(void *arg)
{
  int num = arg - (void *)0;
  printf("\t{Thread %d start}\n", num);
  while (true)
  {
    sleep(rand() % CLIENT_CADENCE);

    // Metricas de clientes totales para auto terminacion
    pthread_mutex_lock(&clients_mutex);
    int client_n = clients++;
    if (client_n >= TOTAL_CLIENTS) {
      pthread_mutex_unlock(&clients_mutex);
      break;
    }
    pthread_mutex_unlock(&clients_mutex);

    // Nos fijamos si hay asientos disponibles
    pthread_mutex_lock(&accessing_free_seats);
    int got_a_seat = sem_trywait(&free_seats);
    if (got_a_seat == -1) {
      // Si no los hay nos retiramos
      lost_clients++;
      printf("(-) %d: lost client %d\n", num, client_n);
      pthread_mutex_unlock(&accessing_free_seats);
      continue;
    }
    // En caso que haya un asiento, nos ponemos en espera
    sem_post(&clients_waiting);

    attended_clients++;
    printf("(+) %d: client %d.\n", num, client_n);
    printf("%d: client %d posted.\n", num, client_n);
    pthread_mutex_unlock(&accessing_free_seats);

    // Si el barbero duerme, lo despertamos
    pthread_mutex_lock(&checking_barber);
    int barber_awake = sem_trywait(&barber_asleep);
    if (barber_awake != -1) {
      printf("%d: client %d woke up barber.\n", num, client_n);
      pthread_cond_signal(&barber_sleep);
    }
    pthread_mutex_unlock(&checking_barber);

    printf("%d: client %d waiting.\n", num, client_n);
    // Esperamos que el barbero este listo y nos llame
    sem_wait(&barber_ready);

    // Recibimos el corte
    me_cortan(client_n);

    // Pagamos
    pagar();
  }
  printf("\t{Thread %d finished}\n", num);
  return NULL;
}

int main()
{
  int i;
  pthread_t t_barber, t_clients[CHAIRS * 2];
  
  sem_init(&barber_ready, 0, 1);
  sem_init(&barber_asleep, 0, 0);
  sem_init(&free_seats, 0, CHAIRS);
  sem_init(&clients_waiting, 0, 0);
  sem_init(&step_sync_sem, 0, 0);

  pthread_create(&t_barber, NULL, barber, (void *)0);
  for (i = 0; i < CHAIRS * 2; i++)
    pthread_create(&t_clients[i], NULL, client, i + (void *)0);

  pthread_join(t_barber, NULL);
  for (i = 0; i < CHAIRS * 2; i++)
    pthread_join(t_clients[i], NULL);

  return 0;
}