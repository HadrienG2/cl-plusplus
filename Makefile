
ifndef CPPC
	CPPC=g++ -c
endif
ifndef CPPLD
	CPPLD=g++
endif

CL_PLUSPLUS_DIR := CLplusplus
CL_PLUSPLUS_HEADERS := $(wildcard ${CL_PLUSPLUS_DIR}/*.hpp)
CL_PLUSPLUS_SOURCE := $(wildcard ${CL_PLUSPLUS_DIR}/*.cpp)
CL_PLUSPLUS_OBJ := $(patsubst %.cpp, %.o, ${CL_PLUSPLUS_SOURCE})

ALL_EXAMPLES := $(wildcard *.cpp)
ALL_EXECUTABLES := $(patsubst %.cpp, %.bin, ${ALL_EXAMPLES})

CCFLAGS := -Wall -Werror -O3 --std=c++11

INC :=

LIBS := -lOpenCL

.PHONY: all clean

all: $(ALL_EXECUTABLES) test

test: clpp_test.bin
	./clpp_test.bin

%.o: %.cpp $(CL_PLUSPLUS_HEADERS) Makefile
	$(CPPC) $< $(INC) $(CCFLAGS) -o $@

%.bin: %.o $(CL_PLUSPLUS_OBJ)
	$(CPPLD) $< $(CL_PLUSPLUS_OBJ) $(LIBS) -o $@
	rm $<

clean:
	rm -f $(CL_PLUSPLUS_DIR)/*.o
	rm -f *.o
	rm -f $(ALL_EXECUTABLES)
