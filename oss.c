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

//Prototypes
int toint(char str[]);
void* create_shared_memory(size_t size);

//Shared clock
static int *seconds = 0;
static long int *nanosec = 0;

#define MAXCHAR 1024
int main(int argc, char **argv)
{
	char file_name[MAXCHAR] = "log.dat";
	char dummy[MAXCHAR];
	int numb_child = 5;
	int max_time = 5;
	int c;

	while ((c = getopt (argc, argv, "hs:l:t:")) != -1){
                switch (c)
                {
                        case 'h':
				printf("To run the program you have following options:\n[-h]\n[s maximum number of processes spawned]\n[l filename]\n[t time in seconds]\n");
				return 0;
			case 's':
				strncpy(dummy, optarg, 255);
                                numb_child = toint(dummy);
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

char* parent_message = "hello";  // parent process will write this message
  char* child_message = "goodbye"; // child process will then write this one

 // void* shmem = create_shared_memory(128);

  //memcpy(shmem, parent_message, sizeof(parent_message));

	seconds = mmap(NULL, sizeof *seconds, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	int pid = fork();

  if (pid == 0) {
	*seconds = 5;
	exit(0);

/*
    printf("Child read: %s\n", shmem);
    memcpy(shmem, child_message, sizeof(child_message));
    printf("Child wrote: %s\n", shmem);
*/
  } else {
	wait(NULL);
	printf("%d\n", *seconds);
        munmap(seconds, sizeof *seconds);
/*
    printf("Parent read: %s\n", shmem);
    sleep(1);
    printf("After 1s, parent read: %s\n", shmem);
*/  
	}

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

//Share the memory
void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, -1, 0);
}

	
