/* Andre Augusto Giannotti Scota (https://sites.google.com/view/a2gs/) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>

void * task(void *data)
{
	sem_t *semP = data;

	sem_wait(semP);

	/* CRITICAL REGION */
	printf("Thread Id [%ld] took the critical region.\n", pthread_self());

	sem_post(semP);

	return(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t t1, t2, t3;
	sem_t sem;
	int ret = 0;

	/* Second argument: shared between threads of same
	 * process, !0 between unrelated processes
	 * (i.e. fork/shmemo/mmap))
	 */
	ret = sem_init(&sem, 0, 1);
	if(ret == -1){
		printf("Error sem_init(): [%s]\n", strerror(errno));
		ret = -1;
		goto clean_all;
	}

	ret = pthread_create(&t1, NULL, task, &sem);
	if(ret != 0){
		printf("Error pthread_create(t1): [%s]\n", strerror(ret));
		ret = -1;
		goto clean_all;
	}

	ret = pthread_create(&t2, NULL, task, &sem);
	if(ret != 0){
		printf("Error pthread_create(t2): [%s]\n", strerror(ret));
		ret = -1;
		goto clean_all;
	}

	ret = pthread_create(&t3, NULL, task, &sem);
	if(ret != 0){
		printf("Error pthread_create(t3): [%s]\n", strerror(ret));
		ret = -1;
		goto clean_all;
	}

	ret = pthread_join(t1, NULL);
	if(ret != 0){
		printf("Error pthread_join(t1): [%s]\n", strerror(ret));
		ret = -1;
		goto clean_all;
	}

	ret = pthread_join(t2, NULL);
	if(ret != 0){
		printf("Error pthread_join(t2): [%s]\n", strerror(ret));
		ret = -1;
		goto clean_all;
	}

	ret = pthread_join(t3, NULL);
	if(ret != 0){
		printf("Error pthread_join(t3): [%s]\n", strerror(ret));
		ret = -1;
		goto clean_all;
	}

	sem_destroy(&sem);

clean_all:
	return(ret);
}
