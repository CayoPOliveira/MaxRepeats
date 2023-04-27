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

# Tests
DNA20 = ./repeats -v -1 -2 -stl -m 10 tests/pizza/dna.20MB.txt tests/pizza/dna.20MB.txt.bwt tests/pizza/dna.20MB.txt.2.lcp tests/pizza/dna.20MB.txt.4.sa
DNA40 = ./repeats -v -1 -2 -stl -m 10 tests/pizza/dna.40MB.txt tests/pizza/dna.40MB.txt.bwt tests/pizza/dna.40MB.txt.2.lcp tests/pizza/dna.40MB.txt.4.sa
DNA60 = ./repeats -v -1 -2 -stl -m 10 tests/pizza/dna.60MB.txt tests/pizza/dna.60MB.txt.bwt tests/pizza/dna.60MB.txt.2.lcp tests/pizza/dna.60MB.txt.4.sa
DNA80 = ./repeats -v -1 -2 -stl -m 10 tests/pizza/dna.80MB.txt tests/pizza/dna.80MB.txt.bwt tests/pizza/dna.80MB.txt.2.lcp tests/pizza/dna.80MB.txt.4.sa
DNA100 = ./repeats -v -1 -2 -stl -m 10 tests/pizza/dna.100MB.txt tests/pizza/dna.100MB.txt.bwt tests/pizza/dna.100MB.txt.2.lcp tests/pizza/dna.100MB.txt.4.sa


all: ${PROJ_LABEL}

${PROJ_LABEL}: ${MCSRC} malloc_count.o
	${CC} ${CFLAGS} ${STXXL} ${MCLIB} $^ -o $@

malloc_count.o: $(MC) $(MC:.c=.h)
	gcc -Wall -Werror -c $< -o $@

clean:
	rm -rf ${PROJ_LABEL} malloc_count.o

debugvalgrind: all
	valgrind --leak-check=yes --track-origins=yes -s $(DNA20)
	@echo __Type1 sequences in double__
	@sort tests/pizza/dna.20MB.txt.bwt.rt1 | uniq -d
	@echo __Type2 sequences in double__
	@sort tests/pizza/dna.20MB.txt.bwt.rt2 | uniq -d

debug: all
	$(DNA20)
	@echo __Type1 sequences in double__
	@sort tests/pizza/dna.20MB.txt.bwt.rt1 | uniq -d
	@echo __Type2 sequences in double__
	@sort tests/pizza/dna.20MB.txt.bwt.rt2 | uniq -d