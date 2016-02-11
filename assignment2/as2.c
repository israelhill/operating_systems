#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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
int mapperPids[NUMBER_MAPPERS];
int reducerPids[NUMBER_REDUCERS];

void close_reducer_pipes() {
  for(int i = 0; i < NUMBER_REDUCERS; i++) {
    close(reducer_pipes[i][WRITE_END]);
  }
}

void mapperWork(int mapperPipe) {
  char read_msg[BUFFER_SIZE];

  // read the line
  close(mapper_pipes[mapperPipe][WRITE_END]);
  read(mapper_pipes[mapperPipe][READ_END], read_msg, BUFFER_SIZE);
  close(mapper_pipes[mapperPipe][READ_END]);

  // send characters to the correct reducer
  for(int i = 0; i < strlen(read_msg); i++){
      // if it is a lowercase letter, send to correct pipe
      if(read_msg[i] >= ALPHA_OFFSET && read_msg[i] < ALPHA_OFFSET + LETTERS){
        int p = read_msg[i] - ALPHA_OFFSET;
        char letter = read_msg[i];

        close(reducer_pipes[p][READ_END]);
        write(reducer_pipes[p][WRITE_END], &letter, 1);
      }
  }

  exit(EXIT_SUCCESS);
}

void reducerWork(int reducerPipe) {
  char buf;
  int letter_as_int = ALPHA_OFFSET + reducerPipe;
  char reducer_letter = letter_as_int;

  for(int i = 0; i < 26; i++) {
    close(reducer_pipes[i][WRITE_END]);
  }

  // increment the count of the character
  while(read(reducer_pipes[reducerPipe][READ_END], &buf, 1) > 0) {
    count++;
  }

  close(reducer_pipes[reducerPipe][READ_END]);
  printf("%c: %d\n", reducer_letter, count);
  exit(EXIT_SUCCESS);
}

void doParent() {
  int status;
  for(int i = 0; i < NUMBER_MAPPERS; i++) {
    close(mapper_pipes[i][READ_END]);
    write(mapper_pipes[i][WRITE_END], lines[i], strlen(lines[i])+1);
    close(mapper_pipes[i][WRITE_END]);
  }

  for(int i = 0; i < NUMBER_MAPPERS; i++) {
    waitpid(mapperPids[i], &status, WUNTRACED);
  }

  // for(int i = 0; i < NUMBER_REDUCERS; i++) {
  //   waitpid(reducerPids[i], &status, WUNTRACED);
  // }

  exit(EXIT_SUCCESS);
}

int main() {
  char buffer[BUFFER_SIZE];
  FILE *input_file = fopen("input.txt", "r");

  // write lines to an array
  int line = 0;
  while(fgets(buffer, BUFFER_SIZE, input_file) > 0) {
    lines[line] = strdup(buffer);
    line++;
  }
  fclose(input_file);

  // create array of mapper pipes
  for(int i = 0; i < 4; i++) {
    if(pipe(mapper_pipes[i]) == -1) {
      perror("ERROR creating pipe!");
      exit(-1);
    }
  }

  // create array of reducer pipes
  int reducerArrays;
  for(reducerArrays = 0; reducerArrays < 26; reducerArrays++) {
    if(pipe(reducer_pipes[reducerArrays]) == -1) {
      perror("ERROR creating pipe!");
      exit(-1);
    }
  }

  // create mapper processes
  for(int mapNum = 0; mapNum < 4; mapNum++) {
    pid_t mchild = fork();

    if(mchild < 0) {
      perror("Error forking child");
      exit(-1);
    }
    else if(mchild == 0) { // mapper process
      int mapperPipe = mapNum;
      mapperWork(mapperPipe);
    }
    else { // parent
      mapperPids[mapNum] = mchild;
    }
  }

  // create reducer processes
  for(int redNum = 0; redNum < 26; redNum++) {
    pid_t rchild = fork();

    if(rchild < 0) {
      perror("Error forking child");
      exit(-1);
    }
    else if(rchild == 0) { // reducer process
      int reducerPipe = redNum;
      reducerWork(reducerPipe);
    }
    else { // parent
      reducerPids[redNum] = rchild;
    }
  }

  doParent();
  return 1;
}
