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

void check_fflush() {
  int ret_val = fflush(stdout);
  if(ret_val != 0) {
    perror("Error occurred while flushing!");
    exit(errno);
  }
}

void check_chdir(char* directory) {
  int val = chdir(directory);

  if(val == -1){
    perror("An error occurred while attempting to change directory!");
    exit(errno);
  }
}

char* check_getcwd(char* buffer) {
  char wd[1024];
  char* ret_val = getcwd(wd, 1024);

  if(ret_val == NULL) {
    perror("An error occurred while getting working directory!");
    exit(errno);
  }
  else {
    return ret_val;
  }
}

char* get_ctime() {
  char* ret_val = ctime(&currenttime);
  if(ret_val == NULL) {
    perror("Error getting current time!");
    exit(-1);
  }
  else {
    return ret_val;
  }
}

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
  fprintf(stdout, "%s: Current Time: %s\n", caller, get_ctime());
  fprintf(stdout, "%s: Working directory: %s\n", caller, check_getcwd(wd));
  check_fflush();
}

void print_child(char* caller) {
	fprintf(stdout, "I am child %s, my PID is %d. My Parent is %d.\n", caller, getpid(), getppid());
	check_fflush();
}

char* check_getenv(char* variable) {
  char* ret_val;
  ret_val = getenv(variable);
  if(ret_val == NULL) {
    perror("Could not locate env. variable!");
    exit(-1);
  }
  else {
    return ret_val;
  }
}

void print_whale(char* caller) {
  char* env_var;
  env_var = check_getenv("WHALE");
  printf("%s: WHALE is %s\n", caller, env_var);
  check_fflush();
}

void decreaseWhaleBy(int x, char* caller) {
	char* env_var = check_getenv("WHALE");
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
    fprintf(stdout, "%s: WHALE is %s\n", caller, check_getenv("WHALE"));
  	check_fflush();
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
  check_chdir("/");
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
  fprintf(stdout, "%s: Working directory: %s\n", caller, check_getcwd(wd));
  check_fflush();

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
  pid_t val1 = wait(NULL);
  pid_t val2 = wait(NULL);

  if(val1 == -1 || val2 == -1) {
    perror("An error occured while waiting for child to terminate!");
    exit(-1);
  }
  return 1;
}
