#include "transaction.h"
#include "helper.h"
#include "csapp.h"
#include "protocol.h"
#include "data.h"
#include "store.h"
#include "debug.h"

int td = 0;

int transaction_id() {
    return td++;
}
/*
 * Initialize the transaction manager.
 */
void trans_init(void) {
    // fprintf(stderr, "init TRANSACTIONs\n");
    trans_list.id = -1;
    trans_list.depends = NULL;
    trans_list.prev = &trans_list;  //Circle Link List
    trans_list.next = &trans_list;
}


/*
 * Finalize the transaction manager.
 */
void trans_fini(void) {
    // fprintf(stderr, "FINAL AND FREE TRANSACTIONs\n");
    TRANSACTION* current_transaction = trans_list.next;
    while (current_transaction != &trans_list) {
        /* Free every transaction in the list */
        TRANSACTION* tp = current_transaction;
        current_transaction = current_transaction->next;    //iterate

        Pthread_mutex_lock(&tp->mutex);

        /* FREE DEPENDENCIES */
        DEPENDENCY *dp = tp->depends;
        while(dp != NULL) {
            DEPENDENCY *dep = dp;
            dp = dp->next;
            Free(dep);
        }

        if(sem_destroy(&tp->sem) == -1) {   //MIGHT NEED TO COMMENT OUT
            // fprintf(stderr, "DESTROY SEM FAILED\n");
        }
        Pthread_mutex_unlock(&tp->mutex);
        Free(tp);
        // fprintf(stderr, "ALL in List FREE\n");
    }
    // fprintf(stderr, "Freeing the initial\n");
    /* free the initial node */
}


/*
 * Create a new transaction.
 *
 * @return  A pointer to the new transaction (with reference count 1)
 * is returned if creation is successful, otherwise NULL is returned.
 */
TRANSACTION *trans_create(void) {
    TRANSACTION* new_transaction = (TRANSACTION*)malloc(sizeof(TRANSACTION));   //allocate
    if (new_transaction == NULL) {
        // fprintf(stderr, "MALLOC ERROR\n");
        return NULL;
    }
    if (pthread_mutex_init(&new_transaction->mutex, NULL) != 0) {   //init mutex
        // fprintf(stderr, "error mutex init\n");
        return NULL;
    }

    if (sem_init(&new_transaction->sem, 0, 0) < 0) {    //init sem
        // fprintf(stderr, "sem init error\n");
        return NULL;
    }

    new_transaction->id = transaction_id();
    // fprintf(stderr, "creating transaction of id: %d\n", new_transaction->id);
    new_transaction->depends = NULL;
    new_transaction->refcnt = 1;
    new_transaction->waitcnt = 0;
    new_transaction->status = TRANS_PENDING;
    new_transaction->next = &trans_list;
    trans_list.prev->next = new_transaction;
    new_transaction->prev = trans_list.prev;
    trans_list.prev = new_transaction;

    // fprintf(stderr, "CREATED NEW TRANSACTION\n");

    return new_transaction;
}

/*
 * Increase the reference count on a transaction.
 *
 * @param tp  The transaction.
 * @param why  Short phrase explaining the purpose of the increase.
 * @return  The transaction pointer passed as the argument.
 */
TRANSACTION *trans_ref(TRANSACTION *tp, char *why) {
    Pthread_mutex_lock(&tp->mutex); /* lock mutex*/

    tp->refcnt++;
    // fprintf(stderr, "INCREASING REF of %d: %d\n",tp->id ,tp->refcnt);
    // fprintf(stderr,"reason: %s\n", why);    //COMMENT OUT

    Pthread_mutex_unlock(&tp->mutex);       /* Unlock mutex */

    return tp;
}

/*
 * Decrease the reference count on a transaction.
 * If the reference count reaches zero, the transaction is freed.
 *
 * @param tp  The transaction.
 * @param why  Short phrase explaining the purpose of the decrease.
 */
void trans_unref(TRANSACTION *tp, char *why) {
    Pthread_mutex_lock(&tp->mutex); /* lock mutex*/

    tp->refcnt--;
    // fprintf(stderr, "DECREASING REF of %d: %d\n",tp->id ,tp->refcnt);
    // fprintf(stderr,"WHY: %s\n", why);    //COMMENT OUT

    if (tp->refcnt == 0) {
        // fprintf(stderr, "REF COUNT of %d is 0 FREE IT\n", tp->id);
        
        /* FREE DEPENDENCIES */
        // fprintf(stderr, "Freeing dependencies\n");
        DEPENDENCY *dp = tp->depends;
        while(dp != NULL) {
            DEPENDENCY *dep = dp;
            dp = dp->next;

            Pthread_mutex_lock(&dep->trans->mutex); //decrease wait count as it no longer need to wait
            dep->trans->refcnt--;
            // fprintf(stderr, "waitcount of transaction in dependency list: %d\n", dep->trans->waitcnt);
            Pthread_mutex_unlock(&dep->trans->mutex);

            Free(dep);
        }
        tp->depends = NULL;
        tp->prev->next = tp->next;
        tp->next->prev = tp->prev;

        if(sem_destroy(&tp->sem) == -1) {   //MIGHT NEED TO COMMENT OUT
            // fprintf(stderr, "DESTROY SEM FAILED\n");
        }
        Pthread_mutex_unlock(&tp->mutex);       /* Unlock mutex */
        Free(tp);
    } else {
        Pthread_mutex_unlock(&tp->mutex);       /* Unlock mutex */
    }
}

/*
 * Add a transaction to the dependency set for this transaction.
 *
 * @param tp  The transaction to which the dependency is being added.
 * @param dtp  The transaction that is being added to the dependency set.
 */
void trans_add_dependency(TRANSACTION *tp, TRANSACTION *dtp) {
    DEPENDENCY* dp = Malloc(sizeof(DEPENDENCY));
    dp->trans = dtp;
    dp->next = NULL;

    /* Add the dependency in set */
    int flag = 0;
    DEPENDENCY* current_d = tp->depends;
    if (current_d == NULL) {    //first dependency
        // fprintf(stderr, "first dependent\n");
        tp->depends = dp;
        trans_ref(dtp, "added to dependency set\n");
    } else {
        while (current_d->next != NULL) {   //Add at the last part in list
            if (current_d->trans == dtp) {
                /* Set can only contain one. */
                flag = 1;
                // fprintf(stderr, "already in dependency set\n");
            }
            current_d = current_d->next;
        }
        if (!flag) {
            // fprintf(stderr, "added dependency\n");
            current_d->next = dp;
            trans_ref(dtp, "added to dependency set\n");
        }
    }
    Pthread_mutex_lock(&dtp->mutex);    //lock
    dtp->waitcnt++;                     //increase wait
    // fprintf(stderr, "wait count for %d: %d\n", dtp->id, dtp->waitcnt);
    Pthread_mutex_unlock(&dtp->mutex);  //unlock
}

/*
 * Try to commit a transaction.  Committing a transaction requires waiting
 * for all transactions in its dependency set to either commit or abort.
 * If any transaction in the dependency set abort, then the dependent
 * transaction must also abort.  If all transactions in the dependency set
 * commit, then the dependent transaction may also commit.
 *
 * In all cases, this function consumes a single reference to the transaction
 * object.
 *
 * @param tp  The transaction to be committed.
 * @return  The final status of the transaction: either TRANS_ABORTED,
 * or TRANS_COMMITTED.
 */
TRANS_STATUS trans_commit(TRANSACTION *tp) {
    DEPENDENCY* current_d = tp->depends;
    while (current_d != NULL) {                 //sem wait on all dependencies
        TRANSACTION* tranp = current_d->trans;
        sem_wait(&tranp->sem);
        /* if any dependency abort tp aborts */
        if (trans_get_status(tranp) == TRANS_ABORTED) {
            /* DO WE TRANS REF? */
            trans_ref(tp, "dependency aborted\n");
            trans_abort(tp);
            return trans_get_status(tp);
        }
        current_d = current_d->next;
    }
    /* all the transactions it depends on are done or there is None */ 
    //need to unref all in the dependency list
    DEPENDENCY* dp = tp->depends;
    while (dp != NULL) {
        TRANSACTION* tranp = dp->trans;
        trans_unref(tranp, "dependency all comitted\n");
        dp = dp->next;
    }
    tp->status = TRANS_COMMITTED;
    /* need to release all waits waiting for this transaction */
    Pthread_mutex_lock(&tp->mutex);
    for (int i = 0; i < tp->waitcnt; i++) {
        sem_post(&tp->sem);
    }
    Pthread_mutex_unlock(&tp->mutex);

    trans_unref(tp, "transaction committed\n");
    return trans_get_status(tp);
}

/*
 * Abort a transaction.  If the transaction has already committed, it is
 * a fatal error and the program crashes.  If the transaction has already
 * aborted, no change is made to its state.  If the transaction is pending,
 * then it is set to the aborted state, and any transactions dependent on
 * this transaction must also abort.
 *
 * In all cases, this function consumes a single reference to the transaction
 * object.
 *
 * @param tp  The transaction to be aborted.
 * @return  TRANS_ABORTED.
 */
TRANS_STATUS trans_abort(TRANSACTION *tp) {
    if (trans_get_status(tp) == TRANS_COMMITTED) {
        // fprintf(stderr, "ALREADY COMMITED CANT ABORT:: FATAL\n");
        abort();

    } else if (trans_get_status(tp) == TRANS_ABORTED) {
        //No change
        return trans_get_status(tp);

    } else if (trans_get_status(tp) == TRANS_PENDING) {
        /* change the status */

        tp->status = TRANS_ABORTED;

        //unlock to access waitcnt
        Pthread_mutex_lock(&tp->mutex);
        for (int i = 0; i < tp->waitcnt; i++) { //release all the transactions that are waiting
            sem_post(&tp->sem);
        }
        Pthread_mutex_unlock(&tp->mutex);   //unlock

        trans_unref(tp, "aborted transaction"); //unref the transaction
    }
    // fprintf(stdout, "Transaction Aborted\n");
    return trans_get_status(tp);
}

/*
 * Get the current status of a transaction.
 * If the value returned is TRANS_PENDING, then we learn nothing,
 * because unless we are holding the transaction mutex the transaction
 * could be aborted at any time.  However, if the value returned is
 * either TRANS_COMMITTED or TRANS_ABORTED, then that value is the
 * stable final status of the transaction.
 *
 * @param tp  The transaction.
 * @return  The status of the transaction, as it was at the time of call.
 */
TRANS_STATUS trans_get_status(TRANSACTION *tp) {
    pthread_mutex_lock(&tp->mutex);
    TRANS_STATUS status = tp->status;
    pthread_mutex_unlock(&tp->mutex);
    return status;
}

/*
 * Print information about a transaction to stderr.
 * No locking is performed, so this is not thread-safe.
 * This should only be used for debugging.
 *
 * @param tp  The transaction to be shown.
 */
void trans_show(TRANSACTION *tp) {

}

/*
 * Print information about all transactions to stderr.
 * No locking is performed, so this is not thread-safe.
 * This should only be used for debugging.
 */
void trans_show_all(void) {
    
}