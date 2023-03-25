CC = g++
CFLAGS = -O2 -Wall

all: FGK Huffman

Huffman: Huffman.cpp
	$(CC) $(CFLAGS) -o Huffman Huffman.cpp

FGK: FGK.cpp
	$(CC) $(CFLAGS) -o FGK FGK.cpp

clean:
	$(RM) FGK Huffman orig_* compr_*
