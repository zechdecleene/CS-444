#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#define MAX 9
#define MIN 2
#define MAX_BUFFER_SIZE 32

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
	mt[0]= s & 0xffffffffUL;
	for (mti=1; mti<N; mti++) {
		mt[mti] =
			(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		mt[mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length)
{
	int i, j, k;
	init_genrand(19650218UL);
	i=1; j=0;
	k = (N>key_length ? N : key_length);
	for (; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
			+ init_key[j] + j; /* non linear */
		mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
		i++; j++;
		if (i>=N) { mt[0] = mt[N-1]; i=1; }
		if (j>=key_length) j=0;
	}
	for (k=N-1; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
			- i; /* non linear */
		mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
		i++;
		if (i>=N) { mt[0] = mt[N-1]; i=1; }
	}

	mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}



/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N) { /* generate N words at one time */
		int kk;

		if (mti == N+1)   /* if init_genrand() has not been called, */
			init_genrand(5489UL); /* a default initial seed is used */

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}

	y = mt[mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}

unsigned int (*myrand)();

unsigned int rdrandom(){
  unsigned int value;
  __asm__ __volatile__(
                      "rdrand %[value]"
                      : [value] "=r" (value)
                      :
                      : "cc"
                      );
  return value;
}


unsigned int genrand32(){
  return genrand_int32();
}

struct args {
  long randomnum;
  long sleep_time;
};

struct queue{
    int num;
    int sleep;
};

struct buffer{
  pthread_mutex_t m;
  pthread_cond_t full;
  pthread_cond_t empty;
  struct queue Q[MAX_BUFFER_SIZE];
  int front;
  int back;
  int size;
};

void push(struct buffer* B, struct queue newQ){
    if(B->size > MAX_BUFFER_SIZE){
      printf("You done fucked\n");
      exit(1);
    }
    B->Q[B->back] = newQ;
    B->back = (B->back + 1) % MAX_BUFFER_SIZE;
    B->size++;
}

struct queue pop(struct buffer* B){
  if(B->size <= 0){
    printf("You done somethin fucky\n");
    exit(1);
  }
  struct queue tmpQ = B->Q[B->front];
  B->front = (B->front + 1) % MAX_BUFFER_SIZE;
  B->size--;
  return tmpQ;
}


void* consumer(void * data){
  struct buffer *QQ = (struct buffer*) data;
  while(1){
    pthread_mutex_lock(&QQ->m);
    while(QQ->size == 0){
      pthread_cond_wait(&QQ->empty, &QQ->m);
    }
    struct queue Q = pop(QQ);
    pthread_cond_signal(&QQ->full);
    pthread_mutex_unlock(&QQ->m);
    sleep(Q.sleep);
    printf("%d\n", Q.num);
  }
}

void* producer(void * data){
  struct buffer *QQ = (struct buffer*) data;
  while(1){
    sleep(myrand() % 4 + 3);
    struct queue Q;
    Q.num = myrand();
    Q.sleep = myrand() % 7 + 2;
    pthread_mutex_lock(&QQ->m);
    while(QQ->size == MAX_BUFFER_SIZE){
      pthread_cond_wait(&QQ->full, &QQ->m);
    }
    push(QQ, Q);
    pthread_cond_signal(&QQ->empty);
    pthread_mutex_unlock(&QQ->m);
  }
}



int main(int argc, char **argv)
{

	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
  int loop;

  struct buffer QQ;
  pthread_t threads[2];
  pthread_mutex_init(&QQ.m, NULL);
  pthread_cond_init(&QQ.full, NULL);
  pthread_cond_init(&QQ.empty, NULL);

  QQ.front = 0;
  QQ.back = 0;
  QQ.size = 0;



	eax = 0x01;

	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );


  if(ecx & 0x40000000){
  //use rdrand
  myrand = rdrandom;


  }
  else{
  //use mt19937
  unsigned long init[4] = {0x123, 0x234, 0x345, 0x456};
	unsigned long length = 4;
	init_by_array(init, length);

  myrand = genrand32;

  }


  pthread_create(&threads[0], NULL, producer, &QQ);
  pthread_create(&threads[1], NULL, consumer, &QQ);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);

	return 0;
}
