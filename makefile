##
# Project Title
#
# @file

CC = gcc
VERSION = 1.0

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

# end
