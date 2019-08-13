//Parent program
//Description in README.md

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

struct termios savedparams;
void reset()
{
    // Set stdin attributes back to default.
    if(tcsetattr(0, TCSANOW, &savedparams) == -1)
    {
        fprintf(stderr, "Error setting attributes: %s\r\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
//----------------------------------------------------------------------------------------
void setinputmodes()
{
    struct termios tempattr;
    
    // Check if stdin (fd0) is pointed to terminal.
    if(!isatty(0))
    {
        fprintf(stderr, "Not a terminal: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Save the terminal attributes for restoration.
    if(tcgetattr (0, &savedparams) == -1)
    {
        fprintf(stderr, "Error getting attributes: %s\r\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    //atexit(reset);
    
    // Get current terminal modes.
    if(tcgetattr (0, &tempattr) == -1)
    {
        fprintf(stderr, "Error getting attributes: %s\r\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Modify to fit according to non-canonical/no echo mode.
    tempattr.c_iflag = ISTRIP;      //set input flag to only 7 lowest bits (strip 8th)
    tempattr.c_oflag = 0;           //set output flag to 0 (no processing)
    tempattr.c_lflag = 0;           //set local flag to 0 (no processing)
    
    // Set new terminal modes (non-canonical, no-echo input).
    if(tcsetattr(0, TCSANOW, &tempattr) == -1)
    {
        fprintf(stderr, "Error setting attributes: %s\r\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
//----------------------------------------------------------------------------------------
void Echo(char* string)
{
    if(write(1, string, strlen(string)) == -1)
    {
        fprintf(stderr, "WRITE ERROR: %s\r\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
//----------------------------------------------------------------------------------------
//MODIFIED (ORIGINAL VERSION PART 2.1)
void proc()
{
    int pipets[2];  //pipe to program i/o descriptors
    int pipefs[2];  //pipe from program i/o descriptors
    
    if(pipe(pipets)==0 && pipe(pipefs)==0)
    {
        int status;
        pid_t pid = fork(); //fork into two processes
        
        struct pollfd fdk[1];
        int timeout = 10000;
        int fd = 0;
        
        //ERROR
        if(pid < 0)
        {
            fprintf(stderr, "FORK ERROR: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        //PARENT (pipe to program)
        else if(pid > 0)
        {
            int childpid = (int)pid;
            
            fprintf(stdout, "PIPE THIS TO PROGRAM:\n");
            setinputmodes();
            char bufo[256];
            char c = 'a';
            
            int i=0;
            while(1)
            {
                fdk[0].fd = fd;
                fdk[0].events = POLLIN | POLLHUP | POLLERR;
                if(poll(fdk, 1, timeout)==0)
                {
                    printf("\r\nTimeout: %d ms\r\n", timeout);
                    if(strlen(bufo) > 0)
                        printf("In Buffer: %s\r\n", bufo);
                    break;
                }
                else
                {
                    if(fdk[0].revents & POLLIN)
                    {
                        //read from keyboard
                        if(read(fdk[0].fd, &c, sizeof(c)) == -1)
                        {
                            fprintf(stderr, "READ ERROR: %s\r\n", strerror(errno));
                        }
                    }
                }
                //read(0, &c, sizeof(c));       //read from keyboard
                
                if(c == '\003')
                {
                    if(write(1, "^C\r\n", 5) == -1)
                    {
                        fprintf(stderr, "WRITE ERROR: %s\r\n", strerror(errno));
                    }
                    strcat(bufo, "^C");
                    if(kill(childpid, SIGINT) == -1)
                    {
                        fprintf(stderr, "ERROR SIGINT (PID %d): %s\r\n", childpid, strerror(errno));
                    }
                }
                if(c == '\004')          //ASCII ^D (EOF)
                {
                    //printf("^D");
                    if(write(1, "^D\r\n", 5) == -1)
                    {
                        fprintf(stderr, "WRITE ERROR: %s\r\n", strerror(errno));
                    }
                    strcat(bufo, "^D");
                    //write(pipets[1], "^D", 3);
                    //printf("\r\n");
                    i+=2;
                    break;
                }
                else if(c == '\r' || c == '\n')
                {
                    //printf("<cr><lf>");
                    if(write(1, "<cr><lf>\r\n", 11) == -1)
                    {
                        fprintf(stderr, "WRITE ERROR: %s\r\n", strerror(errno));
                    }
                    strcat(bufo, "<lf>");
                    Echo(bufo);
                    printf("\r\n");
                    //write(pipets[1], "<lf>", 4);
                    i+=4;
                }
                else
                {
                    //printf("%c", c);
                    if(write(1, &c, 1) == -1)
                    {
                        fprintf(stderr, "WRITE ERROR: %s\r\n", strerror(errno));
                    }
                    bufo[i] = c;
                    //write(pipets[1], &c, sizeof(c));
                    i++;
                }
            }
            reset();
            
            // KEYBOARD OUTPUT TO THE PROGRAM
            // left parent (for left cmd of pipe) -- write piped data to child
            if(close(pipets[0]) == -1)                   //close unused read end
            {
                fprintf(stderr, "CLOSING ERROR pipets[0]: %s\r\n", strerror(errno));
            }
            if(write(pipets[1], bufo, 1024) == -1)       //write input contents into left pipe
            {
                fprintf(stderr, "WRITE ERROR: %s\r\n", strerror(errno));
            }
            if(close(pipets[1]) == -1)                   //close pipe for EOF to work
            {
                fprintf(stderr, "CLOSING ERROR pipets[1]: %s\r\n", strerror(errno));
            }

            //wait(NULL); //wait for child to send string
            if(waitpid(pid, &status, 0) == -1)
            {
                fprintf(stderr, "Waitpid failure (PID %d): %s\r\n", pid, strerror(errno));
            }
            int low = status & 0x007f;
            int high = status & 0xff;
            fprintf(stderr, "\r\nPROGRAM EXIT SIGNAL=%d STATUS=%d", low, high);
            printf("\r\n");
            exit(EXIT_SUCCESS);
        }
        //CHILD (pipe from program)
        else
        {
            char buf1[1024];
            
            // INPUT TO THE PROGRAM
            // right child (for right cmd of pipe) -- read piped data from parent
            if(close(pipets[1]) == -1)                       //close unused write end
            {
                fprintf(stderr, "CLOSING ERROR pipets[1]: %s\r\n", strerror(errno));
            }
            if(dup2(pipets[0], STDIN_FILENO) == -1)          //get input from pipe
            {
                fprintf(stderr, "Error duplicating file descriptor: %s\r\n", strerror(errno));
            }
            
            if(execl(optarg, NULL) == -1)                    //execute program
            {
                fprintf(stderr, "Error executing program: %s\r\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            
            if(read(pipets[0], buf1, 1024) == -1)            //read contents from right pipe
            {
                fprintf(stderr, "READ ERROR: %s\r\n", strerror(errno));
            }
            if(close(pipets[0]) == -1)                       //close for EOF
            {
                fprintf(stderr, "CLOSING ERROR pipets[0]: %s\r\n", strerror(errno));
            }
            
            fprintf(stdout, "String read from parent: %s\n", buf1);     //expect input from parent
            
            //transform \r\n
            char buf2[1024];
            int i=0;
            int j=0;
            while(buf1[i] != '\0')
            {
                if(buf1[i] == '<')
                {
                    while(buf1[i] != '>')
                        i++;                //state of index at end of <lf> is saved for buf1
                    strcat(buf2, "<cr><lf>");
                    j = i+4;
                }
                else
                {
                    buf2[j] = buf1[i];
                }
                
                i++;
                j++;
            }
            
            // PIPE THAT RETURNS OUTPUT FROM THE PROGRAM
            // right child, right side of pipe for output back to left (parent)
            if(close(pipefs[0]) == -1)               //close unused read end
            {
                fprintf(stderr, "CLOSING ERROR pipefs[0]: %s\r\n", strerror(errno));
            }
            if(dup2(pipefs[1], STDOUT_FILENO) == -1) //set output to pipe
            {
                fprintf(stderr, "Error duplicating file descriptor: %s\r\n", strerror(errno));
            }
            if(write(pipefs[1], buf2, 1024) == -1)   //write result to left pipe (parent)
            {
                fprintf(stderr, "WRITE ERROR: %s\r\n", strerror(errno));
            }
            if(close(pipefs[1]) == -1)               //close for EOF
            {
                fprintf(stderr, "CLOSING ERROR pipefs[1]: %s\r\n", strerror(errno));
            }
        }
    }
    else
    {
        fprintf(stderr, "Error calling pipe: %s\r\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
//----------------------------------------------------------------------------------------
void getargs(int argc, char* argv[])
{
    char* usage_statement =
    "Usage: lab1a --program <path>\n";

    int option_index = 0;
    static struct option long_options[] =
    {
        {"program", required_argument, NULL, 1},
        {0, 0, 0, 0}
    };
    
    int c = getopt_long(argc, argv, "", long_options, &option_index);
    
    switch(c)
    {
        case -1:
            errno = EINVAL;
            fprintf(stderr, "GETOPT ERROR: %s\r\n", strerror(errno));
            fprintf(stderr, "%s\r\n", usage_statement);
            exit(EXIT_FAILURE);
        case 1:
            proc();
            break;
        case '?':
            errno = EINVAL;
            fprintf(stderr, "ARG ERROR: %s\r\n", strerror(errno));
            fprintf(stderr, "%s\r\n", usage_statement);
            exit(EXIT_FAILURE);
    }
}
//----------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    getargs(argc, argv);
    
    return 0;
}












