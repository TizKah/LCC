#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define true 1

/* struct sigaction {
               void     (*sa_handler)(int);
               void     (*sa_sigaction)(int, siginfo_t *, void *);
               sigset_t   sa_mask;
               int        sa_flags;
               void     (*sa_restorer)(void);
};
 */

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
    struct sigaction child_action;
    child_action.sa_handler = child_sigusr1_handler;
    sigemptyset(&child_action.sa_mask);
    child_action.sa_flags = 0;
    sigaction(SIGUSR1, &child_action, NULL);

    pause();
    exit(0);
  } else {
    struct sigaction parent_action;
    parent_action.sa_handler = parent_sigusr1_handler;
    sigemptyset(&parent_action.sa_mask);
    parent_action.sa_flags = 0;
    sigaction(SIGUSR1, &parent_action, NULL);

    sleep(1);
    kill(child_pid, SIGUSR1);
    pause();
  }
  wait(NULL);
   
  return 0;
}