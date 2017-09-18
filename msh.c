#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

char linecset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-./_ <>|";
char wordcset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-./_";
char controls[] = "<>|";

int illegal_chars(char compstr[], char charset[]) {
  return ( strspn(compstr, charset) != strlen(compstr) - 1 );
}

void cmd_exec(char* tokengoups[], int cmd_count) {

}

void input_loop() {
  //char line[103];
  //char* tokengroups[50];
  //int i, groupcount;
  const char *pipe = "|";
  const char *space = " ";

  while (1) {
    char line[103];
    char* tokengroups[50];
    char* tokens[51];
    int i, groupcount, tcount, err;

    printf("> ");
    fgets(line, 103, stdin);
    //printf("%lu\n", strlen(line));
    if (strlen(line) > 101) {
      // ERROR MESSAGE GOES HERE
      continue;
    }
    if (strcmp(line, "exit\n") == 0) {
      break;
    }

    /// check that the line only has legal characters
    if (strspn(line, linecset) != strlen(line) - 1) {
      // ERROR MESSAGE GOES HERE
      printf("line has illegal chars\n");
      continue;
    }

    /// check that no token group is both preceded by a pipe operator and has an input file redirection
    if (strchr(line, '|') != NULL && strchr(line, '<') != NULL) {
      if (strchr(line, '|') < strchr(line, '<')) {
	// ERROR MESSAGE
	printf("conflict between pipe and input redirection\n");
	continue;
      }
    }

    /// check that no token group is both followed by a pipe operator and has an output file redirection
    if (strrchr(line, '|') != NULL && strrchr(line, '>') != NULL) {
      if (strrchr(line, '|') > strrchr(line, '>')) {
	// ERROR MESSAGE
	printf("conflict between pipe and output redirection\n");
	continue;
      }
    }

    /// check that there isn't an attempt to redirect input after an output redirection
    if (strchr(line, '<') != NULL && strchr(line, '>') != NULL) {
      if (strchr(line, '>') < strchr(line, '<')) {
	// ERROR MESSAGE
	printf("conflict between pipe and input redirection\n");
	continue;
      }
    }

    /// check that there aren't attempts for multiple of the same file redirection
    if (strchr(line, '<') != strrchr(line, '<') || strchr(line, '>') != strrchr(line, '>')) {
      // ERROR MESSAGE
      printf("attempt for multiple of the same file redirection\n");
    }

    /// split the line into space-delimited tokens
    char* token = strtok(line, space);
    tcount = 0;
    while (token != NULL) {
      tokens[tcount] = token;
      token = strtok(NULL, space);
      tcount++;
    }
    //printf("tcount: %i\n", tcount);

    /// check that pipe and redirection characters only appear alone and not within a word or at the beginning or end of a token group
    if (strpbrk(tokens[0], controls) != NULL) {
      // ERROR MESSAGE
      printf("illegal control character in first token\n");
      continue;
    }
    if (tcount > 1 && strpbrk(tokens[tcount-1], controls) != NULL) {
      // ERROR MESSAGE
      printf("illegal control character in last token\n");
    }

    /// count token groups, checking misplaced <,>,|
    groupcount = 1;
    err = 0;
    for (i = 0; i < tcount - 1; i++) {
      if (strchr(tokens[i], '|') != NULL) {
	if (strlen(tokens[i]) > 1) {
	  // ERROR MESSAGE
	  printf("pipe character placed within word\n");
	  err = 1;
	} else if (strspn(tokens[i+1], controls) > 0) {
	  // ERROR MESSAGE
	  printf("another control operator illegally following pipe operator\n");
	  err = 1;
	} else {
	  groupcount++;
	}
      } else if (strchr(tokens[i], '<') != NULL || strchr(tokens[i], '>') != NULL) {
	if (strspn(tokens[i+1], controls) > 0) {
	  printf("another control operator illegally following file redirect operator\n");
	  err = 1;
	}
      }
    }

    if (err) {
      continue;
    }
    printf ("commands: %i\n", groupcount);


    /// split the line into token groups separated by pipes
    /*groupcount = 0;
    char* token = strtok(line, pipe);
    while (token != NULL) {
      tokengroups[groupcount] = token;
      token = strtok(NULL, pipe);
      groupcount++;
      }*/
   
    //printf("groupcount: %i\n", groupcount);
    //for (i = 0; i < groupcount; i++) {
    //  printf("%i, %s\n", i, tokengroups[i]);
    //}

    /// now send the token groups to the command handler
    //cmd_exec(tokengroups, groupcount);
  }
}

int main(int argc, char* argv[]) {
  input_loop();
  return 0;
}
