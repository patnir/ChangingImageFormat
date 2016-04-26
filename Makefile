TARGET = pe12
GCC = gcc
##
CFLAGS = -g -Wall -Wshadow -Werror
##
CC = $(GCC) $(CFLAGS)
##
SRCS = pe12.c bmp.c
##
VALGRIND = valgrind --tool=memcheck --verbose --log-file

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o pe12

clean:	
	rm -f pe12

test:
	$(VALGRIND)=./logfile ./pe12
