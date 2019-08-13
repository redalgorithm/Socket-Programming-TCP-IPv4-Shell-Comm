//Client Program
//Description in README.md

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <termios.h>
#include <fcntl.h>
//#include <mcrypt.h>       //problems with homebrew installation and linking (will figure it out)

unsigned int PORT = 0;
char* HOST = "localhost";
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
        "usage: lab1b-client [--port=value] [--host=domain]";
        
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
char* filename = "";
char* keyfile = "";
int fd = -1;
void processArgs(int argc, char* argv[])
{
    int flag = 0;       //tracks mandatory port option
    
    while(1)
    {
        int option_index = 0;
        static struct option long_options[] =
        {
            {"port", required_argument, NULL, 1},
            {"log", required_argument, NULL, 2},
            {"encrypt", required_argument, NULL, 3},
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
                filename = optarg;
                fd = open(optarg, O_CREAT | O_WRONLY | O_TRUNC, 0700);
                error("LOG OPTION CURRENTLY UNAVAILABLE (wait for update)", 0, 0);  //remove after including log feature
                break;
            case 3:
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
// FUNCTION: clientToServer
// ---------------------------------
// Creates a socket on client side
// and connects client to server.
//
// ARGS: NA
// RETURNS: int
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
int clientToServer()
{
    if(strcmp(HOST, "localhost") == 0)
        fprintf(stdout, "[+] Assigning default hostname\n");
    
    
    //CREATE SOCKET
    //Returned descriptor for call to socket (comm endpoint)
    int clientsocketfd;
    //Create a socket for comm between self & server (Socket Family: IPv4, Socket Protocol: TCP)
    if((clientsocketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("ERROR opening socket", errno, 0);
    fprintf(stdout, "[+] Creating IPv4 socket using TCP protocol\n");
    
    
    //SET UP IP ADDRESS
    //Convert hostname to IP address
    struct hostent* server = gethostbyname(HOST); //return a pointer to obj describing host (server) = unreliable
    if(server == NULL)
        error("No Such Host", errno, 0);
    
    //IPv4 struct for containing 32b IP address & Port [ encode the ip address and the port for the remote ]
    struct sockaddr_in serverADDR;
    memset(&serverADDR, '\0', sizeof(struct sockaddr_in));  //initialize serverADDR with bytes of 0 (prep for copy)
    
    //SETTING SOCKADDR_IN STRUCT FIELDS-------------------------------------------------
    serverADDR.sin_family = AF_INET;    //server address is IPv4
    
    //Copy IP address from server to serverADDR
    memcpy(&serverADDR.sin_addr.s_addr, server->h_addr, server->h_length);
    fprintf(stdout, "[+] Getting server IP address: %d\n", serverADDR.sin_addr.s_addr);

    
    //SET UP PORT
    //1. convert port # from host-byte-order to network-byte-order
    //2. assign converted port # to serverADDR
    serverADDR.sin_port = htons(PORT);
    fprintf(stdout, "[+] Specifying port #%d\n", htons(serverADDR.sin_port));
    //SETTING SOCKADDR_IN STRUCT FIELDS-------------------------------------------------

    //Initiate connection to the server through clientsocket (using server IP address and associated port #)
    if(connect(clientsocketfd, (struct sockaddr*)&serverADDR, sizeof(serverADDR)) < 0)
        error("ERROR Connecting to Server", errno, 0);
    fprintf(stdout, "[+] Connecting to server\n");
    
    //The server/client reads characters from the socket connection into this buffer.
    //char buffer[1024];
    //printf("[+] Allocating buffer for communicating to server\n");
    
    return clientsocketfd;
}
//-----------------------------------------------------------------------------------------------------
//----------------------------------
// FUNCTION: reset
// ---------------------------------
// Resets STDIN attributes back to
// default.
//
// ARGS: NA
// RETURNS: NA
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
struct termios savedparams;
void reset()
{
    if(tcsetattr(0, TCSANOW, &savedparams) < 0)
        error("ERROR setting attributes", errno, 0);
}
//-----------------------------------------------------------------------------------------------------
//----------------------------------
// FUNCTION: setInputModes
// ---------------------------------
// Set input to non-canonical/
// no echo mode.
//
// ARGS: NA
// RETURNS: NA
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
void setInputModes()
{
    struct termios tempattr;
    
    //Check if stdin (fd0) refers to terminal.
    if(!isatty(0))
        error("ERROR referencing terminal", errno, 0);
    
    //Save the terminal attributes for restoration.
    if(tcgetattr (0, &savedparams) < 0)
        error("ERROR saving attributes", errno, 0);
    
    //Get current terminal modes.
    if(tcgetattr (0, &tempattr) < 0)
        error("ERROR getting attributes", errno, 0);
    
    // Modify to fit according to non-canonical/no echo mode.
    tempattr.c_iflag = ISTRIP;      //set input flag to only 7 lowest bits (strip 8th)
    tempattr.c_oflag = 0;           //set output flag to 0 (no processing)
    tempattr.c_lflag = 0;           //set local flag to 0 (no processing)
    
    //Set new terminal modes (non-canonical, no-echo input).
    if(tcsetattr(0, TCSANOW, &tempattr) < 0)
        error("ERROR setting attributes", errno, 0);
}
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
/*
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
// FUNCTION: keyboardEntry
// ---------------------------------
// Take in input from the keyboard
// and write to the server via TCP
// socket.
//
// ARGS: NA
// RETURNS: NA
// ---------------------------------
//-----------------------------------------------------------------------------------------------------
void keyboardEntry(int socketfd)
{
    /*
    if(strlen(keyfile) > 0)
        prepEncryption();
    */
    
    fprintf(stdout, "\nEnter a Command:\n");
    
    setInputModes();
    char buffer[256] = "";
    char c = '~';
    char* temp = "";
    
    int i=0;
    while(1)
    {
        
        if(read(0, &c, sizeof(c)) < 0)
            error("ERROR reading from STDIN", errno, 0);
        
        if(c == '\003')
        {
            if(write(1, "^C", 3) < 0)
                error("ERROR writing to STDOUT", errno, 0);
            
            temp = "^C";
            
            /*
            if(strlen(keyfile) > 0)
                mcrypt_generic(ed, temp, (int)strlen(temp));      //encrypt each byte in blockbuff
            */
            
            strcat(buffer, temp);
            break;
        }
        if(c == '\004')
        {
            if(write(1, "^D", 3) < 0)
                error("ERROR writing to STDOUT", errno, 0);
            
            temp = "^D";
            
            /*
            if(strlen(keyfile) > 0)
                mcrypt_generic(ed, temp, (int)strlen(temp));
             */
            
            strcat(buffer, temp);
            i+=2;
            break;
        }
        else if(c == '\r' || c == '\n')
        {
            if(write(1, "<cr><lf>", 9) < 0)
                error("ERROR writing to STDOUT", errno, 0);
            
            temp = "<lf>";
            
            /*
            if(strlen(keyfile) > 0)
                mcrypt_generic(ed, temp, (int)strlen(temp));
             */
            
            strcat(buffer, temp);
            i+=4;
        }
        else
        {
            if(write(1, &c, 1) < 0)
                error("ERROR writing to STDOUT", errno, 0);
            
            /*
            if(strlen(keyfile) > 0)
                mcrypt_generic(ed, &c, sizeof(c));
             */
            
            buffer[i] = c;
            i++;
        }
    }
    fprintf(stdout, "\n");
    reset();
    
    if(fd > -1)
    {
        int bytes = (int)strlen(buffer);
        dprintf(fd, "SENT %d bytes: %s\n", bytes, buffer);
    }
    
    if(write(socketfd, buffer, strlen(buffer)) < 0)
        error("ERROR writing to socket", errno, 0);
    
    /*
    //deprecated: use generic_deinit and module_close instead
    //mcrypt_generic_end(ed);     //terminate encryption (clear buffers, close modules, returns - on error)
    if(strlen(keyfile) > 0)
    {
        mcrypt_generic_deinit(ed);  //clear buffers
        mcrypt_module_close(ed);    //close modules
    }
    */
}
//-----------------------------------------------------------------------------------------------------
// MAIN PROGRAM
//-----------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    processArgs(argc, argv);
    int socketfd = clientToServer();    //connect client to server
    keyboardEntry(socketfd);            //input from keyboard and send data through socket to server
    
    char buffer[1024];
    
    /*
    if(strlen(keyfile) > 0)
        prepEncryption();
     */
    
    if(read(socketfd, buffer, 1024) < 0)
        error("ERROR reading from socket", errno, 0);
    
    /*
    if(strlen(keyfile) > 0)
        mdecrypt_generic(ed, buffer, (int)strlen(buffer));
     */
    
    if(fd > -1)
    {
        int bytes = (int)strlen(buffer);
        dprintf(fd, "RECEIVED %d bytes: %s\n", bytes, buffer);
        if(close(fd) < 0)
            error("ERROR closing descriptor", errno, 0);
    }
    
    fprintf(stdout, "\nServer returned:\n");
    fprintf(stdout, "%s\n", buffer);
    
    if(close(socketfd) < 0)
        error("ERROR closing socket endpoint", errno, 0);
    
    /*
    //deprecated: use generic_deinit and module_close instead
    //mcrypt_generic_end(ed);     //terminate encryption (clear buffers, close modules, returns - on error)
    if(strlen(keyfile) > 0)
    {
        mcrypt_generic_deinit(ed);  //clear buffers
        mcrypt_module_close(ed);    //close modules
    }
     */
    return 0;
}






























