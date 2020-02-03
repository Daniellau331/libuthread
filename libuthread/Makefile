# Target library
lib := libuthread.a
CFLAGS = -Wall -Werror -c

all: $(lib)

## TODO: Phase 1.1
# $< means name of first prerequisite of target
# $@ means target
# $^ means all prerequisites of target
# %.o: %.c generates any file from from another file

%.o: %.c context.h
	gcc $(CFLAGS) $<

%.o: %.c preempt.h
	gcc $(CFLAGS) $<

%.o: %.c queue.h
	gcc $(CFLAGS) $<

%.o: %.c uthread.h
	gcc $(CFLAGS) $<

libuthread.a: context.o preempt.o queue.o uthread.o
	ar rcs libuthread.a $^

clean:
	rm -f *.o *.a