#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#define LEFT me
#define RIGHT (me+1)%5

struct Philosohper{
  char* name;
};

struct Table{
  struct Philosohper philosohpers[5];
  int forks[5]; //0 means the fork is unavailable
  pthread_cond_t cond[5];
  pthread_mutex_t mut;
  int counter;
};

void* dine(void*);
void* waiter(void*);
void get_forks(struct Table*, int);
void return_forks(struct Table*, int);

void forkstatus(struct Table* table){
  int i = 0;
  for(i = 0; i < 5; i++){
    printf("Fork %d:\t%d\n", i, table->forks[i]);
  }
}

int main(int argc, char **argv){
  srand(time(NULL));
  //Create the threads. one for each Philosohper
  pthread_t threads[5];

  //Set the Table
  struct Table table;
  table.philosohpers[0].name = "Plato";
  table.philosohpers[1].name = "Aristotle";
  table.philosohpers[2].name = "Socrates";
  table.philosohpers[3].name = "Confucious";
  table.philosohpers[4].name = "Francis Bacon";
  table.counter = 0;

  //Initialize the forks
  int i;
  for(i = 0; i < 5; i++){
    table.forks[i] = 1;
    pthread_cond_init(&table.cond[i], NULL);
  }

  //Initialize the Mutex
  pthread_mutex_init(&table.mut, NULL);

  //Create threads
  for(i = 0; i < 5; i++){
    pthread_create(&threads[i], NULL, dine, &table);
  }

  for(i = 0; i < 5; i++){
    pthread_join(threads[i], NULL);
  }

}

void* dine(void* data){
  struct Table* table = (struct Table*) data;
  int r;
  //Use of mutex to assign each person's identity
  pthread_mutex_lock(&table->mut);
  int me = table->counter;
  table->counter += 1;
  pthread_mutex_unlock(&table->mut);
  while(1){
    r = rand() % 20 + 1;
    printf("%s is thinking for %ds.\n", table->philosohpers[me].name, r);
    sleep(r);
    get_forks(table, me);
    r = rand() % 8 + 2;
    printf("%s is eating for %ds.\n", table->philosohpers[me].name, r);
    sleep(r);
    return_forks(table, me);
  }
}



void get_forks(struct Table* table, int me){
  pthread_mutex_lock(&table->mut);
  while(table->forks[LEFT] == 0 && table->forks[RIGHT] == 0){
    pthread_cond_wait(&table->cond[me], &table->mut);
  }
  table->forks[LEFT] = 0;
  table->forks[RIGHT] = 0;
  pthread_mutex_unlock(&table->mut);
}

void return_forks(struct Table* table, int me){
  pthread_mutex_lock(&table->mut);
  table->forks[LEFT] = 1;
  table->forks[RIGHT] = 1;
  pthread_cond_signal(&table->cond[LEFT]);
  pthread_cond_signal(&table->cond[RIGHT]);
  pthread_mutex_unlock(&table->mut);
}
