#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (void) {
    
  // fork process
  pid_t c_pid = fork();

  if (c_pid == -1) {
    printf("[P] Error forking process.\n");
    exit(EXIT_FAILURE);
    
  } else if (c_pid == 0) {
    printf("[C] Message from child.\n");
    
    // exit (child) process
    exit(EXIT_SUCCESS);
    
  } else {
    printf("[P] Child created, pid=%d.\n", c_pid);
    printf("[P] Message from parent.\n");
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
