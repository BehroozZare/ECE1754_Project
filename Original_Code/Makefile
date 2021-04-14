# On Bridges we will check versus your performance versus Intel MKL library's BLAS. 

CC = g++
#OPT = -O3 -fopenmp -march=native 
OPT = -O3 -march=native 
CFLAGS = -Wall -std=gnu99 $(OPT) 
#MKLROOT = /opt/intel/composer_xe_2013.1.117/mkl
#LDLIBS = -lrt -Wl,--start-group $(MKLROOT)/lib/intel64/libmkl_intel_lp64.a $(MKLROOT)/lib/intel64/libmkl_sequential.a $(MKLROOT)/lib/intel64/libmkl_core.a -Wl,--end-group -lpthread -lm

# LDLIBS = -lrt  -I$(MKLROOT)/include -Wl,-L$(MKLROOT)/lib/intel64/ -lmkl_intel_lp64 -lmkl_core -lmkl_sequential -lpthread -lm -ldl

targets =  input_code
objects = input_code.o

.PHONY : default
default : all

.PHONY : all
all : clean $(targets)


input_code : input_code.o
	$(CC) -o $@ $^  
	
%.o : %.cpp
	$(CC) -c $(CFLAGS) $< 

.PHONY : clean
clean:
	rm -f $(targets) $(objects)
