#include "road.h"
#include "header.h"
#define MAX_CARS_IN_INTERSECTION (3)

sem_t mutex;
sem_t lock;
enum direction inside = none;

/*
 * @brief animates the roads on stdout. this is thread routine. run this as a separate thread. to exit the thread, use pthread_cancel()
 * @param arg pointer to the road structure(s)
 * @returns nothing
 * @throws nothing
 */
void *road_print(void *arg);

// car thread routine - it is supposed to run when the car is inside the intersection
void intersect_routine()
{
    sleep(1);
}

/*
 * @brief car thread routine. it is supposed to run when the car is on the road. this is thread routine. run this as a separate thread.
 * @param arg pointer to the road structure
 * @returns nothing
 * @throws nothing
 */
void *carthread(void *arg)
{
    struct road *proad = (struct road *)arg;
    pthread_t threadID = pthread_self(); // get thread id

    while (proad->blocks[proad->size - 1].car != threadID) // the car has reached the end of road?
        usleep(100000);

front_gate: // entry point of all threads
            // your competitor (opposite road) threads are X threads

    while (inside != proad->dir) // X is inside?
    {
        if (sem_trywait(&mutex) == 0) // try to acquire lock
            break;                    // break if acquired
    }
    // you are inside!
    inside = proad->dir; // stop X from getting in

    while (TRUE)
    {
        if (sem_trywait(&lock) == 0) // allow only 3 threads at a time
            break;                   // break if acquired
        // your threads are waiting for their turns
        // meanwhile...
        usleep(1000000);          // give X a chance to get in
        if (inside != proad->dir) // if X gets in
            goto front_gate;      // retreat your threads
    }
    /* INTERSECTION STARTS */

    road_pop(proad);     // exit the road
    intersect_routine(); // enter intersection
    sem_post(&lock);     // allow more threads

    int lock_count;
    sem_getvalue(&lock, &lock_count); // get available space

    if (lock_count == MAX_CARS_IN_INTERSECTION) // all space is available? no thread is waiting?
        sem_post(&mutex);                       // allow X to get in

    /* INTERSECTION ENDS*/
    return NULL;
}

int main(int argc, char const *argv[])
{
    const unsigned cars_count = MAX_CARS_IN_INTERSECTION * 3;
    const unsigned n = 10;

    pthread_t cars_we[cars_count];
    pthread_t cars_ns[cars_count];
    struct road road_we; 	// west to east : horizontal road
    struct road road_ns; 	// north to south : verical road
    struct road *roads[2] = {&road_we, &road_ns};

    sem_init(&lock, 0, MAX_CARS_IN_INTERSECTION);
    sem_init(&mutex, 0, 1);
    road_init(&road_we, n, ROAD_WTOE);
    road_init(&road_ns, n, ROAD_NTOS);

    pthread_t mover_we = Pthread_create(road_stream, &road_we);
    pthread_t mover_ns = Pthread_create(road_stream, &road_ns);
    pthread_t printer = Pthread_create(road_print, &roads);

    // create car threads
    for (unsigned i = 0; i < cars_count; i++)
    {
        cars_we[i] = Pthread_create(carthread, &road_we);
        cars_ns[i] = Pthread_create(carthread, &road_ns);
        road_push(&road_we, cars_we[i]); // push on road
        road_push(&road_ns, cars_ns[i]); // push on road
    }

    // wait for cars
    for (unsigned i = 0; i < cars_count; i++)
    {
        Pthread_join(cars_we[i]);
        Pthread_join(cars_ns[i]);
    }

    // exit road mover_we:
    // exit road mover_ns:
    pthread_cancel(mover_we);
    pthread_cancel(mover_ns);
    Pthread_join(mover_we);
    Pthread_join(mover_ns);

    // exit road printer:
    pthread_cancel(printer);
    Pthread_join(printer);

    // release resources:
    sem_destroy(&lock);
    sem_destroy(&mutex);
    road_destroy(&road_we);
    road_destroy(&road_ns);
    return 0;
}

/* DEFINITIONS */

void *road_print(void *arg)
{
    struct road **proad = (struct road **)arg;
    while (TRUE) // infinite loop
    {
        printf("wtoe : %d\n", proad[ROAD_WTOE]->car_count);
        printf("ntos : %d\n", proad[ROAD_NTOS]->car_count);

        for (unsigned i = 0; i < proad[ROAD_NTOS]->size; i++)
        {
            for (unsigned i = 0; i < proad[ROAD_WTOE]->size; i++)
                fprintf(stdout, "    ");

            if (proad[ROAD_NTOS]->blocks[i].available)
                fprintf(stdout, "[ ] ");
            else
                fprintf(stdout, "[*] ");
            fprintf(stdout, "\n");
        }

        for (unsigned i = 0; i < proad[ROAD_WTOE]->size; i++)
            if (proad[ROAD_WTOE]->blocks[i].available)
                fprintf(stdout, "[ ] ");
            else
                fprintf(stdout, "[*] ");
        fprintf(stdout, "\n");

        pthread_testcancel(); // exit?
        usleep(proad[ROAD_WTOE]->speed);
        system("clear");
    }
    return NULL;
}
