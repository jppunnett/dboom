PROG=dboom
OBJS=dboom.o req.o url.o
CFLAGS+=-g -Wall -Werror -O0
LDLIBS+=-ldill

$(PROG): $(OBJS)

clean:
	$(RM) $(PROG) $(OBJS)
