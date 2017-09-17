#include <stdio.h>
#include <stdlib.h>

void input_loop() {
  char line [100];
  char* tokens[50];
  while(1) {
    printf("> ");
    fgets(line, 100, stdin);
    printf("%l", strlen(line));
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
