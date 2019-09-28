#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>

struct Clock{
    int sec;
    int ns;
};

long int MAX_DURATION;
static bool quit;

int main(int argc, char *argv[]) 
{
	int shmid;
	quit = true;
	MAX_DURATION = (rand() % (0 - 1000000 + 1)) + 0;	
	
	key_t key = ftok("./oss.c", 21);
	
	shmid = shmget(key, sizeof(struct Clock), IPC_CREAT | 0666);

	struct Clock *clock_point;
	clock_point = shmat(shmid, NULL, 0);	
	//Set up child max duration
	MAX_DURATION = 

	sem_t *semaphore = sem_open("/Semaphore", O_RDWR);
    if (semaphore == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }

    int i;
    while (!quit) {
        if (sem_wait(semaphore) < 0) {
            perror("sem_wait(3) failed on child");
           // continue;
        }

        //printf("PID %ld acquired semaphore\n", (long) getpid());
		printf("User Second: %d\n", clock_point->sec);
		//clock_point->sec++;

        if (sem_post(semaphore) < 0) {
            perror("sem_post(3) error on child");
        }

        sleep(1);
    }

    if (sem_close(semaphore) < 0)
        perror("sem_close(3) failed");

	return 0;
}
// Function is designed to convert string to an integer
int toint(char str[])
{
    int len = strlen(str);
    int i, num = 0;

    for (i = 0; i < len; i++)
    {
        num = num + ((str[len - (i + 1)] - '0') * pow(10, i));
    }

   return num;
}

