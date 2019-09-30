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
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <stdbool.h>
#include <sys/wait.h>

//Prototypes
int toint(char str[]);
static int setuptimer(int s);
static int setupinterrupt();
static void myhandler(int s);
void fix_time();
void sem_clock_lock();
void sem_clock_release();

/////////////////////

#define MAXCHAR 1024
#define TIME_OUT 2
#define ERR_EXIT 1
#define SUCC_EXIT 0
#define SEM_NAME "/Sema_btnkb_11"
#define SEM_NAME_2 "/Sem_output_11"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1

pid_t parent_pid;
pid_t child_id;
pid_t child_arr[20];
static bool quit;
char *shmMsg;

/////////////////////
void on_sigusr1(int sig){
    // Note: Normally, it's not safe to call almost all library functions in a
  // signal handler, since the signal may have been received in a middle of a
   // call to that function.
	printf("Message: %s\n", shmMsg);
	strcpy(shmMsg, "\0");
}

/////////////////////


//Shared clock
struct Clock{
	int sec;
	int ns;
};

struct Clock *clock_point;

/////////Semaphore///////////////
/* semaphore value, for semctl().                */
//union semun sem_val;

/* structure for semaphore operations.           */
struct sembuf sem_op;
int sem_id;
///////////////////////////////

#define MAXCHAR 1024
int main(int argc, char **argv)
{
	char file_name[MAXCHAR] = "log.dat";
	char dummy[MAXCHAR];
	int numb_child = 2;
	int max_time = 5;
	int c;
	short  sarray[] = { 0, 0 };
	int *p;                       /*      shared variable         *//*shared */
//	sem_t *sem;                   /*      synch semaphore         *//*shared */
	unsigned int value = 1;           /*      semaphore value         */
	quit = false;
	
	parent_pid = getpid();
	//struct Clock *clock_point;

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

	signal(SIGUSR1, &on_sigusr1);

	// ftok to generate unique key 
	key_t key = ftok("./oss.c", 21);  
//	printf("Key: %d\n", key);
	key_t key_2 = ftok("./oss.c", 22);
//	printf("Key-2: %d", key_2);
	key_t key_3 = ftok("./oss.c", 23);

	//sleep(10);
	// shmget returns an identifier i`n shmid 
	int shmid = shmget(key,sizeof(struct Clock),0644|IPC_CREAT); 
	int shmid_2 = shmget(key_2, 2048,0666|IPC_CREAT);
	// shmat to attach to shared memory 
	clock_point = shmat(shmid,NULL,0);
//	char buffer[1024];
	shmMsg = shmat(shmid_2,NULL,0); 
  

	clock_point->sec = 0;
	clock_point->ns = 0;
 	
	/* We initialize the semaphore counter to 1 (INITIAL_VALUE) */
	sem_id = semget(key_3, 2, IPC_CREAT | IPC_EXCL | 0666);
	
	semctl(sem_id, 0, SETVAL, 1);
	semctl(sem_id, 1, SETVAL, 1);
//	semctl(sem_id, 2, SETVAL, 1);


    //sem_t *semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
	
  //  if (semaphore == SEM_FAILED) {
  //      perror("sem_open(3) error, Parent");
  //      exit(EXIT_FAILURE);
  //  }

	/* We initialize the semaphore counter to 1 (INITIAL_VALUE) */
  //  sem_t *sem_output = sem_open(SEM_NAME_2, O_CREAT | O_EXCL, SEM_PERMS, 1);

  //  if (sem_output == SEM_FAILED) {
   //      perror("sem_open(3) error, Parent for shmMsg");
   //      exit(EXIT_FAILURE);
   // }


    /* Close the semaphore as we won't be using it in the parent process */
//	if (sem_close(sem_output)

//	< 0) {
//        perror("sem_close(3) failed");
        /* We ignore possible sem_unlink(3) errors here */
 //        sem_unlink(SEM_NAME_2);
//        exit(EXIT_FAILURE);
//    }

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

	int i;             //execvp(args[0], args);
    for(i=0; i<20; i++)
			child_arr[i] = 0;

    int child_pid;
    int count = 0;
    int after_rem = 0;
    int forked_kids_total = 0;

    bool found = false;
    while(!quit){

        if(count < numb_child){
            child_pid = fork();
        //  forked_kids_total++;
        //  if(forked_kids_total >= 100){
        //      printf("Max reached");
        //      union sigval sig_val;
        //      sigqueue(getpid(), SIGALRM, sig_val);
        //      quit = true;
        //  }
        }

        if (child_pid == 0) {
            execl("./user", "./user", file_name, NULL);
        }
        else if(child_pid == -1){
            perror("Error: Fork() failed\n");

        }
        else {
            found = false;
            for(i=0;i<20;i++){
                if(child_arr[i] == child_pid){
                    found = true;
                }
            }
            if(!found){
                for(i=0; i<20; i++){
                    if(child_arr[i] == 0){
                        child_arr[i] = child_pid;
                        count++;
                        break;
                    }
               }
            }
		}
		int stat;
        pid_t remove_pid = waitpid(-1, &stat, WNOHANG);

        if(remove_pid > 0){
            for(i=0; i<20; i++){
                if(child_arr[i] == remove_pid)
                {
                    child_arr[i] = 0;
	                   break;
                }
            }
            count--;
        }

        sem_clock_lock();

        //increment seconds
        clock_point->sec += 1;
        fix_time();

        sem_clock_release();


    }
	
	printf("Freeing memories\n");
    shmdt(clock_point);
    shmdt(shmMsg);
    shmctl(shmid, IPC_RMID, NULL);
    shmctl(shmid_2, IPC_RMID, NULL);
    /* cleanup semaphores */
    semctl(sem_id, 0, IPC_RMID);
    semctl(sem_id, 1, IPC_RMID);


    return 0;



//	return 0;

}

// Function is designed to convert string to an integer
//int toint(char str[])
//{
//    int len = strlen(str);
//    int i, num = 0;

//    for (i = 0; i < len; i++)
//    {
//        num = num + ((str[len - (i + 1)] - '0') * pow(10, i));
//    }

//   return num;
//}

/* Set up timer */
//static int setuptimer(int time){
//    struct itimerval value;
//    value.it_value.tv_sec = time;
//   value.it_value.tv_usec = 0;

//    value.it_interval.tv_sec = 0;
//    value.it_interval.tv_usec = 0;
//    return(setitimer(ITIMER_REAL, &value, NULL));
//}
 
/* Set up interrupt */
//static int setupinterrupt(){
//    struct sigaction act;
//    act.sa_handler = &myhandler;
//    act.sa_flags = SA_RESTART;
//    return(sigemptyset(&act.sa_mask) || sigaction(SIGALRM, &act, NULL));
//}

/* Set up my own handler */
//static void myhandler(int s){
//	printf("Termination begin\n");
//	int i;
//	for(i=0; i<20; i++){
//		if(child_arr[i] != 0){
//			if(kill(child_arr[i], 0) == 0){
//				if(kill(child_arr[i], SIGTERM) != 0){
//					perror("Child can't be terminated for unkown reason\n");
//				}
//			}
// < 0) {
//        perror("sem_close(3) failed");
        /* We ignore possible sem_unlink(3) errors here */
//        sem_unlink(SEM_NAME_2);
//        exit(EXIT_FAILURE);
//    }
	
//	if(setuptimer(10) == -1){
       //  fprintf(fwrite, "ERROR: Failed set up timer");
 //        fprintf(stderr, "ERROR: Failed set up timer");
       //  fclose(fp);
        // fclose(fwrite);
//         return ERR_EXIT;
//     }
//     if(setupinterrupt() == -1){
         //fprintf(fwrite, "ERROR: Failed to set up handler");
//         fprintf(stderr, "ERROR: Failed to set up handler");
        // fclose(fwrite);
        // fclose(fp);
 //        return ERR_EXIT;
 //    }
//	int i;             //execvp(args[0], args);
//	for(i=0; i<20; i++)
//		child_arr[i] = 0;

//	int child_pid;
//	int count = 0;
//	int after_rem = 0;
//	int forked_kids_total = 0;

//	bool found = false;
//	while(!quit){

//		if(count < numb_child){
//			child_pid = fork();
		//	forked_kids_total++;
		//	if(forked_kids_total >= 100){
		//		printf("Max reached");
		//		union sigval sig_val;
		//		sigqueue(getpid(), SIGALRM, sig_val);
		//		quit = true;
		//	}
//		}
		
//  		if (child_pid == 0) {
//            execl("./user", "./user", file_name, NULL);
//  		}
//		else if(child_pid == -1){
//			perror("Error: Fork() failed\n");
			
//		} 
//		else {
//			found = false;
//			for(i=0;i<20;i++){
//				if(child_arr[i] == child_pid){
//					found = true;
//				}
//			}
//			if(!found){
//				for(i=0; i<20; i++){
//					if(child_arr[i] == 0){
//						child_arr[i] = child_pid;
//						count++;
//						break;
//					}
//				}
//			}
//		}
//		int stat;
//		pid_t remove_pid = waitpid(-1, &stat, WNOHANG);
//		
//		if(remove_pid > 0){
//			for(i=0; i<20; i++){
//				if(child_arr[i] == remove_pid)
//				{	
//					child_arr[i] = 0;
//					break;
//				}
//			}
//			count--;
//		}
		
//		sem_clock_lock();

        //increment seconds
//        clock_point->sec += 1;
//		fix_time();

//        sem_clock_release();
	

//	}

//	return 0;

//}

// Function is designed to convert string to an integer
//int toint(char str[])
//{
//    int len = strlen(str);
//    int i, num = 0;

//    for (i = 0; i < len; i++)
//    {
//        num = num + ((str[len - (i + 1)] - '0') * pow(10, i));
//    }

 //  return num;
//}

/* Set up timer */
//static int setuptimer(int time){
//    struct itimerval value;
//    value.it_value.tv_sec = time;
//   value.it_value.tv_usec = 0;

//    value.it_interval.tv_sec = 0;
//    value.it_interval.tv_usec = 0;
//    return(setitimer(ITIMER_REAL, &value, NULL));
//}
 
/* Set up interrupt */
//static int setupinterrupt(){
//    struct sigaction act;
//    act.sa_handler = &myhandler;
//    act.sa_flags = SA_RESTART;
//    return(sigemptyset(&act.sa_mask) || sigaction(SIGALRM, &act, NULL));
//}

/* Set up my own handler */
//static void myhandler(int s){
//	printf("Termination begin\n");
//	int i;
//	for(i=0; i<20; i++){
//		if(child_arr[i] != 0){
//			if(kill(child_arr[i], 0) == 0){
//				if(kill(child_arr[i], SIGTERM) != 0){
//					perror("Child can't be terminated for unkown reason\n");
//				}
//			}
//		}
	

//	printf("Freeing memories\n");	
///	shmdt(clock_point);
//	shmdt(shmMsg);
//	shmctl(shmid, IPC_RMID, NULL);
//	shmctl(shmid_2, IPC_RMID, NULL);
	/* cleanup semaphores */
//	semctl(sem_id, 0, IPC_RMID);
//	semctl(sem_id, 1, IPC_RMID);

//	if (sem_unlink(SEM_NAME) < 0)
 //       perror("sem_unlink(3) failed");
//	if (sem_unlink(SEM_NAME_2) < 0)
 //       perror("sem_unlink(3) failed");

//	return 0;

//}

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
	printf("Termination begin\n");
	int i;
	for(i=0; i<20; i++){
		if(child_arr[i] != 0){
			if(kill(child_arr[i], 0) == 0){
				if(kill(child_arr[i], SIGTERM) != 0){
					perror("Child can't be terminated for unkown reason\n");
				}
			}
		}
	}


	for(i=0;i<20;i++){
		if(child_arr[i] != 0){
			waitpid(child_arr[i], NULL, 0);
			printf("PARENT: Child: %d returned value is: %d\n", child_arr[i], WIFSIGNALED(child_arr[i]));
		}
	}

	quit = true;
}


void fix_time(){
    if((int)(clock_point->ns / 1000000000) == 1){
        clock_point->sec++;
        clock_point->ns -= 1000000000;
    }
}

//Lock the semaphore
void sem_clock_lock(){
	sem_op.sem_num = 0;
	sem_op.sem_op = -1;
	sem_op.sem_flg = 0;
	semop(sem_id, &sem_op, 1);
}

//Release semaphore
void sem_clock_release(){
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

