#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int execute_command_in_fork(char** command, int read_pipe, int write_pipe, int command_args);
void execution_command(char **command, int command_args);
int execute_command(char **command, int command_args);
void stdout_to_file(char **command, int i);
void remove_new_line(char *string);
void read_input(char *input, char *file_path);
void parse_commands(char** commands, char *input, int* total_commands);
void init_pipe(int* pipe_fd, char** commands);
void get_current_command(char** command, char** commands, int* command_args, int* actual_amount_commands);


int main(int argc, char *argv[])
{
  char input[256], file_path[256] = "[shell]";
  char *command[64], *commands[64];

  int total_commands, pipe_fd[2];
  int actual_amount_commands;
  while (1)
  {
    puts("");
    read_input(input, file_path);
    parse_commands(commands, input, &total_commands);

    int max_pid = fork();
    if (max_pid == 0) {
      int command_args, read_pipe;
      actual_amount_commands = 0;
      // Por default empieza en 0 para que el primer comando escriba, sin recibir argumentos
      read_pipe = 0; 
      while (actual_amount_commands < total_commands - 2) {
        init_pipe(pipe_fd, commands);
        get_current_command(command, commands, &command_args, &actual_amount_commands);
        execute_command_in_fork(command, read_pipe, pipe_fd[1], command_args);
        close(pipe_fd[1]);
        read_pipe = pipe_fd[0];
      }

      /* 
        En caso que el pipe que nos dará la salida del comando anterior no haya
        sido todavía ubicado como STDIN, entonces lo ubicamos para que se tome como argumento
        el output del comando anterior.
      */
      if (read_pipe != STDIN_FILENO)
        dup2 (read_pipe, STDIN_FILENO);

      get_current_command(command, commands, &command_args, &actual_amount_commands);
      return execute_command(command, command_args);
    }
    while(wait(NULL) > 0);
  }
  
  exit(EXIT_SUCCESS);
}


int execute_command_in_fork(char** command, int read_pipe, int write_pipe, int command_args) {
  int fork_pid = fork();
  if (fork_pid == 0) {
    /* 
      Primero debemos leer el output que nos dejó el otro comando en el pipe.
    */
    if (read_pipe != STDIN_FILENO) {
      dup2(read_pipe, STDIN_FILENO);
      close(read_pipe);
    }

    /*  
      Luego escribimos el nuevo output del comando actual en el pipe.
    */
    if (write_pipe != STDOUT_FILENO) {
      dup2(write_pipe, STDOUT_FILENO);
      close(write_pipe);
    } 
    return execute_command(command, command_args);
  }
  return -1;
}

int execute_command(char **command, int command_args) {
  if(command_args > 3 && strcmp(command[command_args - 3], ">") == 0)
    stdout_to_file(command, command_args);
  return execvp(command[0], command);
  exit(EXIT_FAILURE);
}

void stdout_to_file(char **command, int i)
{
  int fd_file = open(command[i - 2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
  dup2(fd_file, 1);
  command[i - 2] = command[i - 3] = NULL;
  return;
}

void remove_new_line(char *string)
{
  string[strcspn(string, "\n")] = '\0';
  return;
}

void read_input(char *input, char *file_path) {
  // setbuf(stdout, NULL);
  printf("%s$  ", file_path);
  fgets(input, 255, stdin);
  remove_new_line(input);
  return;
}

void parse_commands(char** commands, char *input, int* total_commands) {
  const char or[2] = "|";
  commands[0] = strtok(input, or);
  *total_commands = 1;
  while(commands[*total_commands - 1] != NULL)
    commands[(*total_commands)++] = strtok(NULL, or);
}

void init_pipe(int* pipe_fd, char** commands) {
  if (commands[1] != NULL) { // commands[1] != NULL -> hay OR
    int pipe_status = pipe(pipe_fd);
    if (pipe_status < 0) {
      perror(": COULDNT CREATE PIPE\n");
      exit(EXIT_SUCCESS);
    }
  }
}

void get_current_command(char** command, char** commands, int* command_args, int* actual_amount_commands) { 
  const char space[2] = " ";
  command[0] = strtok(commands[(*actual_amount_commands)++], space);
  *command_args = 1;
  while (command[*command_args - 1] != NULL)
    command[(*command_args)++] = strtok(NULL, space);
}