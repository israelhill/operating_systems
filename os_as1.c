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
#include <string.h>

// char* cuserid_wrapper() {
//   char* val = cuserid(NULL);
//   if (val == NULL) {
//     perror("cuserid");
//     exit(errno);
//   } else {
//     return val;
//   }
// }

void printParentInfo() {
  int pid = getpid();
  int parentPid = getppid();
  char hostName[1024];
  // char* userId = cuserid_wrapper();
  time_t currenttime;
  time(&currenttime);
  char wd[1024];
  gethostname(hostName, 1024);

  printf("Main PID: %d\n", pid);
  printf("Parent PID: %d\n", parentPid);
  printf("Hostname: %s\n", hostName);
  // printf("User ID: %s\n", userId);
  printf("Current Time: %s\n", ctime(&currenttime));
  printf("Working directory: %s\n", getcwd(wd, 1024));
}

void parent_procedure() {

}

void decreaseWhaleBy(int x) {
	char *env_var = getenv("WHALE");
	int new_env_var;

	sscanf(env_var, "%d", &new_env_var);
  printf("CURRENT VARIABLE: %s\n", env_var);
	new_env_var = (int) new_env_var - x;

  char prefix[] = "WHALE=";
  char buffer[40];
  sprintf(buffer, "%s%d", prefix, new_env_var);
  printf("NEW ENV VSRISBLE: %d\n", new_env_var);

	int ret_val = putenv(buffer);
  printf("ENV_VAR is %s\n", getenv("WHALE"));
	if(ret_val < 0) {
		fprintf(stderr, "ERROR setting environment variable");
		_exit(errno);
	}
}



int main() {
  int child_a, child_b;

  putenv("WHALE=7");
  printParentInfo();

  child_a = fork();

  if(child_a < 0) {
    printf("Error\n");
    exit(1);
  }
  else if(child_a == 0) {
    //child 1
    printf("I am child 1 with pid: %d\n", getpid());
    exit(0);
  }
  else {
    // parent
    child_b = fork();
    if(child_b < 0) {
      //child 2
      printf("Error\n");
    }
    else if(child_b == 0) {
      sleep(2);
      printf("I am child 2 with pid: %d\n", getpid());
      exit(0);
    }

    wait(NULL);
    sleep(3);
    printf("Children finished\n");
    char* env_var;
    env_var = getenv("WHALE");
    printf("WHALE is %s\n", env_var);
    decreaseWhaleBy(3);
    return 1;
  }
}
