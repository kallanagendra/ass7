#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "clock.h"

int main(const int argc, char * const argv[]){

	if(argc != 2){	/* if we don't have 2 arguments */
		fprintf(stderr, "Usage: ./user <nanoseconds>\n");	/* show how to use */
		return EXIT_FAILURE;	/* return error code */
	}


	/* make a key to find to shared region */
	key_t shm_key = ftok("makefile", 4444);
	if(shm_key == -1){
		perror("ftok");
		return EXIT_FAILURE;
	}

	/* get the shared region id */
	const int shm_id = shmget(shm_key, sizeof(struct clock), 0);
	if(shm_id == -1){
		perror("shmget");
		return EXIT_FAILURE;
	}

	/* attach to the shared region id */
	struct clock *shm_clock = (struct clock *) shmat(shm_id, NULL, 0);
	if(shm_clock == (void*)-1){
		perror("shmat");
		return EXIT_FAILURE;
	}

	const unsigned int nsec = atoi(argv[1]); /* convert argument to int */

	//copy oss clock
	struct clock lifetime;
	lifetime.nsec = shm_clock->nsec;
	lifetime.sec	= shm_clock->sec;
	add_time(&lifetime, nsec);

	const pid_t pid = getpid();
	printf("[%i:%i] User with PID=%u started, will run for %d nsec\n", shm_clock->sec, shm_clock->nsec, pid, nsec);

	while(check_time(shm_clock, &lifetime) == BEFORE){	/* until our lifetime has not passed */
			usleep(100);	/* convert nanosecond to microseconds, and sleep */
	}

	printf("[%i:%i] User with PID=%u done\n", shm_clock->sec, shm_clock->nsec, pid);

	shmdt(shm_clock);

	return EXIT_SUCCESS;
}
