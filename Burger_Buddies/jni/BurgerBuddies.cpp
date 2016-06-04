#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>                 // for rand()
#include <unistd.h>		    // for sleep()
const int MAXTIME_BURGER = 4;
const int MAXTIME_SLEEP = 3;
const int MAXTIME_ARRIVE = 10;
const int MAXTIME_ORDER = 2;
const int MAXPTHE = 100;
int custrNum = 0, restCus = 0;      
/* "custrNum" means the number of the current customers;
   "restCus" means the number of customers who has not been satisfied. */
sem_t rack, order, cashr, burg, mutexC, mutexR;
/* "rack" means the number of empty racks;
   "order" means the number of orders maked by customers;
   "cashr" means the number of free cashiers;
   "burg" means the number of burgers maked by cooks;
   "mutexC" is for protect "custrNum" when it changes;
   "mutexR" is for protect "restCus" when it changes. */

void *Cooks(void *param)
{
    int num = *(int*)param;         // the number of Cook
    while (restCus != 0)	    // need to work
    {
        sem_wait(&rack);
        if (restCus == 0) break;
        // start to make a burger
        sleep(rand() % MAXTIME_BURGER);
        sem_post(&burg);
        printf("Cook [%d] make a burger.\n", num);
    }
    printf("Cook [%d] rests.\n", num);
    for (int i = 0; i < MAXPTHE; ++i)// to prevent cashiers' starvation 
    {
	sem_post(&order);
	sem_post(&burg);
    }
    pthread_exit(0);
}

void *Cashiers(void *param)
{
    int num = *(int*)param;
    while (restCus != 0)
    {
        while (custrNum == 0 && restCus != 0)
        {
            printf("Casher [%d] goes to sleep.\n", num);
            sleep(rand() % MAXTIME_SLEEP);
        }
        if (restCus == 0) break;
        sem_wait(&order);
	if (restCus == 0) break;
        printf("Casher [%d] accepts an order.\n", num);
       
        sem_wait(&burg);
        sem_post(&rack);
        printf("Casher [%d] take a burger to customer.\n", num);
        sem_post(&cashr);

	/* There are NO synchronization constraints for a cashier
           presenting food to the customer. */
	sem_wait(&mutexR);
    	--restCus;
    	sem_post(&mutexR);
    }
    printf("Casher [%d] rests.\n", num);
    pthread_exit(0);
}

void *Customers(void *param)
{
    int num = *(int*)param;
    
    // distribute the arrival time
    sleep(rand() % MAXTIME_ARRIVE);
    sem_wait(&mutexC);
    ++custrNum;
    sem_post(&mutexC);
    printf("Customer [%d] come.\n", num);
    
    sem_wait(&cashr);
    // start to make an order
    sleep(rand() % MAXTIME_ORDER);
    sem_post(&order);
    
    sem_wait(&mutexC);
    --custrNum;
    sem_post(&mutexC);
    //printf("%d customers have not get a burger.\n", restCus);
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int C, R, M, B, i, j, k, ret;
    int numList[MAXPTHE];
    pthread_t CookP[MAXPTHE], CashierP[MAXPTHE], CustomerP[MAXPTHE];
    
    // set the initial values
    C = atoi(argv[1]);
    M = atoi(argv[2]);
    B = atoi(argv[3]);
    R = atoi(argv[4]);
    custrNum = 0;
    restCus = B;
    for (i = 1; i < MAXPTHE; ++i)
	numList[i] = i;
    sem_init(&rack, 0, R);
    sem_init(&order, 0, 0);
    sem_init(&cashr, 0, M);
    sem_init(&burg, 0, 0);
    sem_init(&mutexC, 0, 1);
    sem_init(&mutexR, 0, 1);
    
    // begin run
    printf("Cooks [%d], Cashiers [%d], Customers [%d]\n", C, M, B);
    printf("Begin run.\n");
    srand(time(NULL));
    
    // create theads
    for (i = 1; i <= C; ++i)
    {
        ret = pthread_create(&CookP[i], NULL, &Cooks, &numList[i]);
        if (ret != 0)
        {
            printf("Create Cook pthread error!\n");
            return 0;
        }
    }
    for (j = 1; j <= M; ++j)
    {
        ret = pthread_create(&CashierP[j], NULL, &Cashiers, &numList[j]);
        if (ret != 0)
        {
            printf("Create Cashier pthread error!\n");
            return 0;
        }
    }
    for (k = 1; k <= B; ++k)
    {
        ret = pthread_create(&CustomerP[k], NULL, &Customers, &numList[k]);
        if (ret != 0)
        {
            printf("Create Customer pthread error!\n");
            return 0;
        }
    }
    
    // end theads
    for (i = 1; i <= C; ++i)
        pthread_join(CookP[i], NULL);
    for (j = 1; j <= M; ++j)
        pthread_join(CashierP[j], NULL);
    for (k = 1; k <= B; ++k)
        pthread_join(CustomerP[k], NULL);
    
    printf("All customers have been satisfied!\n");
    return 0;
}
