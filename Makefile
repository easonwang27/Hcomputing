ifndef CC
	CC = gcc
endif

CCFLAGS=-std=gnu11

LIBS = -lOpenCL

# Check our platform and make sure we define the APPLE variable
# and set up the right compiler flags and libraries
PLATFORM = $(shell uname -s)
ifeq ($(PLATFORM), Darwin)
	LIBS = -framework OpenCL
endif

block_read: block_read.c
	$(CC) $^ $(CCFLAGS) $(LIBS)  -o $@


clean:
	rm -f block_read