#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
//#include <mcrypt.h>       //problems with homebrew installation and linking (will figure it out)

unsigned int PORT = 0;
//-----------------------------------------------------------------------------------------------------
//----------------------------------
// FUNCTION: error
// ---------------------------------
// Simple and uniform approach to
// error invocation in response to
// failure.
//
// ARGS: char*, int
// RETURNS: NA
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
void error(char* msg, int err, int flag)
{
    if(err != 0)
        errno = err;
    
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    
    if(flag)
    {
        char* statement =
        "usage: lab1b-client [--port=value]";
        
        fprintf(stderr, "%s\n", statement);
    }
    exit(EXIT_FAILURE);
}
//-----------------------------------------------------------------------------------------------------
//----------------------------------
// FUNCTION: processArgs
// ---------------------------------
// Handles arguments passed in
// program execution.
//
// ARGS: int, char*
// RETURNS: NA
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
char* keyfile = "";
void processArgs(int argc, char* argv[])
{
    int flag = 0;       //tracks mandatory port option
    
    while(1)
    {
        int option_index = 0;
        static struct option long_options[] =
        {
            {"port", required_argument, NULL, 1},
            {"encrypt", required_argument, NULL, 2},
            {0, 0, 0, 0}
        };
        
        int c = getopt_long(argc, argv, "", long_options, &option_index);
        
        if(c == -1)
            break;
        
        switch(c)
        {
            case 1:
                PORT = atoi(optarg);    //convert arg from string of digits to int (more precise than casting)
                flag = 1;
                break;
            case 2:
                keyfile = optarg;
                error("MCRYPT CURRENTLY INOPERABLE", 0, 0);     //remove after solving linker issue
                break;
            case '?':
                error("OPT ERROR", EINVAL, 1);
        }
    }
    
    if(!flag)
        error("MISSING PORT", EINVAL, 1);
}
//-----------------------------------------------------------------------------------------------------
//----------------------------------
// FUNCTION: serverToClient
// ---------------------------------
// Creates a socket on server side
// and connects server to client.
//
// ARGS: NA
// RETURNS: int
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
int serverToClient()
{
    //CREATE LISTENING SOCKET
    //Returned descriptor for call to socket (comm end-point)
    int listensocketfd;
    //Create a socket for comm between self & client (Socket Family: IPv4, Socket Protocol: TCP)
    if((listensocketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("ERROR opening socket for listening", errno, 0);
    fprintf(stdout, "[+] Creating IPv4 listening socket using TCP protocol\n");
    
    //CONFIGURE SERVER FOR ACCEPTING CONNECTIONS---------------------------------------------------------------
    //IPv4 struct for containing 32b IP address & Port [ encode the ip address and the port for the remote ]
    struct sockaddr_in serverADDR;                          //server address
    memset(&serverADDR, 0, sizeof(struct sockaddr_in));     //initialize serverADDR with bytes of 0 (prep for copy)
    
    serverADDR.sin_family = AF_INET;            //server address is IPv4
    serverADDR.sin_addr.s_addr = INADDR_ANY;    //server can accept connection from any client IP (?)
    serverADDR.sin_port = htons(PORT);          //set port number
    fprintf(stdout, "[+] Configuring server for accepting connections\n");
    
    //SETUP LISTENING SOCKET
    //bind listening socket to port
    if(bind(listensocketfd, (struct sockaddr*)&serverADDR, sizeof(serverADDR)) < 0)
        error("ERROR binding socket to port", errno, 0);
    fprintf(stdout, "[+] Binding socket to port\n");
    
    //Let the socket listen, maximum 5 pending connections
    if(listen(listensocketfd, 5) < 0)
        error("ERROR listening on socket", errno, 0);
    fprintf(stdout, "[+] Socket is now listening...\n");
    
    //PROCEDURE FOR ACCEPTING INCOMING CONNECTIONS
    //IPv4 struct for containing 32b IP address & Port [ encode the ip address and the port for the remote ]
    struct sockaddr_in clientADDR;              //client address
    //Length of client address
    socklen_t clientlen = sizeof(clientADDR);
    
    //New fd returned from call to accept (accepted socket connection)
    int retfd;
    //Wait for client's connection, clientADDR stores client's IP address
    if((retfd = accept(listensocketfd, (struct sockaddr*)&clientADDR, &clientlen)) < 0)
        error("ERROR accepting connection", errno, 0);
    fprintf(stdout, "[+] Connection accepted from client @%d\n", clientADDR.sin_addr.s_addr);
    
    return retfd;
}
/*
//-----------------------------------------------------------------------------------------------------
//----------------------------------
// FUNCTION: prepEncryption
// ---------------------------------
// Prepare for encryption
// (not tested - requires mcrypt library)
//
// ARGS: NA
// RETURNS: NA
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
MCRYPT ed;
void prepEncryption()
{
    int i;
    char* key;                  //save key d
    char password[20];
    char* IV;                   //random block as first block (size = blocksize)
    int keysize=16;             //128 bits
    
    //------------------------------------------------------------------------
    //The calloc() function contiguously allocates enough space for count
    //objects that are size bytes of memory each and returns a pointer to the
    //allocated memory.  The allocated memory is filled with bytes of value
    //zero.
    //------------------------------------------------------------------------
    key=calloc(1, keysize);
    char tempkey[20];
    int fd = open(keyfile, O_RDONLY);
    read(fd, tempkey, 20);
    close(fd);
    strcpy(password, tempkey);                        //set password
    memmove(key, password, strlen(password));               //move password into key by byte
    ed = mcrypt_module_open("twofish", NULL, "cfb", NULL);  //open library for twofish algorithm with mode cfb
    
    //exit failure if failed to encrypt with specified algorithm
    if(ed==MCRYPT_FAILED)
        error("ERROR opening encryption mod", errno, 0);
    
    //The malloc() function allocates size bytes of memory and returns a
    //pointer to the allocated memory.
    IV = malloc(mcrypt_enc_get_iv_size(ed));
    
    //fill IV with random chars
    for(i=0; i<mcrypt_enc_get_iv_size(ed); i++)
    {
        IV[i] = rand();
    }
    
    //can use the descriptor for encryption or decryption
    i = mcrypt_generic_init(ed, key, keysize, IV);  //same IV must be used for encrypt/decrypt
    if(i<0)
    {
        mcrypt_perror(i);
        exit(1);
    }
}
 */
//-----------------------------------------------------------------------------------------------------
//----------------------------------
// FUNCTION: execShell
// ---------------------------------
// Forks a child process and executes
// a shell to process commands received
// from the client; communicating with
// the shell via pipes.
//
// ARGS: int
// RETURNS: NA
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
int execShell(int socketfd)
{
    int pipepc[2];      //pipe to shell i/o descriptors
    int pipecp[2];      //pipe from shell i/o descriptors
    
    if(pipe(pipepc)==0 && pipe(pipecp)==0)
    {
        int status = 0;
        pid_t pid = fork();     //fork process
        
        //ERROR
        if(pid < 0)
            error("ERROR forking the process", errno, 0);
        //PARENT (server)
        else if(pid > 0)
        {
            /*
            if(strlen(keyfile) > 0)
                prepEncryption();
             */
            
            char tcpbuff[256];
            if(read(socketfd, tcpbuff, 256) < 0)
                error("ERROR reading from socket", errno, 0);
            
            /*
            if(strlen(keyfile) > 0)
                mdecrypt_generic(ed, tcpbuff, (int)strlen(tcpbuff));
             */
            
            //Ensure one command per input by stripping after return/EOF/Kill
            int i=0, j=0, killit=0;
            while(i < (int)strlen(tcpbuff))
            {
                if(j>0 || tcpbuff[i] == '<' || tcpbuff[i] == '^')
                {
                    if(tcpbuff[i] == '^' && tcpbuff[i+1] == 'C')
                        killit = 1;

                    tcpbuff[i] = '\0';
                    j++;
                }
                i++;
            }
            
            if(close(pipepc[0]) < 0)
                error("ERROR closing pipe", errno, 0);
            if(write(pipepc[1], tcpbuff, strlen(tcpbuff)) < 0)
                error("ERROR writing to pipe", errno, 0);
            if(close(pipepc[1]) < 0)
                error("ERROR closing pipe", errno, 0);
            
            /*
            //deprecated: use generic_deinit and module_close instead
            //mcrypt_generic_end(ed);     //terminate encryption (clear buffers, close modules, returns - on error)
            if(strlen(keyfile) > 0)
            {
                mcrypt_generic_deinit(ed);  //clear buffers
                mcrypt_module_close(ed);    //close modules
            }
            */
            
            //wait for child to exit before killing the process

            if(waitpid(pid, &status, 0) < 0)
                error("ERROR terminating shell", errno, 0);
            
            if(killit)
            {
                if(close(pipepc[1]) < 0)
                    error("ERROR closing pipe", errno, 0);
                
                if(kill(pid, SIGINT) < 0)
                    error("ERROR killing child process", errno, 0);
                
                fprintf(stdout, "[+] SIGINT received: terminating shell\n");
            }
            
            /*
            if(strlen(keyfile) > 0)
                prepEncryption();
            */
            
            char buffer[1024];
            if(close(pipecp[1]) < 0)
                error("ERROR closing pipe", errno, 0);
            if(read(pipecp[0], buffer, 1024) < 0)
                error("ERROR reading from pipe", errno, 0);
            if(close(pipecp[0]) < 0)
                error("ERROR closing pipe", errno, 0);
            
            /*
            if(strlen(keyfile) > 0)
                mcrypt_generic(ed, buffer, sizeof(buffer));
             */
            
            if(write(socketfd, buffer, strlen(buffer)) < 0)
                error("ERROR writing to socket", errno, 0);
            if(close(socketfd) < 0)
                error("ERROR closing socket", errno, 0);
            
            /*
            //deprecated: use generic_deinit and module_close instead
            //mcrypt_generic_end(ed);     //terminate encryption (clear buffers, close modules, returns - on error)
            if(strlen(keyfile) > 0)
            {
                mcrypt_generic_deinit(ed);  //clear buffers
                mcrypt_module_close(ed);    //close modules
            }
            */
            
            //EXIT PROCESS
            int low = status & 0x007f;
            int high = status & 0xff;
            fprintf(stderr, "\r\nSHELL EXIT SIGNAL=%d STATUS=%d", low, high);
            printf("\r\n");
            exit(EXIT_SUCCESS);
            
            /*
             notes:
             pipe stores output pipe[1] in input pipe[0] for read on other end
             free input (fd3)
             store in input
             free output (fd4), both ends closed? continue to next proc
             make stored input fd0
             */
        }
        //CHILD (shell)
        else
        {
            char buffer[256];
            if(close(pipepc[1]) < 0)
                error("ERROR closing pipe", errno, 0);
            if(dup2(pipepc[0], STDIN_FILENO) < 0)
                error("ERROR duplicating descriptor", errno, 0);
            if(dup2(pipecp[1], STDERR_FILENO) < 0)
                error("ERROR duplicating descriptor", errno, 0);
            if(dup2(pipecp[1], STDOUT_FILENO) < 0)
                error("ERROR duplicating descriptor", errno, 0);
            if(execl("/bin/bash", NULL) < 0)
                error("ERROR executing shell", errno, 0);
            if(read(pipepc[0], buffer, 256) < 0)
                error("ERROR reading from pipe", errno, 0);
            if(close(pipepc[0]) < 0)
                error("ERROR closing pipe", errno, 0);
        }
    }
    else
        error("ERROR calling pipe", errno, 0);
    
    return -1;
}
//-----------------------------------------------------------------------------------------------------
// MAIN PROGRAM
//-----------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    processArgs(argc, argv);
    
    int socketfd = serverToClient();
    
    if(execShell(socketfd) < 0)
        error("Process incomplete", EMSGSIZE, 0);
    
    return 0;
}

























