/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CDH (Communication)                                                             *
 * Author: Maíra Canal (@mairacanal)                                               *
 * São Carlos School of Engineering - University of São Paulo                      *
 * Abril/2021                                                                      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#include "source/can.h"


struct systemArgs {
    int fd;
    int data;
    pthread_mutex_t *mutex;
};


void transmit(void *args);
void *receive(void *args);

int main () {

    int fd0;
    int rcr;

    pthread_mutex_t systemMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t transmitThread, receiveThread, CTHThread;

    struct systemArgs *transmitArgs = (struct systemArgs *)malloc(sizeof(struct systemArgs));
    struct systemArgs *receiveArgs = (struct systemArgs *)malloc(sizeof(struct systemArgs));

    socket_initiation("vcan0", &fd0);

    transmitArgs->fd = fd0;

    receiveArgs->fd = fd0;
    receiveArgs->mutex = &systemMutex;

    transmitArgs->data = 0;
    transmit(transmitArgs);
    if ( (rcr = pthread_create(&receiveThread, NULL, receive, (void *) receiveArgs)) ) printf("Thread creation failed: %d\n", rcr);

    pthread_join(receiveThread, NULL);

    socket_close(fd0);

}


void transmit(void *args) {

    struct systemArgs *systemArgs = (struct systemArgs*)args;
    struct can_frame frame;

    frame.can_id = 0x020;   
    frame.can_dlc = 1;
    frame.data[0] = systemArgs->data + 1;

    printf("enviei : ");
    printCANframe(frame);

    socket_write(systemArgs->fd, &frame);
    
}

void *receive(void *args) {

    fd_set readfd;
    int selected;
    struct can_frame frame;
    struct timeval tv = {.tv_sec = 0, .tv_usec = 0};
    struct systemArgs *systemArgs = (struct systemArgs*)args;

    while (1) {

        pthread_mutex_lock(systemArgs->mutex);

        FD_ZERO(&readfd);
        FD_SET(systemArgs->fd, &readfd);

        selected = select(systemArgs->fd + 1, &readfd, NULL, NULL, &tv);

        if (selected == -1) {
            perror("CAN error");
        } else if (selected > 0) {
            sleep(1);
            socket_read(systemArgs->fd, &frame);
            printf("recebi : ");
            printCANframe(frame);
            systemArgs->data = frame.data[0];
            sleep(1);
            transmit((void*) systemArgs);
        } else {
            
        }
        
        pthread_mutex_unlock(systemArgs->mutex);

    }

}