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
  int i, j, nextin = 0, filedesc[2];
  char rin[100], rout[100];
  pid_t pid;
  pid_t pids[50];
  char* cmds[50];
  int status;
  int statuses[50];

  /// prepare to redirect input to first command if necessary, otherwise it should remain 0 for stdin
  if (in_redir) {
    getcwd(rin, sizeof(rin));
    strcat(rin, "/");
    strcat(rin, infile);
    
    nextin = open(rin, O_RDONLY);
  }

  for (i = 0; i < groupcount - 1; i++) {
    //char** argv = (char **)malloc(sizeof(commands[0]));
    char* argv[100];
    for (j = 0; j < 100; j++) {
      argv[j] = (char *)malloc(strlen(commands[i][j])+1);
      strcpy(argv[j], commands[i][j]);
    }
    //memcpy(&argv, &commands[i], sizeof(commands[i]));
    //while(j < 100 && strlen(commands[i][j]) > 0) {
    //printf("%s\n", commands[i][j]);
    //strcpy(argv[j], commands[i][j]);
    //j++;
    //}
    /// create the pipes 
    pipe(filedesc);

    pid = fork();
    if (pid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    if (pid == 0) { /// child process
      /// save the pid and cmd string for exit status output
      pids[i] = pid;
      //strcpy(cmds[0], commands[i][0]);
      strcpy(cmds[0], argv[0]);

      /// redirect input if necessary
      if (nextin != 0) {
	dup2(nextin, 0);
	close(nextin);
      }

      /// redirect output, always going to the pipe for these threads
      dup2(filedesc[1], 1);
      close(filedesc[1]);

      /// exec the command
      //execve(commands[i][0], commands[i], NULL);
      printf("exec %s", argv[0]);
      execve(argv[0], argv, NULL);
      _Exit(EXIT_FAILURE);
    } else {
      /// save exit code
      waitpid(pid, &status, 0);
      statuses[i] = status;

      close(filedesc[1]);
      nextin = filedesc[0];
    }
    for (j = 0; j < 100; j++) {
      free(argv[j]);
    }
  } // forloop

  /// run the last command
  char* argv[100];
  for (j = 0; j < 100; j++) {
    //printf("%s : ", commands[i][j]);
    argv[j] = (char *)malloc(strlen(commands[i][j])+1);
    strcpy(argv[j], commands[i][j]);
    //printf("%s\n", argv[j]);
  }

  if (nextin != 0) {
    dup2(nextin, 0);
    close(nextin);
  }
  pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    pids[i] = pid;
    //strcpy(cmds[0], commands[i][0]);
    strcpy(cmds[0], argv[0]);

    printf("bout to exec %s\n", argv[0]);
    //execve(commands[i][0], commands[i], NULL);
    execve(argv[0], argv, NULL);
    _Exit(EXIT_FAILURE);
  } else {
    for (j = 0; j < 100; j++) {
      free(argv[j]);
    }
    waitpid(pid, &status, 0);
    statuses[i] = status;
  }

  /// print out command exit codes
  for (i = 0; i < groupcount; i++) {
    printf("%s exited with exit code %d\n", cmds[i], WEXITSTATUS(statuses[i]));
  }
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

  /// convert all input/output filenames to absolute paths
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
    int i, groupcount, tcount, err;
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
	  //tokens[i][0] = '\0';
	  //tokens[i+1][0] = '\0';
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
	  //tokens[i][0] = '\0';
	  //tokens[i+1][0] = '\0';
	  break;
	}
      }
    }
    //printf("test1\n");
    char* argv[51];
    int x = 0;
    for (i = 0; i < tcount; i++) {
      if (tokens[i][0] == '>' || tokens[i][0] == '<') {
	i++;
      }
      else {
	//strcpy(argv[x], tokens[i]);
	//printf(".\n");
	argv[x] = tokens[i];
	x++;
      }
    }
    //printf("test2\n");
    if (in_redir) { tcount -= 2; }
    if (out_redir) { tcount -=2; }

    if (groupcount > 1) {
      char commands[100][100][100];
      int cmdcount = 0;
      int tok = 0, wordcount = 0, finish = 0;
      //for (i = 0; i < tcount; i++) {
      //printf("%s ", argv[i]);
      //}
      while (!finish && argv[tok] != NULL) {
	char cmd[100][100];
	wordcount = 0;
	while (tok < tcount && strcmp(argv[tok], "|") != 0) {
	  if (argv[tok] == NULL) {
	    finish = 1;
	    //wordcount++;
	    //tok++;
	    continue;
	  }
	  strcpy(cmd[wordcount], argv[tok]);
	  //printf("copied %s\n", argv[tok]);
	  tok++;
	  wordcount++;
	}
	//cmd[wordcount] = NULL;
	tok++;
      
        /// copy cmd, string-by-string into commands[cmdcount]
	//printf("got here, wc = %i\n", wordcount);
	for (i = 0; i < wordcount; i++) {
	  //printf("strcpy, i = %i, cmd[i] = %s\n", i, cmd[i]);
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
      io_redir(argv, in_redir, out_redir, infile, outfile, tcount);
      continue;
    }

    /// otherwise: single command with no io redirection, execute normally
    cmd_exec(argv, groupcount, tcount);
  }
}

int main(int argc, char* argv[]) {
  setenv("TERM", "msh", 0);
  input_loop();
  return 0;
}
