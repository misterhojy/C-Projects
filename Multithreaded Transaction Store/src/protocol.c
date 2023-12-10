#include "protocol.h"
#include "csapp.h"
#include "helper.h"
#include <errno.h>

/*
 * Send a packet header, followed by an associated data payload, if any.
 * Multi-byte fields in the packet header are stored in network byte order.
 *
 * @param fd  The file descriptor on which packet is to be sent.
 * @param pkt  The fixed-size part of the packet, with multi-byte fields
 *   in network byte order
 * @param data  The payload for data packet, or NULL.  A NULL value used
 *   here for a data packet specifies the transmission of a special null
 *   data value, which has no content.
 * @return  0 in case of successful transmission, -1 otherwise.  In the
 *   latter case, errno is set to indicate the error.
 */
int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data) {

    /* Writing packet */
    size_t total_bytes = sizeof(XACTO_PACKET);
    ssize_t bytes_written = Write_help(fd, pkt, total_bytes);
    if (bytes_written == -1) {
        strerror(errno);    //COMMENT OUT
        return -1;
    } else if (bytes_written < total_bytes) { 
        // perror("BYTES WRITTEN != SIZE OF PACKET\n"); //COMMENT OUT
        return -1;
    } else if (bytes_written == 0) { 
        // perror("BYTES WRITTEN == 0\n");  //COMMENT OUT
        return -1;
    }

    /* Writing payload */
    if (data) {
        total_bytes = ntohl(pkt->size);

        bytes_written = Write_help(fd, data, total_bytes);
        if (bytes_written == -1) {
            strerror(errno);    //COMMENT OUT
            return -1;
        } else if (bytes_written < total_bytes) { 
            // perror("BYTES WRITTEN != SIZE OF PAYLOAD\n");   //COMMENT OUT
            return -1;
        } else if (bytes_written == 0) { 
            // perror("BYTES WRITTEN == 0\n");  //COMMENT OUT
            return -1;
        }
    }
    return 0;
}


/*
 * Receive a packet, blocking until one is available.
 * The returned structure has its multi-byte fields in network byte order.
 *
 * @param fd  The file descriptor from which the packet is to be received.
 * @param pkt  Pointer to caller-supplied storage for the fixed-size
 *   portion of the packet.
 * @param datap  Pointer to variable into which to store a pointer to any
 *   payload received.
 * @return  0 in case of successful reception, -1 otherwise.  In the
 *   latter case, errno is set to indicate the error.
 *
 * If the returned payload pointer is non-NULL, then the caller assumes
 * responsibility for freeing the storage.
 */
int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap) {

    /* Read packet */
    size_t total_bytes = sizeof(XACTO_PACKET);
    ssize_t bytes_read = Read_help(fd, pkt, total_bytes);
    if (bytes_read == -1) {
        strerror(errno);    //COMMENT OUT
        return -1;
    } else if (bytes_read < total_bytes) { 
        // perror("BYTES READ != SIZE OF PACKET\n");    //COMMENT OUT
        return -1;
    } else if (bytes_read == 0) { 
        // perror("BYTES READ == 0, EOF REACEHD\n");    //COMMENT OUT
        return -1;
    }


    /* Read payload */
    if (ntohl(pkt->size) > 0) {         //if it is pointing to a valid pointer to a payload

        total_bytes = ntohl(pkt->size);

        *datap = Malloc(total_bytes);          //Malloc the payload
        bytes_read = Read_help(fd, *datap, total_bytes);
        if (bytes_read == -1) {
            strerror(errno);    //COMMENT OUT
            return -1;
        } else if (bytes_read < total_bytes) { 
            // perror("BYTES READ != SIZE OF PAYLOAD\n");   //COMMENT OUT
            return -1;
        } else if (bytes_read == 0) { 
            // perror("BYTES READ == 0\n"); //COMMENT OUT
            return -1;
        }
    }

    return 0;
}
