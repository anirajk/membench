CXXFLAGS := -std=c++14 -O3 -Idocopt.cpp -Wall -Werror
#CXXFLAGS := -std=c++14 -g -Idocopt.cpp -Wall -Werror
LDFLAGS := -lpthread

SRCS := $(wildcard *.cc)
OBJS := $(patsubst %.cc, %.o, $(SRCS)) docopt.o

mem-bench: $(OBJS)
	$(CXX) $^ -o mem-bench $(LDFLAGS)

docopt.o : docopt.cpp/docopt.cpp
	$(CXX) $(CXXFLAGS) -Wno-unknown-pragmas -c $<

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c $<


.PHONY: clean
clean:
	-rm *.o  mem-bench

all: mem-bench 

