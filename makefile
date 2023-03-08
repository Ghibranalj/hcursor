##
# Project Title
#
# @file

CC = gcc
VERSION = 1.1

LIBS =-lX11 -lXfixes
CFLAGS= -DVERSION=\"$(VERSION)\"

SRCs = $(wildcard *.c)
OBJS = $(SRCs:.c=.o)

hcursor: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

run: hcursor
	./hcursor -x

.PHONY: clean
clean:
	rm -rf $(OBJS) hcursor
# end
