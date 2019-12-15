/* 
 * Operating Systems {2INCO} Practical Assignment
 * Threaded Application
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * Extra steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>          // for perror()
#include <pthread.h>

#include "uint128.h"
#include "flip.h"

// create a bitmask where bit at position n is set
#define BITMASK(n)          (((uint128_t) 1) << (n))

// check if bit n in v is set
#define BIT_IS_SET(v,n)     (((v) & BITMASK(n)) == BITMASK(n))

// set bit n in v
#define BIT_SET(v,n)        ((v) =  (v) |  BITMASK(n))

// clear bit n in v
#define BIT_CLEAR(v,n)      ((v) =  (v) & ~BITMASK(n))

// declare a mutex, and it is initialized as well
static pthread_mutex_t 		m_Thread 		  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t      m_Buffer          = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t       joinCond    	  = PTHREAD_COND_INITIALIZER;


static void * my_mutex_thread (void * arg);
static void startThreadsWithMutex (void);
static void initializeBuffer(void);
static void printBuffer(void);

int IdsToFollow = 0;
int threadsStarted = 0;

int main (void)
{
    // TODO: start threads to flip the pieces and output the results
    // (see thread_test() and thread_mutex_test() how to use threads and mutexes,
    //  see bit_test() how to manipulate bits in a large integer)

    //Setting the the bits to 1
	initializeBuffer();
	startThreadsWithMutex();



	printBuffer();
    return (0);
}


static void * my_mutex_thread (void * arg)
{
	//=================================> Flipping the bits
	struct threadType  *threadTypeTmp;
	threadTypeTmp = (struct threadType*) arg;
	int whichPices = threadTypeTmp->toFlip;; 

    printf ("        %lx: thread start; wanting to enter CS...\n", pthread_self());
    //pthread_mutex_lock (&mutex);
    
    for (int k = 1; k * whichPices <= NROF_PIECES; k++){
    	pthread_mutex_lock (&m_Buffer);
    	if (BIT_IS_SET(buffer[(k * whichPices)/128],(k*whichPices)%128)){

    		BIT_CLEAR(buffer[(k * whichPices)/128],(k*whichPices)%128);

    	}else {
    		BIT_SET(buffer[(k * whichPices)/128],(k*whichPices)%128);
    	}
    	pthread_mutex_unlock (&m_Buffer);
    }

    printf ("        %lx: done flipping bits...\n", pthread_self());
    
    //=================================> Signaling that the thread has ended

    bool hasEnded = false;
    while (hasEnded == false){
    	
    	pthread_mutex_lock(&m_Thread);
    	
    	if (IdsToFollow > NROF_THREADS){
    		IdsToFollow = threadTypeTmp->id;
    		threadsStarted--;
    		pthread_cond_signal(&joinCond);
    		hasEnded = true;
    	}

    	pthread_mutex_unlock(&m_Thread);
    }
    //pthread_mutex_unlock (&mutex);
    
    return (NULL);
}


static void startThreadsWithMutex (void)
{
    pthread_t   my_threads[NROF_THREADS];
    struct threadType threadsInfo[NROF_THREADS];
    int pieces=2;
	int threadId =0;
	//starting the first 10 threads
    for (int k = 0; k< NROF_THREADS - 1; k++){
    	
    	pthread_mutex_lock(&m_Thread);
    	
    	//adding its info
    	threadsInfo[k].id = threadId;
    	threadsInfo[k].toFlip = pieces;
    	
	   	printf (" pieces :");
		printf("%d\n",pieces);
		printf (" threadID :");
		printf("%d\n",threadId);
		printf (" threads Started ");
		printf("%d\n",threadsStarted);
    	//Starting the thread
    	printf ("%lx: starting thread ...", pthread_self());
	    printf("%d\n",k);
	    pthread_create (&my_threads[threadId], NULL, my_mutex_thread, (void*) &threadsInfo[threadId]);
	    pieces++;
    	threadId ++;
    	threadsStarted++;
	   

	    pthread_mutex_unlock(&m_Thread);
    }

 
    printf (" pieces :");
	printf("%d\n",pieces);
	printf (" threadID :");
	printf("%d\n",threadId);
	printf (" threads Started ");
	printf("%d\n",threadsStarted);
    //Another increment to not overwright the 10th thread
    
    //Checking whether one of the threads has finished and start another one in his place
    while( pieces <= NROF_PIECES) {

    	pthread_mutex_lock(&m_Thread);
    	//Check if the NROF_Threads was reached;

    	while (threadsStarted >= NROF_THREADS){
    		
    		pthread_cond_wait(&joinCond,&m_Thread);
    		pthread_join(my_threads[IdsToFollow],NULL);
    		threadId = IdsToFollow;

    	}
    	threadsStarted++;

    	
    	//Start a new thread 
    	
    	threadsInfo[threadId].toFlip = pieces;
    	threadsInfo[threadId].id = threadId;
	    printf ("%lx: starting thread ...", pthread_self());
	    printf("%d\n",threadsStarted);
	    pthread_create (&my_threads[threadId], NULL, my_mutex_thread,(void*)&threadsInfo[threadId]);
	    
	    IdsToFollow = NROF_THREADS + 1;
	    
	    pieces++;
	    pthread_mutex_unlock(&m_Thread);
    }
    
    // wait for threads, but we are not interested in the return value 
    //	Join the threads

    while (threadsStarted > 0){
    	pthread_mutex_lock(&m_Thread);

    	pthread_cond_wait(&joinCond,&m_Thread);
    	pthread_join(my_threads[IdsToFollow],NULL);
    	IdsToFollow = NROF_THREADS + 1;

    	pthread_mutex_unlock(&m_Thread);
    }
    
    printf ("%lx: threads ready\n", pthread_self());
    printf ("\n");

    
}

static void initializeBuffer(void){
	// Setting the bits to 1
	for(int s = 0; s <= NROF_PIECES; s++) {

		BIT_SET(buffer[s/128],s%128);
	}
}

static void printBuffer(void){
	for (int i = 1; i<=NROF_PIECES; i++){
		if (BIT_IS_SET(buffer[i/128],i%128)){
			printf("%d\n", i);
		}
	}
}