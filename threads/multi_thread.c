#include<stdio.h>
#include<pthread.h>

int shared=10;

void *thread1_function(void *arg){
	printf("Thread 1 : initial value of shared = %d\n", shared);
    shared += 20;
	printf("Thread 1 : Modified value of shared = %d\n", shared);
}

void *thread2_function(void *arg){
	printf("Thread 2 : current value of shared = %d\n", shared);
    shared += 30;
	printf("Thread 2 : Modified value of shared = %d\n", shared);
}

int main(){
	pthread_t t1, t2;
	printf("Main thread : initial value of shared = %d\n", shared);
	pthread_create(&t1, NULL,thread1_function,NULL);
	pthread_create(&t2, NULL,thread2_function,NULL);
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	printf("Main thread : Final value of shared = %d\n", shared);
	return 0;

}