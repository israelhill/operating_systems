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

void print_child(char* caller) {
	fprintf(stdout, "I am child %s, my PID is %d. My Parent is %d.\n", caller, getpid(), getppid());
	fflush(stdout);
}

void print_whale(char* caller) {
  char* env_var;
  env_var = getenv("WHALE");
  printf("%s: WHALE is %s\n", caller, env_var);
}

void decreaseWhaleBy(int x, char* caller) {
	char* env_var = getenv("WHALE");
	int new_env_var;

	sscanf(env_var, "%d", &new_env_var);
	new_env_var = (int) new_env_var - x;

  char prefix[] = "WHALE=";
  char buffer[40];
  sprintf(buffer, "%s%d", prefix, new_env_var);

	int ret_val = putenv(buffer);

	if(ret_val < 0) {
		fprintf(stderr, "ERROR setting environment variable");
		_exit(errno);
	}
  else {
    fprintf(stdout, "%s: WHALE is %s\n", caller, getenv("WHALE"));
  	fflush(stdout);
  }
}

void parent_procedure() {
  char* caller = "P0";

  sleep(3);
  print_whale(caller); //print whale 7

  sleep(3);
  decreaseWhaleBy(3, caller); // whale 4

  sleep(3);
  decreaseWhaleBy(3, caller); //whale 1
}

void child_a_procedure() {
  char* caller = "C1";

  sleep(1);
  print_child(caller);

  sleep(3);
  decreaseWhaleBy(1, caller);

  sleep(3);
  decreaseWhaleBy(3, caller);

  exit(0);
}

void child_b_procedure() {
  char* caller = "C2";

  sleep(2);
  print_child(caller);

  sleep(3);
  decreaseWhaleBy(2, caller);

  sleep(3);
  decreaseWhaleBy(3, caller);

  exit(0);
}

int main() {
  pid_t child;

  putenv("WHALE=7");
  printParentInfo();

  child = fork();
  if(child == 0) {
    child_a_procedure();
  }

  child = fork();
  if(child == 0) {
    child_b_procedure();
  }

  parent_procedure();
  wait(NULL);
  wait(NULL);
  return 1;
}
