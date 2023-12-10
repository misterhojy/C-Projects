#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include <unistd.h>
#include "csapp.h"
#include "helper.h"
#include "server.h"

/* Static Functions */
static void terminate(int status);
static void sighup_handler(int signum);

volatile sig_atomic_t sig_flag = 0; 
CLIENT_REGISTRY *client_registry;

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int opt, pflag = 0, listenfd, *connfdp;
    char* port;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            pflag = 1;
            port = optarg;
            break;
        case '?':
            exit(EXIT_SUCCESS);
        default:
            break;
        }
    }

    checkargc(argc);    //argc == 3
    checkp(pflag);      //pflag == 1

    // fprintf(stderr, "port number: %s\n", port); //COMMENT
    
    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    trans_init();
    store_init();

    /* install SIGHUP handler */
    Signal(SIGHUP, sighup_handler);

    /* TESTING if SIGHUP handler works */
    // Sleep(1);
    // pid_t pid = getpid();
    // Kill(pid, SIGHUP);

    listenfd = Open_listenfd(port); //tell kernel descriptor used by server
    
    while(1) {  //loop inifitely accepting connections
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA*) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, xacto_client_service, connfdp);
    }
    

    fprintf(stderr, "You have to finish implementing main() "
	    "before the Xacto server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);
    
    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();

    debug("Xacto server terminating");
    exit(status);
}

/* SIGHUP Signal handler so clean termination of the server can be done */
void sighup_handler(int signum) {
    sig_flag = 1;   //set signal flag

    // fprintf(stderr, "SIGHUP received. Signal number: %d\n", signum);    //COMMENT OUT

    sigset_t mask, prev_mask;
    Sigfillset(&mask);                              //fill blocking mask
    Sigprocmask(SIG_BLOCK, &mask, &prev_mask);      //block all signals | maintain previous mask
    terminate(EXIT_SUCCESS);                        //clean terminate the server
    Sigprocmask(SIG_SETMASK, &prev_mask, NULL);     //unblock signals | restore to prev mask
    return;
}
