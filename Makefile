PROG=dboom
OBJS=dboom.o req.o url.o
CFLAGS+=-g -Wall -Werror -O0
LDLIBS+=-ldill -lcurl

$(PROG): $(OBJS)

clean:
	$(RM) $(PROG) $(OBJS)
