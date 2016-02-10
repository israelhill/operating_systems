#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define ALPHA_OFFSET 97 // ascii code for "a"
#define LETTERS 26 // letters in the alphabet
#define READ_END 0 // read end of the pipe
#define WRITE_END 1 // write end of the pipe

int mapper_pipes[2][2];
int reducer_pipes[26][2];
char* lines[4] = {};
char counts[LETTERS] = {0};
int mapperPids[4];
int reducerPids[26];

void close_reducer_pipes() {
  for(int i = 0; i < 26; i++) {
    close(reducer_pipes[i][WRITE_END]);
  }
}

void printCounts() {
  for(int i = 0; i < LETTERS; i++) {
    printf("count %c: %d\n", i + ALPHA_OFFSET, counts[i]);
  }
}

void mapperWork(int mapperPipe) {
  char read_msg[BUFFER_SIZE];
  int current_pipe;

  // read the line
  close(mapper_pipes[mapperPipe][WRITE_END]);
  read(mapper_pipes[mapperPipe][READ_END], read_msg, BUFFER_SIZE);
  printf("MAPPER %d: %s\n", mapperPipe, read_msg);
  close(mapper_pipes[mapperPipe][READ_END]);

  // send characters to the correct reducer
  for(int i = 0; i < strlen(read_msg); i++){
      current_pipe = i;
      // if it is a lowercase letter, send to correct pipe
      if(read_msg[i] >= ALPHA_OFFSET && read_msg[i] < ALPHA_OFFSET + LETTERS){
        int p = read_msg[i] - ALPHA_OFFSET;
        char letter = read_msg[i];

        //printf("Sending: [%c]\n", letter);

        close(reducer_pipes[p][READ_END]);
        write(reducer_pipes[p][WRITE_END], &letter, 1);
      }
  }

  close_reducer_pipes();
  exit(EXIT_SUCCESS);
}

void reducerWork(int reducerPipe) {
  char buf;

  printf("Reducer Here!!");

  sleep(1);
  close(reducer_pipes[reducerPipe][WRITE_END]);

  while(read(reducer_pipes[reducerPipe][READ_END], &buf, 1) > 0) {
    printf("REDUCER index: %d, REDUCER Char: %c\n", reducerPipe, buf);
    counts[reducerPipe]++;
    sleep(1);
  }

  close(reducer_pipes[reducerPipe][READ_END]);
  exit(EXIT_SUCCESS);
}

void doParent() {
  for(int i = 0; i < 4; i++) {
    close(mapper_pipes[i][READ_END]);
    write(mapper_pipes[i][WRITE_END], lines[i], strlen(lines[i])+1);
    //write(mapper_pipes[i][WRITE_END], "a", strlen("a")+1);
    close(mapper_pipes[i][WRITE_END]);
  }
}

int main() {
  char read_msg[BUFFER_SIZE];
  char buffer[BUFFER_SIZE];
  FILE *input_file = fopen("input.txt", "r");

  // write lines to an array
  int i = 0;
  while(fgets(buffer, BUFFER_SIZE, input_file) > 0) {
    lines[i] = strdup(buffer);
    i++;
  }

  // create array of mapper pipes
  for(int i = 0; i < 4; i++) {
    if(pipe(mapper_pipes[i]) == -1) {
      perror("ERROR creating pipe!");
      exit(-1);
    }
  }

  // create array of reducer pipes
  for(int i = 0; i < 26; i++) {
    if(pipe(reducer_pipes[i]) == -1) {
      perror("ERROR creating pipe!");
      exit(-1);
    }
  }

  // create mapper processes
  for(int i = 0; i < 4; i++) {
    pid_t child = fork();

    if(child < 0) {
      perror("Error forking child");
      exit(-1);
    }
    else if(child == 0) { // mapper process
      printf("calling mapper work\n");
      int mapperPipe = i;
      //mapperPids[i] = getpid();
      mapperWork(mapperPipe);
    }
    else { // parent

    }
  }

  // create reducer processes
  for(int i = 0; i < 26; i++) {
    pid_t child = fork();

    if(child < 0) {
      perror("Error forking child");
      exit(-1);
    }
    else if(child == 0) { // reducer process
      int reducerPipe = i;
      printf("Reducer pid: %d\n", getpid());
      //reducerPids[i] = getpid();
      reducerWork(reducerPipe);
    }
  }

  doParent();
  wait(NULL);
  //printCounts();

  return 1;
}
