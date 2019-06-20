#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#define N_CHAIRS 3
#define N_CUSTOMERS 5
#define N_BARBERS 1


void* get_hair_cut(void*);
void* cut_hair(void*);

struct Data{
  int chairsUsed;
  int cutTime;
  pthread_cond_t barber;
  pthread_cond_t done;
  pthread_mutex_t mut;
  pthread_mutex_t mut2;
};

int main(){
  int i;
  srand(time(NULL));
  i = N_CHAIRS;
  printf("NUMBER OF CHAIRS IN BARBERSHOP: %d\n", i);
  sleep(3);
  //Create threads
  pthread_t customer[N_CUSTOMERS];
  pthread_t barber;
    
  //Create DATA
  struct Data data;
  data.chairsUsed = 0;
  pthread_cond_init(&data.barber, NULL);
  pthread_cond_init(&data.done, NULL);
  pthread_mutex_init(&data.mut, NULL);
  pthread_mutex_init(&data.mut2, NULL);
  
  //Initialize threads
  pthread_create(&barber, NULL, cut_hair, &data);
  for(i = 0; i < N_CUSTOMERS; i++){
    pthread_create(&customer[i], NULL, get_hair_cut, &data);
  }
  
  //Join pthread_cond_signal
  pthread_join(barber, NULL);
  for(i = 0; i < N_CUSTOMERS; i++){
    pthread_join(customer[i], NULL);
  }  
  
}

//Customer
void* get_hair_cut(void* newdata){
  struct Data* data = (struct Data*) newdata;
  int r;
  while(1){
    pthread_mutex_lock(&data->mut);
    int openseat = 0;
    printf("Customers Seated: %d\n", data->chairsUsed);
    if(data->chairsUsed < 3){
      openseat = 1;
    }
    pthread_mutex_unlock(&data->mut);
    if(openseat){
      data->chairsUsed += 1;
      r = rand() % 4 + 2;
      data->cutTime = r;
      
      
      pthread_cond_signal(&data->barber);
      printf("Customer:\tGetting hair cut\n");
      pthread_cond_wait(&data->done, &data->mut2);
      printf("Customer\tDone getting hair cut!\n");
      data->chairsUsed -= 1;
    }else{
      printf("Customer\tShop full, customer left\n");
      r = rand() % 6 + 5;
      sleep(r);
    }
  }
}

//Barber
void* cut_hair(void* newdata){
  struct Data* data = (struct Data*) newdata;
  int r;
  while(1){
    pthread_cond_wait(&data->barber, &data->mut);
    printf("Barber\tCutting hair: %ds\n", data->cutTime);
    sleep(data->cutTime);
    pthread_cond_signal(&data->done);
    
  }
  
}