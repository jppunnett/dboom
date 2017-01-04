PROG=dboom
OBJS=dboom.o req.o
# CC=gcc
CFLAGS=-g -Wall -O0
LDLIBS=-ldill -lcurl

$(PROG): $(OBJS)

clean:
	$(RM) $(PROG) $(OBJS)