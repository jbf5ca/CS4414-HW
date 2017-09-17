#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void input_loop() {
  char line [103];
  char* tokens[50];
  while(1) {
    printf("> ");
    fgets(line, 103, stdin);
    printf("%lu\n", strlen(line));
    if(strlen(line) > 101) {
      break;
    }
    if(strcmp(line, "exit\n") == 0) {
      break;
    }
    printf("%s", line);
  }
}

int main(int argc, char* argv[]) {
  input_loop();
  return 0;
}
