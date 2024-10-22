CC = gcc
CXX = g++
CFLAGS = -Wall -Werror -g
CXXFLAGS = $(CFLAGS)
LDFLAGS = $(CFLAGS)

TARGETS = proj3

all: $(TARGETS)

$(TARGETS): proj2.o ArgParser.o WebClient.o
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o
	
distclean: clean
	rm -f $(TARGETS)