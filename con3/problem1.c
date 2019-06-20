#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#define MAX_PROCESSES 3
#define NUM_PROCESSES 5

struct Data{
  int counter;
  int num_data;
  int num_users;
  int fullStatus; //0 = not full
  pthread_mutex_t mut;
  pthread_cond_t cond;
};

void* share(void*);

int main(){
  int i;
  srand(time(NULL));


  //Create threads
  pthread_t threads[NUM_PROCESSES];

  //Create shared data
  struct Data data;
  data.counter = 0;
  data.num_data = 12345;
  data.num_users = 0;

  //Initialize mutexes and conditionals
  pthread_mutex_init(&data.mut, NULL);
  pthread_cond_init(&data.cond, NULL);

  //Initialize the threads
  for(i = 0; i < NUM_PROCESSES; i++){
    pthread_create(&threads[i], NULL, share, &data);
    sleep(1);
  }

  //Join all threads
  for(i = 0; i < NUM_PROCESSES; i++){
    pthread_join(threads[i], NULL);
  }

}

void* share(void* newdata){
  struct Data*  data = (struct Data*) newdata;
  int i, me;
  pthread_mutex_lock(&data->mut);
  me = data->counter;
  data->counter += 1;
  pthread_mutex_unlock(&data->mut);
  while(1){
    int d;

    while(data->fullStatus == 1){
      pthread_mutex_lock(&data->mut);
      pthread_cond_wait(&data->cond, &data->mut);
    }
    data->num_users += 1; //Enters room
    if(data->num_users >= MAX_PROCESSES){
      printf("MAX USERS USING SHARED DATA\n");
      data->fullStatus = 1;
    }
    i = rand() % 6 + 4;
    sleep(i);
    d = data->num_data;
    printf("Num_users now: %d\n", data->num_users);

    data->num_users -= 1; //Leave the room
    if(data->num_users <=0 ){
      data->fullStatus = 0;
      pthread_cond_broadcast(&data->cond);
    }

    if(data->fullStatus == 0){
      printf("DATA OPEN TO BE SHARED\n");
      pthread_mutex_unlock(&data->mut);
    }
    printf("Thread %d is accessing the data: %d\n", me, d);
    i = rand() % 6 + 4;
    sleep(i);

  }

}
