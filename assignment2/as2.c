#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define ALPHA_OFFSET 97 // ascii code for "a"
#define LETTERS 26 // letters in the alphabet
#define READ_END 0 // read end of the pipe
#define WRITE_END 1 // write end of the pipe
#define NUMBER_MAPPERS 4 //there are 4 mappers
#define NUMBER_REDUCERS 26 // there are 26 reducers, 1 for each letter

int mapper_pipes[NUMBER_MAPPERS][2];
int reducer_pipes[NUMBER_REDUCERS][2];
char* lines[NUMBER_MAPPERS] = {};
int count = 0;

void close_wrapper(int fd) {
  if(close(fd) == -1) {
    perror("ERROR closing pipe!");
    exit(errno);
  }
}

int read_wrapper(int fd, void *buf, size_t count) {
  int ret_val = read(fd, buf, count);
  if(ret_val == -1) {
    perror("ERROR reading from pipe!");
    exit(errno);
  }
  else {
    return ret_val;
  }
}

void close_reducer_pipes() {
  int i;
  for(i = 0; i < NUMBER_REDUCERS; i++) {
    close_wrapper(reducer_pipes[i][WRITE_END]);
  }
}

void close_unused_mapper_pipes(int mapperPipe) {
  int z;
  for(int z = 0; z < NUMBER_MAPPERS; z++) {
    if(z != mapperPipe) {
      close_wrapper(mapper_pipes[z][READ_END]);
      close_wrapper(mapper_pipes[z][WRITE_END]);
    }
    else {
      close_wrapper(mapper_pipes[z][WRITE_END]);
    }
  }
}

void mapperWork(int mapperPipe) {
  char read_msg[BUFFER_SIZE];

  close_unused_mapper_pipes(mapperPipe);

  // read line sent from parent
  read_wrapper(mapper_pipes[mapperPipe][READ_END], read_msg, BUFFER_SIZE);
  close_wrapper(mapper_pipes[mapperPipe][READ_END]);

  // send characters to the correct reducer
  int i;
  for(i = 0; i < NUMBER_REDUCERS; i++) {
    close_wrapper(reducer_pipes[i][READ_END]);
  }

  for(i = 0; i < strlen(read_msg); i++){
      // if it is a lowercase letter, send to correct pipe
      if(read_msg[i] >= ALPHA_OFFSET && read_msg[i] < ALPHA_OFFSET + LETTERS){
        int p = read_msg[i] - ALPHA_OFFSET;
        char letter = read_msg[i];

        write(reducer_pipes[p][WRITE_END], &letter, 1);
      }
  }

  for(i = 0; i < NUMBER_REDUCERS; i++) {
    close_wrapper(reducer_pipes[i][WRITE_END]);
  }

  exit(EXIT_SUCCESS);
}

void reducerWork(int reducerPipe) {
  char buf;
  int letter_as_int = ALPHA_OFFSET + reducerPipe;
  char reducer_letter = letter_as_int;

  int i;
  for(i = 0; i < NUMBER_REDUCERS; i++) {
    close_wrapper(reducer_pipes[i][WRITE_END]);
  }

  // increment the count of the character
  while(read_wrapper(reducer_pipes[reducerPipe][READ_END], &buf, 1) > 0) {
    count++;
  }

  close_wrapper(reducer_pipes[reducerPipe][READ_END]);
  printf("%c: %d\n", reducer_letter, count);
  exit(EXIT_SUCCESS);
}

void doParent() {
  // send 1 line to each mapper
  int i;
  for(i = 0; i < NUMBER_MAPPERS; i++) {
    close_wrapper(mapper_pipes[i][READ_END]);
    write(mapper_pipes[i][WRITE_END], lines[i], strlen(lines[i])+1);
    close_wrapper(mapper_pipes[i][WRITE_END]);
  }

  // close write and read ends to all reducer pipes
  int j;
  for(j = 0; j < NUMBER_REDUCERS; j++) {
    close_wrapper(reducer_pipes[j][WRITE_END]);
    close_wrapper(reducer_pipes[j][READ_END]);
  }

  // wait for all children to terminate
  wait(NULL);
  exit(EXIT_SUCCESS);
}

int main() {
  char buffer[BUFFER_SIZE];
  FILE *input_file = fopen("input.txt", "r");

  if(input_file == NULL) {
    perror("ERROR opening file!");
    exit(errno);
  }

  // write lines to an array
  int line = 0;
  while(fgets(buffer, BUFFER_SIZE, input_file) > 0) {
    lines[line] = strdup(buffer);
    line++;
  }
  fclose(input_file);

  // create array of mapper pipes
  int i;
  for(i = 0; i < 4; i++) {
    if(pipe(mapper_pipes[i]) == -1) {
      perror("ERROR creating pipe!");
      exit(errno);
    }
  }

  // create array of reducer pipes
  int reducerArrays;
  for(reducerArrays = 0; reducerArrays < NUMBER_REDUCERS; reducerArrays++) {
    if(pipe(reducer_pipes[reducerArrays]) == -1) {
      perror("ERROR creating pipe!");
      exit(errno);
    }
  }

  // create mapper processes
  int mapNum;
  for(mapNum = 0; mapNum < NUMBER_MAPPERS; mapNum++) {
    pid_t mchild = fork();

    if(mchild < 0) {
      perror("ERROR forking child");
      exit(errno);
    }
    else if(mchild == 0) { // mapper process
      int mapperPipe = mapNum;
      mapperWork(mapperPipe);
    }
    else { // parent
    }
  }

  // create reducer processes
  int redNum;
  for(redNum = 0; redNum < 26; redNum++) {
    pid_t rchild = fork();

    if(rchild < 0) {
      perror("ERROR forking child");
      exit(-1);
    }
    else if(rchild == 0) { // reducer process
      int reducerPipe = redNum;
      reducerWork(reducerPipe);
    }
    else { // parent
    }
  }

  doParent();
  return 1;
}
