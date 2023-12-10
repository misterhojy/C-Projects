#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "client_registry.h"
#include <sys/socket.h>
#include <pthread.h>
#include "protocol.h"
#include "time.h"


#define DEFAULT_SIZE 10
#define PREFIX_SIZE 11

struct timespec current_time;

/*
 * A client registry keeps track of the file descriptors for clients
 * that are currently connected.  Each time a client connects,
 * its file descriptor is added to the registry.  When the thread servicing
 * a client is about to terminate, it removes the file descriptor from
 * the registry.  The client registry also provides a function for shutting
 * down all client connections and a function that can be called by a thread
 * that wishes to wait for the client count to drop to zero.  Such a function
 * is useful, for example, in order to achieve clean termination:
 * when termination is desired, the "main" thread will shut down all client
 * connections and then wait for the set of registered file descriptors to
 * become empty before exiting the program.
 */
typedef struct client_registry {
    int* fd_list;   /* fd array */
    int size;       /* max number of slots */
    int clients;
    sem_t mutex;    /* Protects accesses to buf */
    sem_t slots;    /* Counts available slots */
    sem_t items;    /* Counts available items */
} CLIENT_REGISTRY;


void checkargc(int argc);
void checkp(int pflag);
void usage_err(void);
ssize_t Write_help(int fd, void* buf, size_t count);
ssize_t Read_help(int fd, void* buf, size_t count);
int insert_registery(CLIENT_REGISTRY *cr, int fd);
int increase_registry_size(CLIENT_REGISTRY *cr);
int remove_reigstery(CLIENT_REGISTRY *cr, int fd);
int Pthread_mutex_lock(pthread_mutex_t *__mutex);
int Pthread_mutex_unlock(pthread_mutex_t *__mutex);
