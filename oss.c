/*      Program Description: This program is designed to traverse given or 
 *      default directory using depth-first principle. This utility is a great
 *      way to practice system calls.
 *      Author: Bekzod Tolipov
 *      Date: 09/01/2019
 *      Class: CS:4760-001
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <stdbool.h>
#include <sys/wait.h>

//Prototypes
int toint(char str[]);
static int setuptimer(int s);
static int setupinterrupt();
static void myhandler(int s);

#define MAXCHAR 1024
#define TIME_OUT 2
#define ERR_EXIT 1
#define SUCC_EXIT 0
#define SEM_NAME "/Semaphore"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1

pid_t parent_pid;
pid_t child_id;
pid_t child_arr[20];
static bool quit;

//Shared clock
struct Clock{
	int sec;
	int ns;
};

#define MAXCHAR 1024
int main(int argc, char **argv)
{
	char file_name[MAXCHAR] = "log.dat";
	char dummy[MAXCHAR];
	int numb_child = 5;
	int max_time = 5;
	int c;
	short  sarray[] = { 0, 0 };
	int *p;                       /*      shared variable         *//*shared */
	sem_t *sem;                   /*      synch semaphore         *//*shared */
	unsigned int value = 1;           /*      semaphore value         */
	quit = false;
	
	parent_pid = getpid();
	struct Clock *clock_point;

	while ((c = getopt (argc, argv, "hs:l:t:")) != -1){
		switch (c)
		{
			case 'h':
				printf("To run the program you have following options:\n[-h]\n[s maximum number of processes spawned]\n[l filename]\n[t time in seconds]\n");
				return 0;
			case 's':
				strncpy(dummy, optarg, 255);
                numb_child = toint(dummy);
                if(numb_child > 20)
					abort();
				break;
			case 'l':
				strncpy(file_name, optarg, 255);
				break;
			case 't':
				strncpy(dummy, optarg, 255);
				max_time = toint(dummy);
                break;
			default:
				fprintf(stderr, "ERROR: Wrong Input is Given!");
				abort();

		}
	}

	// ftok to generate unique key 
	key_t key = ftok("./oss.c", 21);  
	// shmget returns an identifier i`n shmid 
	int shmid = shmget(key,sizeof(struct Clock),0666|IPC_CREAT); 
	// shmat to attach to shared memory 
	clock_point = shmat(shmid,NULL,0);

	clock_point->sec = 0;
	clock_point->ns = 0;
 	
	/* We initialize the semaphore counter to 1 (INITIAL_VALUE) */
    sem_t *semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);

    if (semaphore == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);
    }

    /* Close the semaphore as we won't be using it in the parent process */
  //  if (sem_close(semaphore) < 0) {
  //      perror("sem_close(3) failed");
  //      /* We ignore possible sem_unlink(3) errors here */
  //      sem_unlink(SEM_NAME);
  //      exit(EXIT_FAILURE);
  //  }
	
	if(setuptimer(10) == -1){
       //  fprintf(fwrite, "ERROR: Failed set up timer");
         fprintf(stderr, "ERROR: Failed set up timer");
       //  fclose(fp);
        // fclose(fwrite);
         return ERR_EXIT;
     }
     if(setupinterrupt() == -1){
         //fprintf(fwrite, "ERROR: Failed to set up handler");
         fprintf(stderr, "ERROR: Failed to set up handler");
        // fclose(fwrite);
        // fclose(fp);
         return ERR_EXIT;
     }
	//char *args[]={"./user", "85412", "Guru", NULL};
	int i;             //execvp(args[0], args);
	for(i=0; i<20; i++)
		child_arr[i] = 0;

	int child_pid;
	int count = 0;
	while(!quit){

		if(count < numb_child)
			child_pid = fork();
		
  		if (child_pid == 0) {
            execl("./user", "./user", NULL);
			printf("Got out of\n");
  		} 
		else {

			for(i=0; i<20; i++){
			//	printf("Adding child\n");
				if(child_arr[i] == 0){
					//printf("Adding child: %d\n", child_pid);
					child_arr[i] = child_pid;
					break;
				}
			}
			count++;
		}
		int stat;
		pid_t remove_pid = waitpid(-1, &stat, WNOHANG);
		
	//printf("remove_pid: %d\n", remove_pid);
		if(remove_pid > 0){
		//	printf("Remove in first if: %d\n", remove_pid);
			for(i=0; i<20; i++){
			//	printf("Looping\n");
				if(child_arr[i] == remove_pid)
				{	
			//		printf("removing %d\n", remove_pid);
					child_arr[i] = 0;
					count--;
				}
			}
		}
		
		if (sem_wait(semaphore) < 0) {
            perror("sem_wait(3) failed on Parent");
            continue;
        }

        //increment seconds
        clock_point->sec += 1;

        if (sem_post(semaphore) < 0) {
            perror("sem_post(3) error on parent");
        }
	

	}

			
	shmdt(clock_point);
	shmctl(shmid, IPC_RMID, NULL);
	/* cleanup semaphores */
    //sem_destroy (sem);
	if (sem_unlink(SEM_NAME) < 0)
        perror("sem_unlink(3) failed");
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

/* Set up timer */
static int setuptimer(int time){
    struct itimerval value;
    value.it_value.tv_sec = time;
   value.it_value.tv_usec = 0;

    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    return(setitimer(ITIMER_REAL, &value, NULL));
}
 
/* Set up interrupt */
static int setupinterrupt(){
    struct sigaction act;
    act.sa_handler = &myhandler;
    act.sa_flags = SA_RESTART;
    return(sigemptyset(&act.sa_mask) || sigaction(SIGALRM, &act, NULL));
}

/* Set up my own handler */
static void myhandler(int s){
	printf("My handler enter\n");
	int i;
	for(i=0; i<20; i++){
		if(child_arr[i] != 0){
			if(kill(child_arr[i], 0) == 0){
				kill(child_arr[i], SIGTERM);
			}
		}
	}

	quit = true;
	printf("Myhandler\n");
}
	
