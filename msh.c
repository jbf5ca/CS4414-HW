#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

char linecset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-./_ <>|";
char wordcset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-./_";
char controls[] = "<>|";

int illegal_chars(char compstr[], char charset[]) {
  return ( strspn(compstr, charset) != strlen(compstr) - 1 );
}

void cmd_exec(char* argv[], int cmd_count, int tcount) {
  if (cmd_count == 1) {
    //int i;
    //char* argv[tcount];
    //for (i = 0; i < tcount; i++) {
    //argv[i] = tokens[i];
    //}

    pid_t pid = fork();
    int status;
    //printf("%i", pid);
    if (pid == -1) {
      // ERROR MESSAGE
      //printf("error creating child process");
      exit(EXIT_FAILURE);
    }
    if (pid == 0) {  /// child process
      execve(argv[0], argv);
      _Exit(EXIT_FAILURE);
    } else { /// parent
      waitpid(pid, &status, 0);
      printf("%s exited with status %d\n", argv[0], WEXITSTATUS(status));
    }
  } else {
    printf("piping not yet implemented\n");
  }
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

    /// remove trailing newline from last token
    tokens[tcount-1][strcspn(tokens[tcount-1], "\n")] = 0;
    
    /// strip token array to proper length
    //char* argv[tcount];
    for (i = tcount; i < 51; i++) {
      //argv[i] = tokens[i];
      tokens[i] = '\0';
    }

    //printf("commands: %i\n", groupcount);
    //printf("tokens: ");
    //for (i = 0; i < tcount; i++) {
    //printf("[%s] ", tokens[i]);
    //}
    //printf("\n");
    /// now send the token groups to the command handler
    cmd_exec(tokens, groupcount, tcount);
  }
}

int main(int argc, char* argv[]) {
  setenv("TERM", "msh", 0);
  input_loop();
  return 0;
}
