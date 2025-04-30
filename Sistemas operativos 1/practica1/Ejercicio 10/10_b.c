#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define true 1

int parent_pid, child_pid;

void parent_sigusr1_handler() {
  printf("Parent recieved SIGUSR1\n");
  kill(child_pid, SIGUSR1);
}

void child_sigusr1_handler() {
  printf("Child recieved SIGUSR1\n");
  kill(parent_pid, SIGUSR1);
}

int main(int argc, char *argv[]) {
  parent_pid = getpid();
  child_pid = fork();

  if (child_pid == 0) {
    signal(SIGUSR1, child_sigusr1_handler);
    pause();
    exit(0);
  } else {
    signal(SIGUSR1, parent_sigusr1_handler);
    sleep(1);
    kill(child_pid, SIGUSR1);
    pause();
  }
  wait(NULL);
   
  return 0;
}