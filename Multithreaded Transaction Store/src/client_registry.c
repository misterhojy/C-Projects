#include "helper.h"
#include "csapp.h"
/*
 * Initialize a new client registry.
 *
 * @return  the newly initialized client registry, or NULL if initialization
 * fails.
 */
CLIENT_REGISTRY *creg_init() {
    CLIENT_REGISTRY* init_registery = (CLIENT_REGISTRY*)malloc(sizeof(CLIENT_REGISTRY));
    if (init_registery == NULL) {
        fprintf(stderr,"Allocation failed");    //COMMENT OUT
        return NULL;
    }

    init_registery->fd_list = (int*)malloc(DEFAULT_SIZE * sizeof(int));
    if (init_registery->fd_list == NULL) {
        free(init_registery);
        fprintf(stderr,"Allocation failed");    //COMMENT OUT
        return NULL;
    }

    for (int i = 0; i < DEFAULT_SIZE; i++) {
        init_registery->fd_list[i] = -1;    /* no fd connected */
    }

    init_registery->clients = 0;            /* no clients yet */
    init_registery->size = DEFAULT_SIZE;    /* Adjust the size later */

    if (sem_init(&init_registery->mutex, 0, 1) < 0) {   /* Binary semaphore for locking, initially unlocked */
	    fprintf(stderr,"Sem_init error");       //COMMENT OUT
        free(init_registery->fd_list);
        free(init_registery);
        return NULL;
    }
    if (sem_init(&init_registery->slots, 0, init_registery->size) < 0) {   /* Initially, buf has "size" empty slots */
	    fprintf(stderr,"Sem_init error");       //COMMENT OUT
        free(init_registery->fd_list);
        free(init_registery);
        return NULL;
    }
    if (sem_init(&init_registery->items, 0, 0) < 0) {   /* Initially, buf has zero data items */
	    fprintf(stderr,"Sem_init error");       //COMMENT OUT
        free(init_registery->fd_list);
        free(init_registery);
        return NULL;
    }

    return init_registery;
}

/*
 * Finalize a client registry, freeing all associated resources.
 * This method should not be called unless there are no currently
 * registered clients.
 *
 * @param cr  The client registry to be finalized, which must not
 * be referenced again.
 */
void creg_fini(CLIENT_REGISTRY *cr) {
    /* free empty list & free registery */
    free(cr->fd_list);
    free(cr);
}

/*
 * Register a client file descriptor.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be registered.
 * @return 0 if registration is successful, otherwise -1.
 */
int creg_register(CLIENT_REGISTRY *cr, int fd) {
    if (sem_wait(&cr->slots) < 0) {             /* P operation, Wait for available slot */
        fprintf(stderr, "P error\n");           //COMMENT OUT
        return -1;
    }  
    if (sem_wait(&cr->mutex) < 0) {             /* P operation, Lock the buffer */
        fprintf(stderr, "P error\n");           //COMMENT OUT
        return -1;
    }      

    int insert_result = insert_registery(cr, fd);
    if (insert_result == -1) {                                /* if Full Array so dynamically expand it */
        if (increase_registry_size(cr) == -1) {
            fprintf(stderr, "realloc Error\n"); //COMMENT OUT
            return -1;
        }
        insert_result = insert_registery(cr, fd);             /* insert back into updated array if full */
        if (insert_result == -1) {
            fprintf(stderr, "Insertion after realloc failed\n"); //COMMENT OUT
            return -1;
        }
    } 

    if (sem_post(&cr->mutex) < 0) {                         /*V operation, Unlock the buffer */
        fprintf(stderr, "V error\n");
        return -1;
    }  
    if (sem_post(&cr->items) < 0) {                         /* V operation, Announce available item */
        fprintf(stderr, "V error\n");
        return -1;
    }  
    return 0;
}

/*
 * Unregister a client file descriptor, removing it from the registry.
 * If the number of registered clients is now zero, then any threads that
 * are blocked in creg_wait_for_empty() waiting for this situation to occur
 * are allowed to proceed.  It is an error if the CLIENT is not currently
 * registered when this function is called.
 *
 * @param cr  The client registry.
 * @param fd  The file descriptor to be unregistered.
 * @return 0  if unregistration succeeds, otherwise -1.
 */
int creg_unregister(CLIENT_REGISTRY *cr, int fd) {
    if (sem_wait(&cr->mutex) < 0) {             /* P operation, Lock the buffer */
        fprintf(stderr, "P error\n");           //COMMENT OUT
        return -1;
    }        
    
    int remove_result = remove_reigstery(cr, fd);
    if(remove_result == -1) {
        fprintf(stderr, "CLIENT NOT REGISTERED\n");           //COMMENT OUT
        sem_post(&cr->mutex);
        return -1;
    }

    if (cr->clients == 0) {                     /* if the client = 0 release the slot semaphore */
        sem_post(&cr->slots);
    }
    
    if (sem_post(&cr->mutex) < 0) {             /* V operation, Unlock the buffer */
        fprintf(stderr, "V error\n");
        return -1;
    }     
    
    if (sem_post(&cr->items) < 0) {             /* V operation, Announce available item */
        fprintf(stderr, "V error\n");           //COMMENT OUT
        return -1;
    }
    return 0;
}

/*
 * A thread calling this function will block in the call until
 * the number of registered clients has reached zero, at which
 * point the function will return.  Note that this function may be
 * called concurrently by an arbitrary number of threads.
 *
 * @param cr  The client registry.
 */
void creg_wait_for_empty(CLIENT_REGISTRY *cr) {
    if (sem_post(&cr->items) < 0) {             /* wait for clients to be deregistered */
        fprintf(stderr, "V error\n");           //COMMENT OUT
    }      

    while (1) {                 /* loop until client gets to zero */
        if (sem_wait(&cr->mutex) < 0) {             /*acquire the mutex to protect access to the clients count */
            fprintf(stderr, "P error\n");           //COMMENT OUT
        }   

        if (cr->clients == 0) { /* If all clients are deregistered, release the mutex and break out of the loop */
            if (sem_post(&cr->mutex) < 0) {             /* V operation, Unlock the buffer */
                fprintf(stderr, "V error\n");
            }
            break;
        }
        if (sem_post(&cr->mutex) < 0) {             /* Release the mutex */
            fprintf(stderr, "V error\n");
        }   
    }
}


/*
 * Shut down (using shutdown(2)) all the sockets for connections
 * to currently registered clients.  The file descriptors are not
 * unregistered by this function.  It is intended that the file
 * descriptors will be unregistered by the threads servicing their
 * connections, once those server threads have recognized the EOF
 * on the connection that has resulted from the socket shutdown.
 *
 * @param cr  The client registry.
 */
void creg_shutdown_all(CLIENT_REGISTRY *cr) {
    for (int i = 0; i < cr->size; i++) {
        if (cr->fd_list[i] != -1) {
            if (shutdown(cr->fd_list[i], SHUT_RDWR) == -1) {
                perror("shutdown error");
            }
        }
    }
}
