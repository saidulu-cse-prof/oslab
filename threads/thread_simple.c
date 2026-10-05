#include <stdio.h>
#include <pthread.h>

void *thread_function(void *arg){
printf("Hello From the thread\n");
printf("Thread ID : %lu\n", (unsigned long)pthread_self());
return NULL;
}

int main(){
	pthread_t thread;
	printf("Main thread started.\n");
	pthread_create(&thread, NULL,thread_function,NULL);
	pthread_join(thread,NULL);
	printf("Main thread finished\n");
	return 0;

}