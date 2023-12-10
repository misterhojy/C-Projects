#include "helper.h"
#include "csapp.h"
#include "client_registry.h"
#include "protocol.h"

void usage_err() {
    fprintf(stderr, "Usage: -p <port>\n");
    exit(EXIT_SUCCESS);
}

void checkargc(int argc) {
    if (argc != 3) {
        usage_err();
    }
}

void checkp(int pflag) {
    if (!pflag) {
        usage_err();
    }
    
}

ssize_t Write_help(int fd, void* buf, size_t count) {
    ssize_t current_wrote = 0;
    ssize_t bytes_wrote;

    while (current_wrote < count) {
        bytes_wrote = write(fd, buf + current_wrote, count - current_wrote);    //need subsequent call in case partial write()

        if (bytes_wrote <= 0) {    //if an error was thrown
            if (errno == EINTR) {  //interrupted by signal restart
                current_wrote = 0;
            } else if (bytes_wrote == 0) {
                return 0;
            } else {
                return -1;    
            }
        }
        current_wrote += bytes_wrote;
    }
    return current_wrote;
}

ssize_t Read_help(int fd, void* buf, size_t count) {
    ssize_t current_read = 0;
    ssize_t bytes_read;

    while (current_read < count) {
        bytes_read = read(fd, buf + current_read, count - current_read);

        if (bytes_read < 0) {       //if error occured
            if (errno == EINTR) {   //interrupted by signal restart
                current_read = 0;
            } else {
                return -1;
            }
        } else if (bytes_read == 0) {   //EOF reached
            break;
        }
        current_read += bytes_read;
    }
    return current_read;
}

int insert_registery(CLIENT_REGISTRY *cr, int fd) {
    for (int i = 0; i < cr->size; i++) {        /* Insert into the list */
        if (cr->fd_list[i] == -1) {
            cr->fd_list[i] = fd;
            cr->clients++;      //++ clients connected
            return 0;
        }
    }
    return -1;
}

int increase_registry_size(CLIENT_REGISTRY *cr) {
    int new_size = 2 * cr->size;
    int* new_fd_list = (int*)realloc(cr->fd_list, new_size * sizeof(int));
    if (new_fd_list == NULL) {
        free(cr->fd_list);
        free(cr);
        return -1;
    }
    cr->size = new_size;
    cr->fd_list = new_fd_list;
    return 0;
}

int remove_reigstery(CLIENT_REGISTRY *cr, int fd) {
    for (int i = 0; i < cr->size; i++) {
        if (cr->fd_list[i] == fd) {
            cr->fd_list[i] = -1;
            cr->clients--;
            return 0;
        }
    }
    return -1;
}

int Pthread_mutex_lock(pthread_mutex_t *__mutex) {
    if (pthread_mutex_lock(__mutex) != 0) {
        unix_error("pthread_mutex_lock error\n");
    }
    return 0;
}

int Pthread_mutex_unlock(pthread_mutex_t *__mutex) {
    if (pthread_mutex_unlock(__mutex) != 0) {
        unix_error("pthread_mutex_unlock error\n");
    }
    return 0;
}

