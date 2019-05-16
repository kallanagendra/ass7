#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "clock.h"

static FILE * input = NULL;
static FILE * output = NULL ;
static int n = 0, N = 4;  /* started, max started */
static int r = 0, R = 2;  /* running, max running */
static int t = 0; /* started, running,terminated users */

static int run = 1; /* master loop flag */
static struct clock * shm_clock = NULL;

static int set_options(const int argc, char * const argv[]){
  int opt;
	while((opt=getopt(argc, argv, "hn:s:i:o:")) != -1){  //get option
		switch(opt){
			case 'n': N      = atoi(optarg);    break;
      case 's': R	     = atoi(optarg);    break;

      case 'i':
        if((input = fopen(optarg, "r")) == NULL){
          perror("fopen");
          return 1;
        }
        break;

      case 'o':
        if((output = fopen(optarg, "w")) == NULL){
          perror("fopen");
          return 1;
        }
        break;

      default:
				fprintf(stdout, "Error: Invalid option '%c'\n", opt);
        //break;  /* fall through to 'h' */
      case 'h':
        fprintf(stdout,"Usage: oss {OPTIONS}");
        fprintf(stdout,"\t-h\t OPTIONS\n");
        fprintf(stdout,"\t-n [4]\tMax users to start\n");
        fprintf(stdout,"\t-s [2]\t Max users running at the same time\n");
        fprintf(stdout,"\t-i [input.txt]\tInput filename\n");
        fprintf(stdout,"\t-o [output.txt]\tOutput filename\n");
  			return 1;
		}
	}

	if(input == NULL){
    if((input = fopen("input.txt", "r")) == NULL){
      perror("fopen");
      return 1;
    }
	}

  if(output == NULL){
    if((output = fopen("output.txt", "w")) == NULL){
      perror("fopen");
      return 1;
    }
	}

  return 0;
}

static void sig_handler(const int sig){
  int status;
  pid_t pid;

  //check what signal we caught
  switch(sig){
    case SIGTERM: case SIGALRM:
      fprintf(output,"[%i:%i] Caught signal %d\n", shm_clock->sec, shm_clock->nsec, sig);
      run = 0; //set the flat to true, to interrupt main loop
      break;

    case SIGCHLD:

      while((pid = waitpid(-1, &status, WNOHANG)) > 0){  //call wait on the terminated child
        if (WIFEXITED(status)) {
          fprintf(output,"[%i:%i] child %i terminated with exit code %d\n", shm_clock->sec, shm_clock->nsec, pid, WEXITSTATUS(status));
        }else if(WIFSIGNALED(status)){
          fprintf(output,"[%i:%i] child %i killed (signal %d)\n", shm_clock->sec, shm_clock->nsec, pid, WTERMSIG(status));
        }

        r--;
        if(++t >= N)  /* if all started have terminated */
          run = 0;    /* we are done */
      }
      break;

    default:
      break;
  }
}

int main(const int argc, char * const argv[]){

  if(set_options(argc, argv) == 1){ //if we failed to parse arguments
    return EXIT_FAILURE;
  }

  //allocate space for users array
  pid_t *pids = NULL;
  pids = (pid_t *) malloc(sizeof(pid_t)*N);
  if(pids == NULL){
    perror("malloc");
    return EXIT_FAILURE;
  }

  key_t key = ftok("makefile", 4444);
  if(key == -1){
    perror("ftok");
    return EXIT_FAILURE;
  }

  int shm_id = shmget(key, sizeof(struct clock), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  if(shm_id == -1){
    perror("shmget");
    return EXIT_FAILURE;
  }

  shm_clock = (struct clock*) shmat(shm_id, NULL, 0);
  if(shm_clock == NULL){
    perror("shmat");
    return EXIT_FAILURE;
  }


  signal(SIGCHLD, sig_handler); /* catch dead children */
  signal(SIGTERM, sig_handler); /* catch Ctrl-C */
  signal(SIGALRM, sig_handler); /* setup alarm */
  alarm(2);

  //set up the virtual clock
  shm_clock->sec = shm_clock->nsec = 0;
  bzero(pids, sizeof(pid_t)*N); /* zero out the array */


  //get first user line
  unsigned int increment = 0;
  struct clock fork_clock;
  unsigned int duration;

  fscanf(input, "%u", &increment); //read first line with duration
  fscanf(input, "%u %u %u", &fork_clock.sec, &fork_clock.nsec, &duration);

	while(run){  //master loop


    if( /* if we have more lines in the input file*/
        (duration > 0) &&

        /* if we can start more processes */
        (n < N) && (r < R) &&

        /* if its time to fork */
        (check_time(shm_clock, &fork_clock) == AFTER)  ){

        pid_t pid = fork(); /* create a child */
        if(pid > 0){  /* parent section */
          pids[n++] = pid;
          r++;
          fprintf(output,"[%i:%i] Started child %i with PID=%i\n", shm_clock->sec, shm_clock->nsec, n, pid);

          if(!feof(input)){
            fscanf(input, "%u %u %u", &fork_clock.sec, &fork_clock.nsec, &duration);
          }else{
            duration = 0;
          }

        }else if(pid == 0){ /* child section */

          char child_arg[20];
          snprintf(child_arg, 20, "%u", duration); /* make child argument */

          execl("user", "user", child_arg, NULL); /* run the user program */
          perror("execl");

          exit(1);

        }else{ /* pid < 0, which means error */
          perror("fork");
          run = 0;
        }
    }

		add_time(shm_clock, increment);
    //usleep(1000);
	}


  /* stop the child users */
  int i, status;
  for(i=0; i < n; i++){
    kill(pids[i], SIGTERM);
    wait(&status);
  }
  free(pids);

  fprintf(output, "[%i:%i] oss done\n", shm_clock->sec, shm_clock->nsec);

  //release chared memory
  shmdt(shm_clock);
  shmctl(shm_id, IPC_RMID, NULL);

  return EXIT_SUCCESS;
}
