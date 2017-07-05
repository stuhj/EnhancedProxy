HOME_WIN = /mnt/h/Hou/Documents
MUDUO_DIRECTORY = $(HOME_WIN)/muduo-cpp11/build/release-install-cpp11
#MUDUO_DIRECTORY ?= $(HOME)/build/install 
MUDUO_INCLUDE = $(MUDUO_DIRECTORY)/include
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/lib

CXXFLAGS = -g -std=c++11 -O0 -Wall -Wextra 
#CXXFLAGS += -Wconversion -Wno-unused-parameter
#CXXFLAGS += -Wold-style-cast -Woverloaded-virtual
CXXFLAGS +=-Wpointer-arith -Wshadow -Wwrite-strings 
CXXFLAGS +=-march=native -rdynamic 
CXXFLAGS +=-I$(MUDUO_INCLUDE) 

LDFLAGS = -L$(MUDUO_LIBRARY) -lmuduo_net -lmuduo_base  -lpthread -lrt 

all: server 
clean: 
	rm -f server core 
server: Test.cpp ../MasterProcess.cpp ../WorkerProcess.cpp
	#_HttpContext.cpp
	g++ $(CXXFLAGS) -I ../ -o $@ $^ $(LDFLAGS) 

.PHONY: all clean 