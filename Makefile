# Name the project executable
PROJ_LABEL = repeats

# Main code source
MCSRC = repeat.cpp

# Compiler
CC = g++

# Compile Flags
CFLAGS = 	\
	# -Wall -Werror


# Malloc_Count
MC = external/malloc_count/malloc_count.c
MCLIB = -ldl

# STXXL
STXXL = -Iexternal/stxxl/include/ -Lexternal/stxxl/lib

all: ${PROJ_LABEL}

${PROJ_LABEL}: ${MCSRC} malloc_count.o
	${CC} -std=c++20 ${CFLAGS} ${STXXL} ${MCLIB} $^ -o $@

malloc_count.o: $(MC) $(MC:.c=.h)
	gcc -Wall -Werror -c $< -o $@

clean:
	rm -rf ${PROJ_LABEL} malloc_count.o
