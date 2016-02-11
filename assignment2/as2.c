#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define ALPHA_OFFSET 97 // ascii code for "a"
#define LETTERS 26 // letters in the alphabet
#define READ_END 0 // read end of the pipe
#define WRITE_END 1 // write end of the pipe

int mapper_pipes[4][2];
int reducer_pipes[26][2];
char* lines[4] = {};
int count = 0;
int mapperPids[4];
int reducerPids[26];

void close_reducer_pipes() {
  for(int i = 0; i < 26; i++) {
    close(reducer_pipes[i][WRITE_END]);
  }
}

void mapperWork(int mapperPipe) {
  char read_msg[BUFFER_SIZE];
  // printf("mapper: %d\n", getpid());

  // read the line
  close(mapper_pipes[mapperPipe][WRITE_END]);
  read(mapper_pipes[mapperPipe][READ_END], read_msg, BUFFER_SIZE);
  // printf("MAPPER %d: %s\n", getpid(), read_msg);
  close(mapper_pipes[mapperPipe][READ_END]);

  // send characters to the correct reducer
  for(int i = 0; i < strlen(read_msg); i++){
      // if it is a lowercase letter, send to correct pipe
      if(read_msg[i] >= ALPHA_OFFSET && read_msg[i] < ALPHA_OFFSET + LETTERS){
        int p = read_msg[i] - ALPHA_OFFSET;
        char letter = read_msg[i];

        //printf("Sending: [%c]\n", letter);

        close(reducer_pipes[p][READ_END]);
        write(reducer_pipes[p][WRITE_END], &letter, 1);
      }
  }

  //close_reducer_pipes();
  // printf("Mapper exit: %d\n", getpid());
  exit(EXIT_SUCCESS);
}

void reducerWork(int reducerPipe) {
  char buf;
  int letter_as_int = ALPHA_OFFSET + reducerPipe;
  char reducer_letter =  letter_as_int;

  // sleep(1);
  close(reducer_pipes[reducerPipe][WRITE_END]);
  for(int i = 0; i < 26; i++) {
    close(reducer_pipes[i][WRITE_END]);
  }

  while(read(reducer_pipes[reducerPipe][READ_END], &buf, 1) > 0) {
    printf("REDUCER index: %d, REDUCER Char: %c\n", reducerPipe, buf);
    count++;
    // sleep(1);
  }

  close(reducer_pipes[reducerPipe][READ_END]);
  printf("%c: %d\n", reducer_letter, count);
  exit(EXIT_SUCCESS);
}

void doParent() {
  int status;
  for(int i = 0; i < 4; i++) {
    close(mapper_pipes[i][READ_END]);
    //printf("Parent sending: %s\n", lines[i]);
    write(mapper_pipes[i][WRITE_END], lines[i], strlen(lines[i])+1);
    close(mapper_pipes[i][WRITE_END]);
  }

  for(int i = 0; i < 4; i++) {
    printf("Pid of mapper: %d\n", mapperPids[i]);
  }

  for(int i = 0; i < 26; i++) {
    printf("Pid of reducer: %d\n", reducerPids[i]);
  }

  for(int i = 0; i < 4; i++) {
    waitpid(mapperPids[i], &status, WUNTRACED);
  }

  for(int i = 0; i < 26; i++) {
    close(reducer_pipes[i][WRITE_END]);
  }
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

  //printCounts();
  doParent();

  return 1;
}
