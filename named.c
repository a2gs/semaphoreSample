/* Andre Augusto Giannotti Scota (https://sites.google.com/view/a2gs/) */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define SEM_NAMED ("semnamed")
#define TOT_ITERATION (5)

int randSleep(unsigned int range)
{
	return(rand() % range);
}

int parent(sem_t *sem)
{
	int i = 0;
	int v = 0;

	srand((unsigned int)(time(0) / getpid()));

	for(i = 0; i < TOT_ITERATION; i++){
		sleep(randSleep(4));

		sem_wait(sem);

		/* CRITICAL REGION */
		if(sem_getvalue(sem, &v) == -1) printf("PARENT: Error sem_getvalue(): [%s]\n", strerror(errno));
		printf("PARENT: CRITICAL! [%d] Semaphore value: [%d]\n", i, v);

		sem_post(sem);
	}

	return(0);
}

int child(char id, sem_t *sem)
{
	int i = 0;
	int v = 0;

	srand((unsigned int)(time(0) / getpid()));

	for(i = 0; i < TOT_ITERATION; i++){
		sleep(randSleep(4));

		sem_wait(sem);

		/* CRITICAL REGION */
		if(sem_getvalue(sem, &v) == -1) printf("\t\tCHILD [%c]: Error sem_getvalue(): [%s]\n", id, strerror(errno));
		printf("\t\tCHILD [%c]: CRITICAL! [%d] Semaphore value: [%d]\n", id, i, v);

		sem_post(sem);
	}

	return(2);
}

void printWaitMsg(int ws)
{
	if     (WIFEXITED(ws)    != 0) printf("PARENT: WIFEXITED - Child terminated normally. Returned code: [%d]\n", WEXITSTATUS(ws));
	else if(WIFSIGNALED(ws)  != 0) printf("PARENT: WIFSIGNALED - Child received signal: [%d] %s\n", WTERMSIG(ws), (WCOREDUMP(ws) != 0 ? "COREDUMP" : ""));
	else if(WIFSTOPPED(ws)   != 0) printf("PARENT: WIFSTOPPED - Child process was stopped by signal: [%d]\n", WSTOPSIG(ws));
	else if(WIFCONTINUED(ws) != 0) printf("PARENT: WIFCONTINUED - Child process was resumed by SIGCONT\n");
	else                           printf("PARENT: Unknow child status returned\n");
}

int main(int argc, char *argv[])
{
	pid_t proc = 0;
	pid_t waitProc = 0;
	int wstatus = 0;
	int ret = 0;
	sem_t *sem;

	/* sem_open() could be done here for both processes, but I'm doing
	 * after fork() to simulate no related-processes (parent-child).

	sem = sem_open(SEM_NAMED, O_CREAT, S_IRWXU, 1);

	 */

	proc = fork();

	if(proc == -1){
		printf("Error fork(): [%s]\n", strerror(errno));
		ret = -1;
		goto clean_all;
	}

	if(proc == 0){
		pid_t cproc = 0;
		char cid = ' ';

		cproc = fork(); /* just another child to stress the semaphore */
		if(cproc == 0) cid = 'A';
		else           cid = 'B';

		printf("\t\tCHILD [%c]: Child process PID: [%d]\n", cid, getpid());

		sem = sem_open(SEM_NAMED, O_CREAT, S_IRWXU, 1);
		if(sem == SEM_FAILED){
			printf("\t\tCHILD [%c]: Error sem_open(): [%s]\n", cid, strerror(errno));
			ret = -1;
			goto clean_all;
		}

		ret = child(cid, sem);

		sem_close(sem);

		printf("\t\tCHILD [%c]: child returned.\n", cid);

		goto clean_all;
	}else{
		printf("PARENT: Parent process PID: [%d]\n", proc);

		sem = sem_open(SEM_NAMED, O_CREAT, S_IRWXU, 1);
		if(sem == SEM_FAILED){
			printf("PARENT: Error sem_open(): [%s]\n", strerror(errno));
			ret = -1;
			goto clean_all;
		}

		ret = parent(sem);

		sem_close(sem);

		printf("PARENT: Waiting child...\n");
		waitProc = waitpid(-1, &wstatus, 0); /* wait for all children */

		if(waitProc == -1){
			printf("PARENT: Error waitpid(): [%s]\n", strerror(errno));
			ret = -1;
			goto clean_all;
		}

		printWaitMsg(wstatus);
		printf("PARENT: child returned.");

		if(sem_unlink(SEM_NAMED) == -1){
			printf("PARENT: Error sem_unlink(): [%s]\n", strerror(errno));
			ret = -1;
			goto clean_all;
		}
	}

clean_all:
	return(ret);
}
