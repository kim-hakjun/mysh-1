#include <string.h>
#include <assert.h>
#include "commands.h"
#include "built_in.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
//void catch_sigint(int signalNo);

int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  pid_t pid;
  char path_resolve[5][100]={"/usr/local/bin/","/usr/bin/","/bin/","/usr/sbin/","/sbin/"};
  char temp[100];
  

  if (n_commands > 0) {
    struct single_command* com = (*commands);

    assert(com->argc != 0);

    int built_in_pos = is_built_in_command(com->argv[0]);
    if (built_in_pos != -1) {
      if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
        if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
          fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } else {
        fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        return -1;
      }
    } else if (strcmp(com->argv[0], "") == 0) {
      return 0;
    } else if (strcmp(com->argv[0], "exit") == 0) {
      return 1;
    } else if((pid=fork())==-1){
	perror("fork error!");
	exit(-1);
    }
      else if(pid==0){ // child
	strcpy(temp,com->argv[0]);
	for(int search=0;search<5;search++){
	  if(execv(temp,com->argv)!=0){
  	    strcat(path_resolve[search],com->argv[0]);
	    strcpy(temp,path_resolve[search]);
	  }	
	  else
	    break;
	}
	fprintf(stderr, "%s: command not found\n", com->argv[0]);	
	exit(1);
    }
      else if(wait(NULL)) 
	return -1;
  }
  return 0;
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
