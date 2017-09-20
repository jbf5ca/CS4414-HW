/* MSH simple shell
 * John Fultz jbf5ca
 * CS 4414 Fall 2017
 * HW1
 *
 * compile: make
 * all code in msh.c, executable: msh
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
char linecset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-./_ <>|";
char wordcset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-./_";
char controls[] = "<>|";
void pipe_exec(char commands[100][100][100], int groupcount, int in_redir, int out_redir, char infile[], char outfile[], int tcount) {
  int i,j;
  if (in_redir) { printf("{%s} ", infile); }
  for (i = 0; i < groupcount; i++) {
    j = 0;
    while (strlen(commands[i][j]) > 0){// != NULL) {
      printf("[%s]", commands[i][j]);
      j++;
    }
    if (i < groupcount - 1) { printf("| "); }
  }
  if (out_redir) { printf("{%s}\n", outfile); }
}
void io_redir(char* argv[], int in_redir, int out_redir, char infile[], char outfile[], int tcount) {
  int fd;
  char rin[100], rout[100], tmp[100];
  /// if the command is a relative path, prepend cwd to make it absolute
  if (argv[0][0] != '/') {
    strcpy(tmp, argv[0]);
    getcwd(argv[0], sizeof(argv[0]));
    strcat(argv[0], "/");
    strcat(argv[0], tmp);
  }
  if (in_redir) {
    getcwd(rin, sizeof(rin));
    strcat(rin, "/");
    strcat(rin, infile);
  }
  if (out_redir) {
    getcwd(rout, sizeof(rout));
    strcat(rout, "/");
    strcat(rout, outfile);
  }
  pid_t pid = fork();
  int status;
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) { /// child process
    if (in_redir) {
      fd = open(rin, O_RDONLY);
      dup2(fd, 0);
      close(fd);
    }
    if (out_redir) {
      fd = open(rout, O_WRONLY | O_CREAT, 0666);
      dup2(fd, 1);
      close(fd);
    }
    execve(argv[0], argv, NULL);
    _Exit(EXIT_FAILURE);
  } else { /// parent
    waitpid(pid, &status, 0);
    printf("%s exited with exit code %d\n", argv[0], WEXITSTATUS(status));
  }
}
void cmd_exec(char* argv[], int cmd_count, int tcount) {
  /// if the command is a relative path, prepend cwd to make it absolute
  char tmp[100];
  if (argv[0][0] != '/') {
    strcpy(tmp, argv[0]);
    getcwd(argv[0], sizeof(argv[0]));
    strcat(argv[0], "/");
    strcat(argv[0], tmp);
  }
  pid_t pid = fork();
  int status;
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) { /// child process
    execve(argv[0], argv, NULL);
    _Exit(EXIT_FAILURE);
  } else { /// parent
    waitpid(pid, &status, 0);
    printf("%s exited with exit code %d\n", argv[0], WEXITSTATUS(status));
  }
}

void input_loop() {
  //const char *pipe = "|";
  const char *space = " ";
  while (1) {
    char line[103];//, line2[104];
    //char* tokengroups[50];
    char* tokens[51];
    //char tokens[51][100];
    int i, j, groupcount, tcount, err;
    int in_redir = 0, out_redir = 0;
    char infile[100];
    char outfile[100];
    printf(">");
    
    /// read the input line and check for the end of file
    if (fgets(line, 103, stdin) == NULL) {
      break;
    }

    /// make sure input line is 100 characters or less
    if (strlen(line) > 101) {
      perror("Input line too long");
      continue;
    }

    /// check for the exit command to terminate shell 
    if (strcmp(line, "exit\n") == 0 ) { 
      break;
    }

    /// check that the line only has legal characters
    if (strspn(line, linecset) != strlen(line) - 1) {
      perror("line has illegal chars\n");
      continue;
    }

    /// check that no token group is both preceded by a pipe operator and has an input file redirection
    if (strchr(line, '|') != NULL && strchr(line, '<') != NULL) {
      if (strchr(line, '|') < strchr(line, '<')) {
	perror("conflict between pipe and input redirection\n");
	continue;
      }
    }

    /// check that no token group is both followed by a pipe operator and has an output file redirection
    if (strrchr(line, '|') != NULL && strrchr(line, '>') != NULL) {
      if (strrchr(line, '|') > strrchr(line, '>')) {
	perror("conflict between pipe and output redirection\n");
	continue;
      }
    }

    /// check that there isn't an attempt to redirect input after an output redirection
    if (strchr(line, '<') != NULL && strchr(line, '>') != NULL) {
      if (strchr(line, '>') < strchr(line, '<')) {
	perror("conflict between pipe and input redirection\n");
	continue;
      }
    }

    /// check that there aren't attempts for multiple of the same file redirection
    if (strchr(line, '<') != strrchr(line, '<') || strchr(line, '>') != strrchr(line, '>')) {
      perror("attempt for multiple of the same file redirection\n");
    }

    /// check for legal IO redirections in the line
    if (strchr(line, '<') != NULL) { in_redir = 1; }
    if (strchr(line, '>') != NULL) { out_redir = 1; }

    /// split the line into space-delimited tokens
    char* token = strtok(line, space);
    tcount = 0;
    while (token != NULL) {
      tokens[tcount] = token;
      token = strtok(NULL, space);
      tcount++;
    }

    /// check that pipe and redirection characters only appear alone and not within a word or at the beginning or end of a token group
    if (strpbrk(tokens[0], controls) != NULL) {
      perror("illegal control character in first token\n");
      continue;
    }
    if (tcount > 1 && strpbrk(tokens[tcount-1], controls) != NULL) {
      perror("illegal control character in last token\n");
    }

    /// count token groups, checking for misplaced <,>,|
    groupcount = 1;
    err = 0;
    for (i = 0; i < tcount - 1; i++) {
      if (strchr(tokens[i], '|') != NULL) {
	if (strlen(tokens[i]) > 1) {
	  perror("pipe character placed within word\n");
	  err = 1;
	} else if (strspn(tokens[i+1], controls) > 0) {
	  perror("another control operator illegally following pipe operator\n");
	  err = 1;
	} else {
	  groupcount++;
	}
      } else if (strchr(tokens[i], '<') != NULL || strchr(tokens[i], '>') != NULL) {
	if (strspn(tokens[i+1], controls) > 0) {
	  perror("another control operator illegally following file redirect operator\n");
	  err = 1;
	}
      }
    }

    /// continue to next input line if any operator placement errors
    if (err) {
      continue;
    }

    /// remove trailing newline from last token
    tokens[tcount-1][strcspn(tokens[tcount-1], "\n")] = 0;

    /// strip token array to proper length
    for (i = tcount; i < 51; i++) {
      tokens[i] = '\0';
    }

    /// input redirect: extract input file and remove non-arg tokens
    if (in_redir) {
      for (i = 0; i < tcount - 1; i++) {
	if (tokens[i][0] == '<') {
	  strcpy(infile, tokens[i+1]);
	  //for (j = i; j < tcount - 2; j++) {
	  //strcpy(tokens[j], tokens[j+2]);
	  //}
	  //tokens[tcount - 1] = '\0';
	  //tokens[tcount - 2] = '\0';
	  //tcount -= 2;
	  //break;
	  tokens[i] = '\0';
	  tokens[i+1] = '\0';
	  break;
	}
      }
    }

    /// output redirect: extract output file and remove non-arg tokens
    if (out_redir) {
      for (i = 0; i < tcount - 1; i++) {
	if (tokens[i][0] == '>') {
	  strcpy(outfile, tokens[i+1]);
	  //for (j = i; j < tcount - 2; j++) {
	  //strcpy(tokens[j], tokens[j+2]);
	  //}
	  //tokens[tcount - 1] = '\0';
	  //tokens[tcount - 2] = '\0';
	  //tcount -= 2;
	  tokens[i] = '\0';
	  tokens[i+1] = '\0';
	  break;
	}
      }
    }

    if (groupcount > 1) {
      char commands[100][100][100];
      int cmdcount = 0;
      int tok = 0, wordcount = 0, finish = 0;
      for (i = 0; i < tcount; i++) {
	printf("%s ", tokens[i]);
      }
      while (!finish && tokens[tok] != NULL) {
	char cmd[100][100];
	wordcount = 0;
	while (tok < tcount && strcmp(tokens[tok], "|") != 0) {
	  if (tokens[tok] == NULL) {
	    finish = 1;
	    //wordcount++;
	    //tok++;
	    continue;
	  }
	  strcpy(cmd[wordcount], tokens[tok]);
	  printf("copied %s\n", tokens[tok]);
	  tok++;
	  wordcount++;
	}
	//cmd[wordcount] = NULL;
	tok++;
      
        /// copy cmd, string-by-string into commands[cmdcount]
	printf("got here, wc = %i\n", wordcount);
	for (i = 0; i < wordcount; i++) {
	  printf("strcpy, i = %i, cmd[i] = %s\n", i, cmd[i]);
	  strcpy(commands[cmdcount][i], cmd[i]);
	}
	//commands[cmdcount][wordcount] = "\0";
	cmdcount++;
      }
      //commands[cmdcount] = NULL;
      
      pipe_exec(commands, groupcount, in_redir, out_redir, infile, outfile, tcount);
      continue;
    }

    /// identify and execute commands with io redirection
    else if (in_redir || out_redir) {
      io_redir(tokens, in_redir, out_redir, infile, outfile, tcount);
      continue;
    }

    /// otherwise: single command with no io redirection, execute normally
    cmd_exec(tokens, groupcount, tcount);
  }
}

int main(int argc, char* argv[]) {
  setenv("TERM", "msh", 0);
  input_loop();
  return 0;
}
