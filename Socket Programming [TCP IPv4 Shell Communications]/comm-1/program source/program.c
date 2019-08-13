#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <termios.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

int main()
{
    char buf[1024];
    
    struct pollfd fds[1];
    int fd = 0;
    int timeout = 10000;
    
    while(1)
    {
        fds[0].fd = fd;
        fds[0].events = POLLIN | POLLHUP | POLLERR;
        if(poll(fds, 1, timeout)==0)
        {
            fprintf(stderr, "\r\nTimeout: %d ms\r\n", timeout);
            exit(EXIT_FAILURE);
        }
        else
        {
            if(fds[0].revents & POLLIN)
            {
                //read from stdin
                if(read(fds[0].fd, buf, 1024) == -1)
                {
                    fprintf(stderr, "READING ERROR: %s\r\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            else if(fds[0].revents & POLLHUP)
            {
                fprintf(stderr, "The device or socket has been disconnected\r\n");
                exit(EXIT_FAILURE);
            }
            else if(fds[0].revents & POLLERR)
            {
                fprintf(stderr, "An exceptional condition has occurred on the device or socket\r\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
    }
    
    fprintf(stdout, "\nPRINTING FROM CHILD:\r\n");
    if(write(1, buf, strlen(buf)) == -1)    //write to stdout
    {
        fprintf(stderr, "WRITING ERROR: %s\r\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return 0;
}

