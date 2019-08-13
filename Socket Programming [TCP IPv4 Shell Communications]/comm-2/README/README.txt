Created by Shaanan Curtis 2019.

DESCRIPTION ------------------------------------------------------------
Included in this project are two C source files, representing the
server and client for demonstrating encrypted* communication via 
IPv4 TCP socket. Additional items include a key file and a Makefile
for building the program and tarball.

SERVER.C
The c file contains the source code for the server program. This module 
was designed to pipe decrypted* data received from an IPv4 TCP socket end-point
to an executed shell program and send resulting output back through
the socket to the client program.

CLIENT.C
The c file contains the source code for reading a stream of
text from the keyboard and sending via IPv4 TCP socket
to server program.  Additional functionality includes
printing received data to console.

options included in either or both programs:
--port (gathers port # and uses it to make a connection)
#--log (allows programmer to log data sent/received to file)*
#--encrypt (allows programmer to utilize mcrypt library)*
#encryption not fully tested

MAKEFILE
The Makefile creates several object files which are used in the 
compilation process of lab2-client/server.c and includes additional options
for cleaning, distributing, and extracting the tarball.

lab2-server:
Anytime a change is made to lab1b-server.c, recompile lab1b-server.c into
the executable lab1b-server.

lab2-client:
Anytime a change is made to lab1b-client.c, recompile lab1b-client.c into
the executable lab1b-client.

dist:
Builds a distribution tarball containing project1b submission
items.

extract:
Additional target included for tarball extraction.

clean:
deletes all files created by the Makefile and returns the
directory to its freshly un-tarred state.

------------------------------------------------------------------------------
* Feature unavailable until next update