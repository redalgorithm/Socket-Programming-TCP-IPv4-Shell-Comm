#Comm-1 Makefile
#Description in README.md

lab1:	main.c
		gcc -g -Wall -Wextra main.c -o lab1

dist:
		tar pczf comm-1.tar.gz *

extract:
		tar xzvf comm-1.tar.gz

clean:
		rm -r *.dSYM
		rm lab1
		rm *.tar.gz
