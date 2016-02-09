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

void mapperWork(int mapperPipe) {
  char read_msg[BUFFER_SIZE];

  // read the line
  close(mapper_pipes[mapperPipe][WRITE_END]);
  read(mapper_pipes[mapperPipe][READ_END], read_msg, BUFFER_SIZE);
  printf("MAPPER %d: %s\n", mapperPipe, read_msg);
  close(mapper_pipes[mapperPipe][READ_END]);

  // send characters to the correct reducer
  for(int i = 0; i < strlen(read_msg); i++){
      // if it is a lowercase letter, send to correct pipe
      if(read_msg[i] >= ALPHA_OFFSET && read_msg[i] < ALPHA_OFFSET + LETTERS){
        int p = read_msg[i] - ALPHA_OFFSET;
        char letter = read_msg[i];
        printf("Pipe array index: %d. Character: %c\n", p, letter);
        printf("Length of character: %lu\n", strlen(&letter));

        close(reducer_pipes[0][READ_END]);
        write(reducer_pipes[0][WRITE_END], &letter, 1);
        //close(reducer_pipes[0][WRITE_END]);

      }
  }

  close(reducer_pipes[0][WRITE_END]);
  exit(EXIT_SUCCESS);
}

void reducerWork(int reducerPipe) {
  char buf;
  printf("Reducer pid: %d, reading from array index: %d\n", getpid(), reducerPipe);

  sleep(1);
  close(reducer_pipes[reducerPipe][WRITE_END]);

  while(read(reducer_pipes[0][READ_END], &buf, 1) == 1) {
    printf("REDUCER Char: %c\n", buf);
    sleep(1);
  }

  close(reducer_pipes[reducerPipe][READ_END]);
  exit(EXIT_SUCCESS);
}

void doParent() {
  for(int i = 0; i < 1; i++) {
    close(mapper_pipes[i][READ_END]);
    write(mapper_pipes[i][WRITE_END], lines[i], strlen(lines[i])+1);
    //write(mapper_pipes[i][WRITE_END], "a", strlen("a")+1);
    close(mapper_pipes[i][WRITE_END]);
  }
}

int main() {
  char read_msg[BUFFER_SIZE];
  char buffer[BUFFER_SIZE];
  pid_t mapper;
  pid_t reducer;
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
  for(int i = 0; i < 1; i++) {
    if(pipe(reducer_pipes[i]) == -1) {
      perror("ERROR creating pipe!");
      exit(-1);
    }
  }

  // create mapper processes
  for(int i = 0; i < 1; i++) {
    pid_t child = fork();

    if(child < 0) {
      perror("Error forking child");
      exit(-1);
    }
    else if(child == 0) { // mapper process
      int mapperPipe = i;
      mapperWork(mapperPipe);
    }
  }

  // create reducer processes
  for(int i = 0; i < 1; i++) {
    pid_t child = fork();

    if(child < 0) {
      perror("Error forking child");
      exit(-1);
    }
    else if(child == 0) { // reducer process
      int reducerPipe = i;
      reducerWork(reducerPipe);
    }
  }

  doParent();
  wait(NULL);

  return 1;
}
