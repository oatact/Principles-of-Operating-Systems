#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <assert.h>
typedef int buffer_item;
#define BUFFER_SIZE 3
#define TRUE 1
double prepairTurnAround;
int workingOn;
pthread_mutex_t mutex;           //the mutex lock
sem_t full, empty;               //the semaphores
buffer_item buffer[BUFFER_SIZE]; //the buffer
int counter;                     //buffer counter
pthread_t tid;                   //Thread ID
pthread_t tid2;                  //Thread ID
pthread_attr_t attr;             //Set of thread attributes
void *cooker(void *cno);         //the producer thread
void *prepairer(void *pno);      //the prepairer thread
void setUpData(int item);

double ppTime;
struct timespec start, finish;
char orderName[20];

//assign attribute name

int ingeNum;
char ingredientName1[20];
char ingredientName2[20];
char ingredientName3[20];
int prepairingTime1;
int prepairingTime2;
int prepairingTime3;
char cookingMedthod1[20];
char cookingMedthod2[20];
char cookingMedthod3[20];
int cookingTime1;
int cookingTime2;
int cookingTime3;

void initializeData()
{
    pthread_mutex_init(&mutex, NULL); //create the mutex lock
    //Create the full semaphore and initialize to 0
    sem_init(&full, 0, 0);
    //Create the empty semaphore and initialize to BUFFER_SIZE
    sem_init(&empty, 0, BUFFER_SIZE);
    pthread_attr_init(&attr); //get the default attributes
    counter = 0;              //init buffer
}
// reset buffer
void reset()
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = 0;
    }
    counter = 0;
}

// Add an item to the buffer
int insert_item(buffer_item item)
{
    //When the buffer is not full add the item and increment the counter
    if (counter < BUFFER_SIZE)
    {
        buffer[counter] = item;
        counter++;
        return 0;
    }
    else
    { //Error if the buffer is full
        return -1;
    }
}
// Remove an item from the buffer
int remove_item(buffer_item *item)
{
    /* When the buffer is not empty remove the item
 and decrement the counter */
    if (counter > 0)
    {
        *item = buffer[(counter - 1)];
        counter--;
        return 0;
    }
    else
    { // Error buffer empty
        return -1;
    }
}
// Producer Thread
void *cooker(void *cno)
{

    buffer_item item;
    int item_;
    int finished = 0;
    long tid;
    tid = (long)cno;

    item_ = workingOn;
    setUpData(item_);
    printf("\n  ↳ <THREAD %ld>: WAIT ingredient for order %s\n", tid, orderName);

    while (finished < ingeNum)
    {

        item = (buffer_item)finished + 1;
        printf("");
        sem_wait(&full);            // acquire the empty lock
        pthread_mutex_lock(&mutex); // acquire the mutex lock

        int sleepTime;
        char tempIngName[20];
        char tempCookingMedthod[20];

        // switch data by ingredient
        switch (item)
        {
        case 1:
            strncpy(tempIngName, ingredientName1, 20);
            strncpy(tempCookingMedthod, cookingMedthod1, 20);
            sleepTime = cookingTime1;
            break;
        case 2:
            strncpy(tempIngName, ingredientName2, 20);
            strncpy(tempCookingMedthod, cookingMedthod2, 20);
            sleepTime = cookingTime2;
            break;
        case 3:
            strncpy(tempIngName, ingredientName3, 20);
            strncpy(tempCookingMedthod, cookingMedthod3, 20);
            sleepTime = cookingTime3;
            break;
        default:
            break;
        }

        if (remove_item(&item))
        {
            // fprintf(stderr, "Producer report error condition\n");
        }
        else
        {
            if (sleepTime != 0)
                sleep(sleepTime);
            fprintf(stderr, "   ↳ <THREAD %ld>: finish %s %s\n", tid, tempCookingMedthod, tempIngName);
            finished++;
        }

        pthread_mutex_unlock(&mutex); //release the mutex lock
        sem_post(&empty);             //signal full
    }
    pthread_exit(0);
}
// prepairer Thread
void *prepairer(void *pno)
{

    buffer_item item;
    int item_;
    long tid;
    tid = (long)pno;
    int finished = 0;

    // select order
    item_ = workingOn;
    setUpData(item_);

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (finished < ingeNum)
    {
        item = (buffer_item)finished + 1;
        sem_wait(&empty);           //acquire the empty lock
        pthread_mutex_lock(&mutex); //acquire the mutex lock

        char tempIngName[20];
        int sleepTime;
        // switch data by ingredient
        switch (item)
        {
        case 1:
            strncpy(tempIngName, ingredientName1, 20);
            sleepTime = prepairingTime1;
            break;
        case 2:
            strncpy(tempIngName, ingredientName2, 20);
            sleepTime = prepairingTime2;
            break;
        case 3:
            strncpy(tempIngName, ingredientName3, 20);
            sleepTime = prepairingTime3;
            break;
        default:
            break;
        }
        if (sleepTime != 0)
            sleep(sleepTime);
        // item = i;
        insert_item((buffer_item)item);
        printf("   ↳ <THREAD %ld>: finish prepaired ingredient %s\n", tid, tempIngName);
        finished++;
        pthread_mutex_unlock(&mutex); //release the mutex lock
        sem_post(&full);              //signal full
    }
    pthread_exit(0);
}
int main(int argc, char *argv[])
{

    long i;           //loop counter
    initializeData(); //Initialize the app
    double listTurnAroundTime[argc - 1];
    double avgTurnTime = 0;

    // Verify the correct number of arguments were passed in
    if (argc > 6 || argc == 1)
    {
        fprintf(stderr, "\n ❌❌❌ Incorrect usage command ❌❌❌ \n");
        fprintf(stderr, "Please see the order ID from 'menu.txt' \n");
        fprintf(stderr, "(maximum order is 5) USAGE:./main.out <orderID> <orderID> .... <orderID>\n\n");
        exit(-1);
    }
    struct timespec start, finish;

    for (int i = 1; i <= argc - 1; i++)
    {
        workingOn = atoi(argv[i]);
        setUpData(workingOn);
        clock_gettime(CLOCK_MONOTONIC, &start);
        printf("---------------------------------------------");
        printf("\n<THREAD %d>: PUT %s\n", 1, orderName);
        printf("<THREAD %d>: <ORDER :%s>, START\n", 1, orderName);
        pthread_create(&tid, &attr, prepairer, (void *)1);
        pthread_create(&tid2, &attr, cooker, (void *)2);
        pthread_join(tid, NULL);
        pthread_join(tid2, NULL);
        clock_gettime(CLOCK_MONOTONIC, &finish);
        ppTime = (finish.tv_sec - start.tv_sec);
        ppTime += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        //    pthread_join(tid2, NULL);
        printf("\n<THREAD %d>: <ORDER :%s>, FINISH\n", 2, orderName);
        printf("<ORDER %s>: SERVED, <%f SEC>\n", orderName, ppTime);
        listTurnAroundTime[i - 1] = ppTime;
        reset();
    }

    for (int i = 0; i < argc - 1; i++)
    {
        avgTurnTime += listTurnAroundTime[i];
    }

    printf("\nAVERAGE TURNAROUND TIME: %f SEC\n\n", avgTurnTime / (argc - 1));
    exit(0);
}

void setUpData(int item)
{
    switch (item)
    {
    case 1: // menu id 1: Chachu Ramen (hard)

        // total ingredient number
        ingeNum = 3;
        strncpy(orderName, "Chachu Ramen", 20); // ingredient name

        // ingredient data 1
        strncpy(ingredientName1, "noodle", 20);
        prepairingTime1 = 4;
        strncpy(cookingMedthod1, "boil", 20);
        cookingTime1 = 3;

        // ingredient data 2
        strncpy(ingredientName2, "chashu", 20);
        prepairingTime2 = 3;
        strncpy(cookingMedthod2, "grill", 20);
        cookingTime2 = 4;

        // ingredient data 3
        strncpy(ingredientName3, "shoyu", 20);
        prepairingTime3 = 5;
        strncpy(cookingMedthod3, "mixing", 20);
        cookingTime3 = 4;

        break;
    case 2: // menu id 2: kurobuta rice (easy)
        ingeNum = 2;
        strncpy(orderName, "Kurobuta Rice", 20);

        strncpy(ingredientName1, "pork", 20);
        prepairingTime1 = 3;
        strncpy(cookingMedthod1, "grill", 20);
        cookingTime1 = 3;

        strncpy(ingredientName2, "rice", 20);
        prepairingTime1 = 3;
        strncpy(cookingMedthod2, "take", 20);
        cookingTime2 = 0;

        /* code */
        break;
    case 3: // menu id 3: American rice (easy)
        ingeNum = 2;
        strncpy(orderName, "American Rice", 20);

        strncpy(ingredientName1, "meat", 20);
        prepairingTime1 = 5;
        strncpy(cookingMedthod1, "grill", 20);
        cookingTime1 = 6;

        strncpy(ingredientName2, "rice", 20);
        prepairingTime2 = 3;
        strncpy(cookingMedthod2, "take", 20);
        cookingTime2 = 0;

        /* code */
        break;
    case 4: // menu id 4: Mama with egg Ramen (hard)

        // total ingredient number
        ingeNum = 3;
        strncpy(orderName, "Mama with egg Ramen", 20); // ingredient name

        // ingredient data 1
        strncpy(ingredientName1, "mama noodle", 20);
        prepairingTime1 = 3;
        strncpy(cookingMedthod1, "boil", 20);
        cookingTime1 = 3;

        // ingredient data 2
        strncpy(ingredientName2, "egg", 20);
        prepairingTime2 = 5;
        strncpy(cookingMedthod2, "boil", 20);
        cookingTime2 = 3;

        // ingredient data 3
        strncpy(ingredientName3, "vegetable", 20);
        prepairingTime3 = 5;
        strncpy(cookingMedthod3, "take", 20);
        cookingTime3 = 0;

        break;
    default:
        break;
    }
}
