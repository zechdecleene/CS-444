#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

void* tobacco_guy(void*);
void* paper_guy(void*);
void* matches_guy(void*);
void* agent_guy(void*);

struct Data{
  int tOut;
  int pOut;
  int mOut;
  int go;
  pthread_mutex_t mut;
  pthread_cond_t cond;
  pthread_cond_t smoking;
  pthread_cond_t ready;
};

int main(){
  int i;
  srand(time(NULL));
  printf("---Smoker Problem---\n");
  //Create threads
  pthread_t tobacco, paper, matches;
  pthread_t agent;

  //Create DATA
  struct Data data;
  data.tOut = 0;
  data.mOut = 0;
  data.pOut = 0;
  data.go = 0;
  pthread_mutex_init(&data.mut, NULL);
  pthread_cond_init(&data.cond, NULL);
  pthread_cond_init(&data.smoking, NULL);
  pthread_cond_init(&data.ready, NULL);

  //Initialize threads
  pthread_create(&tobacco, NULL, tobacco_guy, &data);
  pthread_create(&paper, NULL, paper_guy, &data);
  pthread_create(&matches, NULL, matches_guy, &data);
  pthread_create(&agent, NULL, agent_guy, &data);

  //Join pthread_cond_signal
  pthread_join(tobacco, NULL);
  pthread_join(paper, NULL);
  pthread_join(matches, NULL);
  pthread_join(agent, NULL);

}

void* tobacco_guy(void* newdata){
  struct Data* data = (struct Data* ) newdata;
  int r;
  int wait;
  char* name = "tobacco";
  while(1){
    pthread_mutex_unlock(&data->mut);
    // printf("T\n");
    wait = 0;
    pthread_mutex_lock(&data->mut);
    printf("%s\n", name);
    if(data->mOut == 0 || data->pOut == 0){
      data->tOut = 1;
    }else{
      wait = 1;
    }
    pthread_mutex_unlock(&data->mut);
    if(wait){
      pthread_cond_signal(&data->ready);
      pthread_cond_wait(&data->cond, &data->mut);
      printf("The %s guy walks to the agent with the other two ingredients and makes the cigarrette\n", name);
    }else{
    pthread_cond_wait(&data->smoking, &data->mut);
    }
    r = rand() % 4 + 2;
    sleep(r);
  }
}

void* paper_guy(void* newdata){
  struct Data* data = (struct Data* ) newdata;
  int r;
  int wait;
  char *name = "paper";
  while(1){
    pthread_mutex_unlock(&data->mut);
    // printf("P\n");
    wait = 0;
    pthread_mutex_lock(&data->mut);
    printf("%s\n", name);
    if(data->mOut == 0 || data->tOut == 0){
      data->pOut = 1;
    }else{
      wait = 1;
    }
    pthread_mutex_unlock(&data->mut);
    if(wait){
      pthread_cond_signal(&data->ready);
      pthread_cond_wait(&data->cond, &data->mut);
      printf("The %s guy walks to the agent with the other two ingredients and makes the cigarrette\n", name);
    }else{
    pthread_cond_wait(&data->smoking, &data->mut);
    }
    r = rand() % 4 + 2;
    sleep(r);
  }
}

void* matches_guy(void* newdata){
  struct Data* data = (struct Data* ) newdata;
  int r;
  int wait;
  char* name = "matches";
  while(1){
    pthread_mutex_unlock(&data->mut);
    // printf("M\n");
    wait = 0;
    pthread_mutex_lock(&data->mut);
    printf("%s\n", name);
    if(data->tOut == 0 || data->pOut == 0){
      data->mOut = 1;
    }else{
      wait = 1;
    }
    pthread_mutex_unlock(&data->mut);
    if(wait){
      pthread_cond_signal(&data->ready);
      pthread_cond_wait(&data->cond, &data->mut);
      printf("The %s guy walks to the agent with the other two ingredients and makes the cigarrette\n", name);
    }else{
    pthread_cond_wait(&data->smoking, &data->mut);
    }
    r = rand() % 4 + 2;
    sleep(r);

  }
}

void* agent_guy(void* newdata){
  struct Data* data = (struct Data* ) newdata;
  int i;
  int counter;
  int p,t,m;
  char* name = "agent";
  while(1){
    pthread_cond_wait(&data->ready, &data->mut);
    printf("The gang comes together to smoke a cigarrette\n");
    counter = 0;
    p = 0;
    t = 0;
    m = 0;
    if(data->tOut == 1){
      counter++;
      t = 1;
    }
    if(data->pOut == 1){
      counter++;
      p = 1;
    }
    if(data->mOut == 1){
      counter++;
      m = 1;
    }

    if(counter >= 2){
      data->mOut = 0;
      data->pOut = 0;
      data->tOut = 0;
      pthread_cond_signal(&data->cond);
      printf("The Agent aquired the ingredients and signals the last guy to make the cigarrette\n");
      pthread_cond_broadcast(&data->smoking);
    }
  }
}
