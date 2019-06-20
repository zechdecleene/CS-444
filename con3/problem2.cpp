#include <iostream>
#include <cstdlib>
#include <time.h>
#include <list>
#include <string>
#include <pthread.h>
#include <unistd.h>

using namespace std;




struct Data{
  list<int> linkedlist;
  pthread_mutex_t mutex;
};

void* deleterF(void*);
void* inserterF(void*);
void* searcherF(void*);

int main(){
  srand(time(NULL));
  //Initialize Data
  Data data;
  for(int i = 0; i <= 10; i++){
    data.linkedlist.push_back(i);
  }
  pthread_mutex_init(&data.mutex, NULL);

  //Create pthreads
  pthread_t searchers[3];
  pthread_t inserters[2];
  pthread_t deleter;

  //Start up threads
  for(int i = 0; i < 3; i++){
    pthread_create(&searchers[i], NULL, searcherF, &data);
  }
  for(int i = 0; i < 2; i++){
    pthread_create(&inserters[i], NULL, inserterF, &data);
  }
  pthread_create(&deleter, NULL, deleterF, &data);

  //Join threads
  for(int i = 0; i < 3; i++){
    pthread_join(searchers[i], NULL);
  }
  for(int i = 0; i < 2; i++){
    pthread_join(inserters[i], NULL);
  }
  pthread_join(deleter, NULL);


}

void* searcherF(void* newdata){
  Data* data = (struct Data*) newdata;
  while(1){
    int s = rand() % 6;
    sleep(s);
    pthread_mutex_lock(&data->mutex);
    int r = rand() % int(data->linkedlist.size());
    list<int>::iterator it =  data->linkedlist.begin();
    for(int i = 0; i < r; i++){
      it++;
    }
    cout << "Searcher:\tPrinting searched value: "<< *it << endl;
    pthread_mutex_unlock(&data->mutex);
  }
}

void* inserterF(void* newdata){
  struct Data* data = (struct Data*) newdata;
  while(1){
    int s = rand() % 6;
    sleep(s);
    pthread_mutex_lock(&data->mutex);
    int r = rand() % 11;
    cout << "Inserter:\tInserting " << r << " to linked list." << endl;
    data->linkedlist.push_back(r);
    cout << "Inserter:\tValue inserted." << endl;
    pthread_mutex_unlock(&data->mutex);
  }
}

void* deleterF(void* newdata){
  struct Data* data = (struct Data*) newdata;
  while(1){
    int s = rand() % 6;
    sleep(s);
    pthread_mutex_lock(&data->mutex);
    int r = rand() % int(data->linkedlist.size());
    list<int>::iterator it =  data->linkedlist.begin();
    for(int i = 0; i < r; i++){
      it++;
    }
    cout << "Deleter:\tRemoving " << *it << " from linked list." << endl;
    data->linkedlist.remove(r);
    cout << "Deleter:\tValue removed." << endl;
    pthread_mutex_unlock(&data->mutex);
  }
}
