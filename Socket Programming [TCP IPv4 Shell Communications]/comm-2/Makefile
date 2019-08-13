#NAME:  Shaanan Curtis
#EMAIL: shaanancurtis@ieee.org

default:	lab2-server	lab2-client

lab2-server:	server.c
			gcc -g -Wall -Wextra server.c -o lab2-server

lab2-client:	client.c
			gcc -g -Wall -Wextra client.c -o lab2-client

dist:
		tar pczf comm-2.tar.gz *

extract:
		tar xzvf comm-2.tar.gz

clean:
		rm -r *.dSYM
		rm lab2-server
		rm lab2-client
		rm *.tar.gz