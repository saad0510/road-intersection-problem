/*
 * author:         Saad Bin Khalid
 * rollno:         20k-0161
 * section:        F
 * batch:          BSCS - sophomore
 * email:          ayyansaad46@gmail.com
 *                 k200161@nu.edu.pk
 * last-modified:  14 April 2022
 */

#ifndef _ROAD_H_
#define _ROAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef unsigned char bool;
#define TRUE (1)
#define FALSE (0)

#define ROAD_DEFAULT_SIZE (10)
#define ROAD_DEFAULT_SPEED (10000)
#define ROAD_WTOE (0) // west to east
#define ROAD_NTOS (1) // north to south

#define ACQUIRE(mutex) (pthread_mutex_lock(&mutex))
#define RELEASE(mutex) (pthread_mutex_unlock(&mutex))

// the direction of traffic on a road
enum direction
{
    north_to_south,
    west_to_east,
    none,
};

// a single block of a road. it can hold only one car at a time
struct block
{
    pthread_t car;  // the car thread routine
    bool available; // true when block is occupied
};

// the road of cars
struct road
{
    unsigned size;      // number of blocks
    unsigned car_count; // number of cars on the road
    unsigned speed;     // speed of each car

    enum direction dir;    // allowed direction
    struct block *blocks;  // blocks of road
    pthread_mutex_t lock;  // mutex-lock for internel synchronizations
};


/*
 * @brief initializes the road with proper values and empty blocks. necessary to run this function before proceeding any futher.
 * @param proad pointer to road structure to initialize
 * @param size the number of blocks on the road
 * @returns nothing
 * @throws when there is no memory available in heap
 */
void road_init(struct road *proad, unsigned size, unsigned dir)
{
    proad->size = size;
    proad->car_count = 0;
    proad->speed = ROAD_DEFAULT_SPEED;

    proad->dir = dir;
    proad->blocks = (struct block *)malloc(size * sizeof(struct block));

    for (unsigned i = 0; i < size; i++)
        proad->blocks[i].available = TRUE;
    pthread_mutex_init(&proad->lock, 0);
}

/*
 * @brief releases the resources hold by the road
 * @param proad pointer to road structure to destroy
 * @returns nothing
 * @throws when the road is already destroyed
 */
void road_destroy(struct road *proad)
{
    free(proad->blocks);
    proad->blocks = NULL;
    pthread_mutex_destroy(&proad->lock);
}

/*
 * @brief moves each car one step ahead on the road. the function returns if the road is already full.
 * @param proad pointer to road structure to move
 * @returns nothing
 * @throws nothing
 */
void road_move(struct road *proad)
{
    // read number of cars:
    ACQUIRE(proad->lock);
        unsigned car_count = proad->car_count;
    RELEASE(proad->lock);

    if (car_count == proad->size) // road is full?
        return;

    unsigned i = proad->size - 1;               // start from end
    while (proad->blocks[i].available == FALSE) // already a car?
        i--;                                    // move back
    // now, i points to the fist empty position
    // note: i cannot be < 0 because road will never be full at this point
    // starting from i, move all cars one block ahead:
    while (i > 0)
    {
        proad->blocks[i].available = proad->blocks[i - 1].available;
        proad->blocks[i].car = proad->blocks[i - 1].car;
        i--;
    }
    proad->blocks[0].available = TRUE; // starting block will be empty
}

/*
 * @brief inserts a new car at the front of road. the function also waits until front is empty
 * @param proad pointer to road structure
 * @param car the car thread routine
 * @returns nothing
 * @throws nothing
 */
void road_push(struct road *proad, pthread_t car)
{
    while (proad->blocks[0].available == FALSE) // start is full?
        usleep(100000);

    ACQUIRE(proad->lock);
        proad->car_count++;
    RELEASE(proad->lock);

    proad->blocks[0].available = FALSE;
    proad->blocks[0].car = car;

}

/*
 * @brief removes a car from the end of road.
 * @param proad pointer to road structure
 * @returns nothing
 * @throws nothing
 */
void road_pop(struct road *proad)
{
    proad->blocks[proad->size - 1].available = TRUE;

    ACQUIRE(proad->lock);
        proad->car_count--;
    RELEASE(proad->lock);
}

/*
 * @brief moves the traffic on the road, like a stream of water. this is thread routine. run this as a separate thread. to exit the thread, use pthread_cancel()
 * @param arg pointer to the road structure
 * @returns nothing
 * @throws nothing
 */
void *road_stream(void *arg)
{
    struct road *proad = (struct road *)arg;
    while (TRUE) // infinte loop
    {
        road_move(proad);
        pthread_testcancel(); // exit?
        usleep(proad->speed * 10);
    }
    return NULL;
}

#endif