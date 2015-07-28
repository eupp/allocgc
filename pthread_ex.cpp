#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

typedef struct data
{
 char name[10];
 int age;
}data;

void sig_func(int sig)
{
 fprintf(stderr, "catch\n");
 write(1, "Caught signal 11\n", 17);
 // signal(SIGSEGV,sig_func);
}

void * func(void *p)
{
 data * d = (data*)p;
 fprintf(stderr, "This is from thread function\n");
 sleep(20000);
 fprintf(stderr, "wakedup\n");
 strcpy(d->name,"Mr. Linux");
 d->age=30;
 sleep(2); // Sleep to catch the signal
}

int main()
{
 pthread_t tid;
 pthread_attr_t attr;
 data d;
 data *ptr = &d;

 signal(SIGSEGV,sig_func); // Register signal handler before going multithread
 pthread_attr_init(&attr);
 pthread_create(&tid,&attr,func,(void*)ptr);
 sleep(1); // Leave time for initialisation
 pthread_kill(tid,SIGSEGV);

 pthread_join(tid,NULL);
 fprintf(stderr, "Name:%s\n",ptr->name);
 fprintf(stderr, "Age:%d\n",ptr->age);
}