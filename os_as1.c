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

time_t currenttime;
 
char* checkUserId() {
  char* val = cuserid(NULL);
  if (val == NULL) {
    perror("cuserid");
    exit(errno);
  } else {
    return val;
  }
}

/*
char* check_c_time(time_t current_time) {
  char* ret_val[] = ctime(&current_time);
  if(ret_val == NULL) {
    perror("Error getting current time!");
    exit(-1);
  }
  else {
    return ret_val;
  }
}
*/

int checkFork(int syscall) {
  if (syscall < 0) {
    perror("Error while forking child process!");
    exit(errno);
  }
  else {
    return syscall;
    }
}

void checkTime() {
  time_t val = time(&currenttime);
  if (val == ((time_t) -1)) {
    perror("Error in time call");
    exit(errno);
    }
}

void printParentInfo() {
  char* caller = "P0";
  int pid = getpid();
  int parentPid = getppid();
  char hostName[1024];
  char* userId = checkUserId();
  checkTime();
  char wd[1024];
  gethostname(hostName, 1024);

  fprintf(stdout, "%s: Main PID: %d\n", caller, pid);
  fprintf(stdout, "%s: Parent PID: %d\n", caller, parentPid);
  fprintf(stdout, "%s: Hostname: %s\n", caller, hostName);
  fprintf(stdout, "%s: User ID: %s\n", caller, userId);
  fprintf(stdout, "%s: Current Time: %s\n", caller, ctime(&currenttime));
  fprintf(stdout, "%s: Working directory: %s\n", caller, getcwd(wd, 1024));
  fflush(stdout);
}

void print_child(char* caller) {
	fprintf(stdout, "I am child %s, my PID is %d. My Parent is %d.\n", caller, getpid(), getppid());
	fflush(stdout);
}

void print_whale(char* caller) {
  char* env_var;
  env_var = getenv("WHALE");
  printf("%s: WHALE is %s\n", caller, env_var);
  fflush(stdout);
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
		perror("ERROR setting environment variable");
		exit(errno);
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

  sleep(3);
  chdir("/");
  char* command = "/bin/ls";
  execl(command, command, "-la", (char*)NULL);

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

  sleep(3);
  char wd[1024];
  fprintf(stdout, "%s: Working directory: %s\n", caller, getcwd(wd, 1024));
  fflush(stdout);

  exit(0);
}

int main() {
  pid_t child;

  int ret_val;
  if(ret_val = putenv("WHALE=7") != 0) {
    perror("An error occurred while setting env. variable!");
    exit(-1);
  }
  printParentInfo();

  child = checkFork(fork());
  if(child == 0) {
    child_a_procedure();
  }

  child = checkFork(fork());
  if(child == 0) {
    child_b_procedure();
  }

  parent_procedure();
  wait(NULL);
  wait(NULL);
  return 1;
}
