# Text Compression Demonstration

This is a demonstration of various compression algorithms for my CPSC4660
(Database Management Systems) class final project.

## Algorithms

### Static Huffman Encoding

The static Huffman encoding algorithm works by first generating a frequency
table from the source file, then building a Huffman tree from it. This tree
is a binary tree, with the leaf nodes representing the symbols we see in the
source file. Each symbol is then encoded as the sequence of left/right 
traversals through the tree to reach its corresponding node. By storing the
frequent symbols near the top of the tree, we can shorten (on average) the
number of bits needed to represent it.

It is worth noting that my implementation does not encode the representation
of the Huffman tree we used to encode the source file. In a real-world scenario
we would need to also save some representation of the tree so that the decoder
can actually function. Lelewer & Hirschberg (1987) suggest that an optimal
representation of the tree takes 2n bits.

### Dynamic/Adaptive Huffman Encoding (FGK Algorithm)

The dynamic version of Huffman encoding removes the need to perform an
initial scan to generate the frequency table and Huffman tree. We instead
create and update the Huffman tree as we encode the source file. This means
we no longer have to encode the entire tree along with the file, as it can
be generated during decoding process.

## Results

![Results Table](https://github.com/dustin-ward/text-compression/blob/master/images/results.jpg?raw=true)

## Usage

Use provided makefile to build the programs.

`make all`

`./Huffman ./testing_data/lorem1000.txt`

`./FGK ./testing_data/lorem1000.txt`
