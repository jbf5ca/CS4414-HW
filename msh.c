#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

void input_loop() {
  char line[103];
  char* tokens[51];
  int tcount, i;
  while (1) {
    printf("> ");
    fgets(line, 103, stdin);
    //printf("%lu\n", strlen(line));
    if (strlen(line) > 101) {
      // ERROR MESSAGE GOES HERE
      break;
    }
    if (strcmp(line, "exit\n") == 0) {
      break;
    }
    //printf("%s", line);
    char* token = strtok(line, " ");
    tcount = 0;
    while (token != NULL) {
      tokens[tcount] = token;
      token = strtok(NULL, " ");
      tcount++;
    }
    printf("%s\nargs:", tokens[0]);
    for (i = 1; i < tcount; i++) {
      printf(" %s", tokens[i]);
    }
  }
}

int main(int argc, char* argv[]) {
  input_loop();
  return 0;
}
