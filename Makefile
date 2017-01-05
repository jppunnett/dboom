PROG=dboom
OBJS=dboom.o req.o
CFLAGS=-g -Wall -O0
LDLIBS=-ldill -lcurl

$(PROG): $(OBJS)

clean:
	$(RM) $(PROG) $(OBJS)