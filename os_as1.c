#define _XOPEN_SOURCE // required for cuserid to work

// includes
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>

char* cuserid_wrapper() {
  char* val = cuserid(NULL);
  if (val == NULL) {
    perror("cuserid");
    exit(errno);
  } else {
    return val;
  }
}

void printParentInfo() {
  int pid = getpid();
  int parentPid = getppid();
  char hostName[1024];
  char* userId = cuserid_wrapper();
  time_t currenttime;
  time(&currenttime);
  char wd[1024];
  gethostname(hostName, 1024);

  printf("Main PID: %d\n", pid);
  printf("Parent PID: %d\n", parentPid);
  printf("Hostname: %s\n", hostName);
  printf("User ID: %s\n", userId);
  printf("Current Time: %s\n", ctime(&currenttime));
  printf("Working directory %s\n", getcwd(wd, 1024));
}

int main() {
  printParentInfo();
}
