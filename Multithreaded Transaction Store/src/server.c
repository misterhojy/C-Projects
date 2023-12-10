#include "server.h"
#include "helper.h"
#include "csapp.h"
#include "transaction.h"
#include "protocol.h"
#include "data.h"
#include "store.h"


CLIENT_REGISTRY *client_registry;
/*
 * Thread function for the thread that handles client requests.
 *
 * @param  Pointer to a variable that holds the file descriptor for
 * the client connection.  This pointer must be freed once the file
 * descriptor has been retrieved.
 */
void *xacto_client_service(void *arg) {

    int connfd = *(int*)arg;                        //get fd
    if (pthread_detach(pthread_self()) != 0) {
        unix_error("detatch error\n");
    }
    Free(arg);

    /* register the connected */
    if (creg_register(client_registry, connfd) == -1) {
        // fprintf(stderr, "coud not insert into registery\n");    //COMMENT OUT
        return NULL;
    }
    // fprintf(stderr, "inserted fd %d in CR\n", connfd);      //COMMENT OUT

    /* create a transaction */
    TRANSACTION* transaction = trans_create();
    if (transaction == NULL) {
        // fprintf(stderr, "creating transaction failed\n");   //COMMENT OUT
        return NULL;
    }

    while (1) {
        /* inside the service loop */
        XACTO_PACKET* packet = (XACTO_PACKET*)Malloc(sizeof(XACTO_PACKET)); //retrieve request packet
        void* payload;
        if (proto_recv_packet(connfd, packet, &payload) == -1) {
            // fprintf(stderr, "recv packet failedA\n");   //COMMENT OUT
            trans_abort(transaction);
            Free(packet);
            break;
        }

        if (packet->type == XACTO_PUT_PKT) {    //if a PUT
            /* Getting the Key Packet */
            if (proto_recv_packet(connfd, packet, &payload) == -1) {
                // fprintf(stderr, "recv packet failed\n");   //COMMENT OUT
                trans_abort(transaction);
                Free(packet);
                break;   
            }
            BLOB* key_blob = blob_create((char*)payload, ntohl(packet->size));  //make the key blob
            KEY* key = key_create(key_blob);    //make a key

            /* Getting the Value Packet */
            void* value_payload;
            if (proto_recv_packet(connfd, packet, &value_payload) == -1) {
                // fprintf(stderr, "recv packet failed\n");   //COMMENT OUT
                trans_abort(transaction);
                Free(packet);
                break;
            }
            BLOB* value = blob_create((char*)value_payload, ntohl(packet->size));

            /* Put in the store */
            store_put(transaction, key, value);

            /* check result of PUT*/
            if (trans_get_status(transaction) == TRANS_ABORTED) {
                // fprintf(stderr, "Transaction ABORTED\n");
                trans_abort(transaction);
                Free(packet);
                break;
            }
            //REPLY from server to CLIENT
            packet->null = 0;
            packet->size = 0;
            packet->status = trans_get_status(transaction);
            packet->type = XACTO_REPLY_PKT;
            
            clock_gettime(CLOCK_MONOTONIC, &current_time);
            packet->timestamp_sec = current_time.tv_sec;
            packet->timestamp_nsec = current_time.tv_nsec;

            if (proto_send_packet(connfd, packet, NULL) == -1) {
                // fprintf(stderr, "send packet failed\n");   //COMMENT OUT
                trans_abort(transaction);
                Free(packet);
                break;
            }
            Free(packet);
        } else if (packet->type == XACTO_GET_PKT) {
            /* Getting Key */
            if (proto_recv_packet(connfd, packet, &payload) == -1) {
                // fprintf(stderr, "recv packet failed\n");   //COMMENT OUT
                trans_abort(transaction);
                Free(packet);
                break;   
            }
            BLOB* key_blob = blob_create((char*)payload, ntohl(packet->size));  //make the key blob
            KEY* key = key_create(key_blob);    //make a key

            BLOB* value = blob_create(NULL, 0); //Value to be filled

            store_get(transaction, key, &value);

            if (trans_get_status(transaction) == TRANS_ABORTED) {
                // fprintf(stderr, "Transaction ABORTED\n");
                trans_abort(transaction);
                Free(packet);
                break;
            }
            // fprintf(stderr, "VALUE FROM GET: %s\n", value->prefix);
            // fprintf(stderr, "SIZE FROM GET: %ld\n", value->size);
            
            //REPLY from server to CLIENT
            packet->null = 0;
            packet->size = 0;
            packet->status = trans_get_status(transaction);
            packet->type = XACTO_REPLY_PKT;
            
            clock_gettime(CLOCK_MONOTONIC, &current_time);
            packet->timestamp_sec = current_time.tv_sec;
            packet->timestamp_nsec = current_time.tv_nsec;

            if (proto_send_packet(connfd, packet, NULL) == -1) {
                // fprintf(stderr, "send packet failed\n");   //COMMENT OUT
                trans_abort(transaction);
                Free(packet);
                break;
            }

            if (value == NULL) {
                packet->null = 1;
                packet->size = 0;
                packet->status = trans_get_status(transaction);
                packet->type = XACTO_VALUE_PKT;
                
                clock_gettime(CLOCK_MONOTONIC, &current_time);
                packet->timestamp_sec = current_time.tv_sec;
                packet->timestamp_nsec = current_time.tv_nsec;

                if (proto_send_packet(connfd, packet, NULL) == -1) {
                    // fprintf(stderr, "send packet failed\n");   //COMMENT OUT
                    trans_abort(transaction);
                    Free(packet);
                    break;
                }
            } else {
                /* SEND DATA BACK TO CLIENT VALUE PACKET WITH PAYLOAD */
                char* data = value->content;
                packet->null = 0;
                packet->size = htonl(value->size);
                packet->status = trans_get_status(transaction);
                packet->type = XACTO_VALUE_PKT;

                clock_gettime(CLOCK_MONOTONIC, &current_time);
                packet->timestamp_sec = current_time.tv_sec;
                packet->timestamp_nsec = current_time.tv_nsec;

                if (proto_send_packet(connfd, packet, data) == -1) {
                    // fprintf(stderr, "send packet failed\n");   //COMMENT OUT
                    trans_abort(transaction);
                    Free(packet);
                    break;
                }
            }
            Free(packet);
        } else if (packet->type == XACTO_COMMIT_PKT) {
            trans_commit(transaction);
            if (trans_get_status(transaction) == TRANS_ABORTED) {
                // fprintf(stderr, "Transaction ABORTED\n");
                trans_abort(transaction);
                Free(packet);
                break;
            } else if(trans_get_status(transaction) == TRANS_COMMITTED){
                //REPLY from server to CLIENT
                packet->null = 0;
                packet->size = 0;
                packet->status = trans_get_status(transaction);
                packet->type = XACTO_REPLY_PKT;
                
                clock_gettime(CLOCK_MONOTONIC, &current_time);
                packet->timestamp_sec = current_time.tv_sec;
                packet->timestamp_nsec = current_time.tv_nsec;

                if (proto_send_packet(connfd, packet, NULL) == -1) {
                    // fprintf(stderr, "send packet failed\n");   //COMMENT OUT
                    trans_abort(transaction);
                    Free(packet);
                    break;
                }
            }
            Free(packet);
            break;
        } else {
            /* NOT A PUT GET COMMIT */
            // fprintf(stderr, "NOT PUT GET COMMIT\n");   //COMMENT OUT
            trans_abort(transaction);
            Free(packet);
            break;
        }
    }
    creg_unregister(client_registry, connfd);
    Close(connfd);
    return NULL;
}