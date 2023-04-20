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

# Test dblp
DBLP25 = ./repeats -v -1 -2 -stl -m 4096 tests/pizza/dblp.25MB.txt tests/pizza/dblp.25MB.txt.2.lcp tests/pizza/dblp.25MB.txt.4.sa tests/pizza/dblp.25MB.txt.bwt
DBLP50 = ./repeats -v -1 -2 -stl -m 4096 tests/pizza/dblp.50MB.txt tests/pizza/dblp.50MB.txt.2.lcp tests/pizza/dblp.50MB.txt.4.sa tests/pizza/dblp.50MB.txt.bwt



all: ${PROJ_LABEL}

${PROJ_LABEL}: ${MCSRC} malloc_count.o
	${CC} ${CFLAGS} ${STXXL} ${MCLIB} $^ -o $@

malloc_count.o: $(MC) $(MC:.c=.h)
	gcc -Wall -Werror -c $< -o $@

clean:
	rm -rf ${PROJ_LABEL} malloc_count.o

debugvalgrind: all
# ${CC} ${CFLAGS} ${STXXL} ${MCLIB} ${MCSRC} -o ${PROJ_LABEL}
	valgrind --leak-check=yes --track-origins=yes -s $(DBLP25)
	echo __Type1 sequences in double__
	sort tests/pizza/dblp.25MB.txt.bwt.rt1 | uniq -d
	echo __Type2 sequences in double__
	sort tests/pizza/dblp.25MB.txt.bwt.rt2 | uniq -d

debug: all
	$(DBLP50)
	__Type1 sequences in double__
	sort tests/pizza/dblp.50MB.txt.bwt.rt1 | uniq -d
	_____________________________
	__Type2 sequences in double__
	sort tests/pizza/dblp.50MB.txt.bwt.rt2 | uniq -d
	_____________________________