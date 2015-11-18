
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

CCFLAGS := -Wall -Werror -Wextra -O3 --std=c++11

INC :=

LIBS := -lOpenCL

.PHONY: all clean

ALL_EXECUTABLES := clpp_test.bin clpp_info.bin clpp_context.bin clpp_command_queue.bin clpp_buffer.bin
all: $(ALL_EXECUTABLES) test

test: clpp_test.bin
	./clpp_test.bin

%.o: %.cpp $(CL_PLUSPLUS_HEADERS)
	$(CPPC) $< $(INC) $(CCFLAGS) -o $@

CLPP_TEST_OBJ := clpp_test.o $(CL_PLUSPLUS_OBJ)
clpp_test.bin: $(CLPP_TEST_OBJ) $(CL_PLUSPLUS_HEADERS)
	$(CPPLD) $(CLPP_TEST_OBJ) $(LIBS) -o clpp_test.bin

CLPP_INFO_OBJ := clpp_info.o $(CL_PLUSPLUS_OBJ)
clpp_info.bin: $(CLPP_INFO_OBJ) $(CL_PLUSPLUS_HEADERS)
	$(CPPLD) $(CLPP_INFO_OBJ) $(LIBS) -o clpp_info.bin

CLPP_CONTEXT_OBJ := clpp_context.o $(CL_PLUSPLUS_OBJ)
clpp_context.bin: $(CLPP_CONTEXT_OBJ) $(CL_PLUSPLUS_HEADERS)
	$(CPPLD) $(CLPP_CONTEXT_OBJ) $(LIBS) -o clpp_context.bin

CLPP_COMMAND_QUEUE_OBJ := clpp_command_queue.o $(CL_PLUSPLUS_OBJ)
clpp_command_queue.bin: $(CLPP_COMMAND_QUEUE_OBJ) $(CL_PLUSPLUS_HEADERS)
	$(CPPLD) $(CLPP_COMMAND_QUEUE_OBJ) $(LIBS) -o clpp_command_queue.bin

CLPP_BUFFER_OBJ := clpp_buffer.o $(CL_PLUSPLUS_OBJ)
clpp_buffer.bin: $(CLPP_BUFFER_OBJ) $(CL_PLUSPLUS_HEADERS)
	$(CPPLD) $(CLPP_BUFFER_OBJ) $(LIBS) -o clpp_buffer.bin

clean:
	rm -f $(CL_PLUSPLUS_DIR)/*.o
	rm -f *.o
	rm -f $(ALL_EXECUTABLES)
