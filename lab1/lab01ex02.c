#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (void) {
  
  int msg;
  int filedes[2];
  
  msg = pipe(filedes);
  if (msg == -1) {
    printf("[P] Error creating pipe.\n");
    exit(EXIT_FAILURE);
  }
  
  // fork process
  pid_t c_pid = fork();

  if (c_pid == -1) {
    printf("[P] Error forking process.\n");
    exit(EXIT_FAILURE);
    
  } else if (c_pid == 0) {
    printf("[C] Message from child.\n");
    
    // Close file descriptor for writing to pipe
    close(filedes[1]);
    
    char message[255];
    int message_length;
    
    // read message from pipe
    message_length = read(filedes[0], message, sizeof(message));
    
    // close file descriptor for reading from pipe
    close(filedes[0]);
    
    if (message_length < 0) {
      printf("[C] Error on reading from pipe: %d.\n", message_length);
    } else if (message_length == 0) {
      printf("[C] Received empty IPC message.\n");
    } else {    
      printf("[C] Received IPC message: '%s'\n", message);
    }
    
    // exit (child) process
    exit(EXIT_SUCCESS);
    
  } else {
    printf("[P] Child created, pid=%d.\n", c_pid);
    printf("[P] Message from parent.\n");
    
    // Close file descriptor for reading from pipe
    close(filedes[0]);
    
    char message[] = "IPC message";
    
    // Write message to pipe (IPC)
    msg = write(filedes[1], message, sizeof(message));
    
    // close file descriptor for writing to pipe
    close(filedes[1]);
    
    if (msg < 0) {
      printf("[P] Error on writing to pipe: %d\n", msg);
    } else {
      printf("[P] Send IPC message: '%s'\n", message);
    }
  }
  
  int close_reason;
  
  // Wait on child process
  c_pid = wait(&close_reason);

  if (c_pid < 0) {
    printf("[P] Waiting for child process failed (%d, %d).\n", 
      c_pid, close_reason);
    exit(EXIT_FAILURE);
  }
  
  printf("[P] Child (pid=%d) exited with status %d.\n", c_pid, close_reason);
    
  return EXIT_SUCCESS;
}
