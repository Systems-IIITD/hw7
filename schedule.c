#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ucontext.h>

struct thread {
	char *rsp;
	ucontext_t context;
	int tid;
	int valid;
};

#define MAX_THREADS 16
struct thread listThreads[MAX_THREADS] = {0};
struct thread *curThread = NULL;
static int numThreads = 0;
ucontext_t errContext;

void
errorfn(void)
{
	printf ("This line shlould not be printed!\n");
}

struct thread *
nextthread (int tid)
{
	int i, nexttid;
	for (i = 1; i <= MAX_THREADS; i++) {
		nexttid = (tid + i) % MAX_THREADS;
		if (listThreads[nexttid].valid) {
			return &listThreads[nexttid];
		}
	}
	return NULL;
}

void
schedule (int signal)
{
	struct thread *nextThread = nextthread (curThread->tid);
	struct thread *cur;

	if (nextThread == NULL) {
		printf ("no thread to schedule!\n");
		exit (0);
	}
	alarm (1);
	if (nextThread != curThread) {
		cur = curThread;
		curThread = nextThread;
		swapcontext (&cur->context, &curThread->context);
	}
}

void thread1 ()
{
	long long i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 1000000000; j++);
		printf ("thread1 is running\n");
	}
}

void thread2 ()
{
	long long i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 1000000000; j++);
		printf ("thread2 is running\n");
	}
}

void
default_handler (void(*fnptr)(), int *valid)
{
	fnptr ();
	valid[0] = 0;
	kill (getpid(), SIGALRM);
}

int
create_context (ucontext_t *context, void(*fnptr)(), int *valid)
{
	char *rsp = malloc (4096);
	if (rsp == NULL) {
		return 0;
	}
	getcontext(context);
	context->uc_stack.ss_sp = (void*)(rsp + 4096);
	context->uc_stack.ss_size = 4096;
	context->uc_link = &errContext;
	makecontext (context, (void*)default_handler, 2, fnptr, valid);
	return 1;
}


void
create_thread (void(*fnptr)())
{
	ucontext_t *context;

	if (numThreads >= MAX_THREADS) {
		printf ("Unable to create more threads!\n");
		return;
	}
	context = &listThreads[numThreads].context;
	if (!create_context (context, fnptr, &listThreads[numThreads].valid)) {
		return;
	}
	listThreads [numThreads].tid = numThreads;
	listThreads [numThreads].valid = 1;
	numThreads++;
}

int main ()
{
	create_context (&errContext, errorfn, NULL);
	signal (SIGALRM, schedule);
	memset (listThreads, 0, sizeof(listThreads));
	create_thread (thread1);
	create_thread (thread2);
	curThread = &listThreads[0];
	alarm (1);
	setcontext (&curThread->context);
	return 0;
}
