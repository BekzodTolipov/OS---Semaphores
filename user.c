#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h> 
#include <sys/types.h>
#include <time.h>
void fix_time();
static int setupinterrupt();
static void myhandler(int s);
void strfcat(char *fmt, ...);
void sem_clock_lock();
void sem_clock_release();
void sem_print_lock();
void sem_print_release();

struct Clock{
    int sec;
    int ns;
};

#define SEM_NAME "/Sema_btnkb_11"
#define SEM_NAME_2 "/Sem_output_11"

int rand_numb;
static bool quit;
int sec;
int ns;
char *shmMsg;
int *check_permission;
struct Clock *clock_point;
//int semaphore;
//int print_sem;
char buffer[2048];
/////////Semaphore///////////////
/* semaphore value, for semctl().                */
//union semun sem_val;

/* structure for semaphore operations.           */
struct sembuf sem_op;
int sem_id;
///////////////////////////////


int main(int argc, char *argv[]) 
{
	int shmid;
	quit = false;
	srand((unsigned) time(NULL));
	rand_numb = (rand() % (5 - 1000000 + 1)) + 5;	
	
	key_t key = ftok("./oss.c", 21);
	
	shmid = shmget(key, sizeof(struct Clock), IPC_CREAT | 0644);

	// ftok to generate unique key 
    key_t key_2 = ftok("./oss.c", 22); 
	key_t key_3 = ftok("./oss.c", 23);
	key_t key_4 = ftok("./oss.c", 24);
	
  
    // shmget returns an identifier in shmid 
    int shmid_2 = shmget(key_2,2048,0666 | IPC_CREAT); 
	int shmid_3 = shmget(key_4, sizeof(int), 0666 | IPC_CREAT);
  
    // shmat to attach to shared memory 
    shmMsg = (char*) shmat(shmid_2, NULL, 0);
	check_permission = (int*) shmat(shmid_3, NULL, 0); 

	if(setupinterrupt() == -1){
         fprintf(stderr, "ERROR: Failed to set up handler");
         return 1;
    }

	sem_id = semget(key_3, 2, IPC_CREAT | IPC_EXCL | 0666);
	semctl(sem_id, 0, SETVAL, 1);
	semctl(sem_id, 1, SETVAL, 1);
	clock_point = shmat(shmid, NULL, 0);
	
	//Set up child max duration
	sem_clock_lock();
	sec = clock_point->sec;
	ns = clock_point->ns;
	sem_clock_release;
	ns += rand_numb;

	fix_time();


	/* We initialize the semaphore counter to 1 (INITIAL_VALUE) */
 //   sem_id = semget(key_3, 2, IPC_CREAT | IPC_EXCL | 0666);

   // semctl(sem_id, 0, SETVAL, 1);
   // semctl(sem_id, 1, SETVAL, 1);

//	semaphore = sem_open(SEM_NAME, O_RDWR);
//    if (semaphore == SEM_FAILED) {
//        perror("sem_open(3) failed, Child\n");
 //       exit(EXIT_FAILURE);
 //   }

//	print_sem = sem_open(SEM_NAME_2, O_RDWR);
 //   if (print_sem == SEM_FAILED) {
 //       perror("sem_open(3) failed, Child printing\n");
 //       exit(EXIT_FAILURE);
 //   }


    int i;
	int length;
	char *target = buffer;
	union sigval value;
    while (!quit) {
		sem_clock_lock();
       // if (sem_wait(semaphore) < 0) {
       //     fprintf(stderr, "sem_wait(3) failed on child: %d", getpid());
       //     continue;
       // }
		printf("\n!Enter clock check! PID: %d\n", getpid());
		if(*check_permission == 0){
			printf("\n!!Passed permission check!! PID: %d\n", getpid());
			if(clock_point->sec == sec){
				if ( clock_point->ns > ns){
			//	if (sem_wait(print_sem) < 0) {
			//		fprintf(stderr, "sem_wait(3) failed on child(sem_print): %d", getpid());
			//	}
					sem_print_lock();
					//printf("Modified\n");
					strfcat("child reached %d.%d in child process\n", sec, ns);
				//sigqueue(getppid(), SIGUSR1, value);
				
					sem_print_release();
			//	if(sem_post(print_sem) < 0){
			//		perror("sem_post(3) error on child");
			//	}
					*check_permission = 1;
					//printf("Value in child permission %d\n", *check_permission);
					quit = true;
				}
			}
			else if(clock_point->sec > sec){
		//	if (sem_wait(print_sem) < 0) {
		//		fprintf(stderr, "sem_wait(3) failed on child(sem_print): %d", getpid());
		//	}
				sem_print_lock();

				strfcat("child reached %d.%d in child process\n", sec, ns);
		//	sigqueue(getppid(), SIGUSR1, value);
             
				sem_print_release();
		//	if(sem_post(print_sem) < 0){
		//		perror("sem_post(3) error on child");
         //   }
				*check_permission = 1;
			//	printf("Else statement %d\n", *check_permission);
				quit = true;
			}
		}
		else
			quit = false;

		printf("\n!Left clock check! PID: %d\n", getpid());
		sem_clock_release();
	
//		if(sem_post(semaphore) < 0){
//			perror("sem_post(3) error on child");
//		}
		
	}

//    if (sem_close(semaphore) < 0)
//        perror("sem_close(3) failed child\n");
//	if(sem_close(print_sem) < 0)
//		perror("Sem_close(3) failed child\n");
	
	//detach memory
	shmdt(shmMsg);
	shmdt(clock_point);
//	semctl(sem_id, 0, IPC_RMID);

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

void fix_time(){

	if(ns > 1000000000){
		sec++;
		ns -= 1000000000;
	}

}

/* Set up interrupt */
static int setupinterrupt(){
    struct sigaction act;
    act.sa_handler = &myhandler;
    act.sa_flags = SA_SIGINFO;
    return(sigemptyset(&act.sa_mask) || sigaction(SIGTERM, &act, NULL));
}

/* Set up my own handler */
static void myhandler(int s){
 //   printf("Child Termination\n");
	shmdt(shmMsg);
    shmdt(clock_point);
//	if (sem_close(semaphore) < 0)
//        perror("sem_close(3) failed child\n");
//    if(sem_close(print_sem) < 0)
//        perror("Sem_close(3) failed child\n");

	exit(1);
}

/* Copy child message to shared memory */
void strfcat(char *fmt, ...){
	va_list args;
	
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	strcpy(shmMsg, buffer);
}

//Lock clock semaphore
void sem_clock_lock(){
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

//Release clock semaphore
void sem_clock_release(){
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;    
	semop(sem_id, &sem_op, 1);
}

//Lock print semaphore
void sem_print_lock(){
    sem_op.sem_num = 1;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

//Release print semaphore
void sem_print_release(){
    sem_op.sem_num = 1;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

