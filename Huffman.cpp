// Static Huffman Encoding
// Author: Dustin Ward
// Date: March 24th, 2023
//
// This is an implementation of static huffman encoding for my CPSC4660
// (Database Managment Systems) class. The goal of this project is to
// demonstrate the performance of various text compression methods.
//
// The static Huffman encoding algorithm works by first generating a frequency
// table from the source file, then building a Huffman tree from it. This tree
// is a binary tree, with the leaf nodes representing the symbols we see in the
// source file. Each symbol is then encoded as the sequence of left/right 
// traversals through the tree to reach its corresponding node. By storing the
// frequent symbols near the top of the tree, we can shorten (on average) the
// number of bits needed to represent it.
//
// This file (when supplied with a source file as the first argument) will
// encode it using this static huffman algorithm, and write it to file. The 
// file has the name "compr_huffman.dat". Then the file will be decoded and
// written to disk again as "orig_huffman.dat". We can diff the original source
// file with "orig_huffman.dat" to ensure the process has not lost any data.

#include <bits/stdc++.h>

// Representation of a node in the Huffman tree
struct TreeNode {
    char c;
    int freq;

    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    TreeNode(char c, int f) : c(c), freq(f) {};
};

// Custom comparator for usage in PQ later.
// Special emphasis is placed on the EOF character to ensure it has the lowest
// frequency among all characters.
struct CompareTreeNode {
    bool operator()(TreeNode* a, TreeNode* b) {
        if(b->c == 3) return true;
        if(a->c == 3) return false;
        return a->freq >= b->freq;
    }
};

const char END_TEXT = -1;

// Scan through each character in the input data and lookup the Huffman code
// corresponding to it. We need to use a byte as a buffer to write the
// individual bytes to disk.
//
// Returns a vector of bytes representing the encoded file to be written.
std::vector<char>* encode(std::map<char,std::string>* codes,
        char* data, int N) {
    std::vector<char>* output = new std::vector<char>;

    char buffer = 0;
    int counter = 0;
    int dataPos = 0;
    int codePos = 0;
    std::string curCode = (*codes)[data[0]];
    while(dataPos < N) {
        // Do we need to move to the next byte?
        if(codePos == (int)curCode.length()) {
            dataPos++;
            curCode = (*codes)[data[dataPos]];
            codePos = 0;
        }

        // Copy bit in the code to our buffer
        if(curCode[codePos++] == '1')
            buffer |= 1;
        counter++;

        // Do we need to flush our buffer?
        if(counter==8) {
            output->push_back(buffer);
            buffer = 0;
            counter = 0;
        }
        
        // Move to next bit in buffer
        buffer <<= 1;
    }

    // Flush any remaining data
    output->push_back(buffer);

    return output;
}

std::vector<char>* decode(TreeNode* hfTree, char* buffer, int N) {
    std::vector<char>* output = new std::vector<char>;

    TreeNode* curNode = hfTree;
    int i = 0;
    int bitIdx = 7;
    while(i < N) {
        // Test bit 'bitIdx' and traverse tree accordingly
        if(buffer[i] & (1<<bitIdx--))
            curNode = curNode->right;
        else
            curNode = curNode->left;
        
        // Do we need to move to the next byte in our data?
        if(bitIdx < 0) {
            i++;
            bitIdx = 7;
        }

        // Have we found a leaf in our tree?
        if(curNode->c != 0) {
            if(curNode->c == END_TEXT)
                return output;
            output->push_back(curNode->c);
            curNode = hfTree;
        }
    }

    return output;
}

// Create lookup table of codes associated with each symbol.
void gen_huffman_codes(std::map<char,std::string>* codes,
        TreeNode* root, std::string code="") {
    if(!root) return;

    // We have found a leaf (or symbol) in our tree.
    // We can now finish our current code.
    if(root->c != 0)
        codes->insert({root->c, code});

    // Traversing left is represented as a '0' in the binary code, while
    // traversing right is represented as a '1'.
    gen_huffman_codes(codes, root->left, code + '0');
    gen_huffman_codes(codes, root->right, code + '1');
}

TreeNode* build_huffman_tree(std::map<char,int>* freq) {
    // MinHeap built on frequency
    std::priority_queue<TreeNode*, std::vector<TreeNode*>, CompareTreeNode> PQ;

    // Initialize leaf node for each char
    for(auto &[c,f] : *freq)
        PQ.push(new TreeNode(c,f));

    // Tree merging step
    while((int)PQ.size() > 1) {
        // Grab the 2 lowest frequency nodes
        TreeNode* left = PQ.top();
        PQ.pop();

        TreeNode* right = PQ.top();
        PQ.pop();

        // Merge them together with combined frequency
        TreeNode* parent = new TreeNode(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        PQ.push(parent);
    }
    
    // Remaining node is root of huffman tree
    return PQ.top();
}

// Create frequency table for each character in the input file
std::map<char,int>* gen_freq_table(char* buffer, int N) {
    std::map<char,int>* M = new std::map<char,int>;

    for(int i=0; i<N; i++) {
        (*M)[buffer[i]]++;
    }

    return M;
}

int main (int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "no filename provided" << std::endl;
        return -1;
    }
    
    // Open file
    std::ifstream ifs(argv[1], std::ios::binary | std::ios::ate);
    if(!ifs) {
        std::cerr << "error opening file" << std::endl;
        return -1;
    }

    // Determine length
    int data_size = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    // Read data
    char* buffer = new char[data_size+1];
    ifs.read(buffer, data_size);
    buffer[data_size] = END_TEXT; // Insert ASCII 3 as psuedo-sybol for EOF
    ifs.close();

    std::cout << "Compressing file..." << std::endl;

    // Generate frequency table
    std::map<char,int>* freq_table;
    freq_table = gen_freq_table(buffer, data_size+1);

    // Create the huffman coding tree and code table
    TreeNode* HuffmanTree = build_huffman_tree(freq_table);
    std::map<char,std::string>* Codes = new std::map<char,std::string>;
    gen_huffman_codes(Codes, HuffmanTree);


    // Encode file
    std::vector<char>* encoded = encode(Codes, buffer, data_size+1);

    std::cout << "Writing to disk..." << std::endl;

    // Write compressed data to file
    std::ofstream ofs("compr_huffman.dat", std::ios::out | std::ios::binary);
    ofs.write(encoded->data(), encoded->size());
    ofs.flush();

    // Open compressed file
    ifs.open("compr_huffman.dat",
            std::ios::in | std::ios::binary | std::ios::ate);
    if(!ifs) {
        std::cerr << "error opening file" << std::endl;
        return -1;
    }

    // Determine length
    int data_size2 = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    // Read data
    char* buffer2 = new char[data_size2];
    ifs.read(buffer2, data_size2);
    ifs.close();

    std::cout << "Decompressing file..." << std::endl; 

    // Decode file
    std::vector<char>* decoded = decode(HuffmanTree, buffer2, data_size2);
 
    std::cout << "Testing files..." << std::endl;

    // Check for inconsistencies
    bool matching = true;
    for(int i=0; i<data_size; i++) {
        if(buffer[i] != (*decoded)[i]) {
            matching = false;
            break;
        }
    }

    // Display results
    if(!matching) {
        std::cerr << "error encoding data... files do not match!" << std::endl;
    } else {
        std::cout << "Compression success! files match 100%" << std::endl;
        std::cout << "======================================\n";
        std::cout << std::left << std::setw(22)
                << "Original file size: " << "| " << data_size << "B\n";
        std::cout << std::left << std::setw(22)
                << "Compressed size: " << "| " << data_size2 << "B\n";
        std::cout << std::left << std::setw(22)
                << "Reduction: " << "| " << std::fixed << std::setprecision(2)
                << 100 - ((double)data_size2 / data_size) * 100 << "%"
                << std::endl;
    }

    // Write decoded file to disk
    ofs.close();
    ofs.open("orig_huffman.txt",
            std::ios::out | std::ios::binary);
    ofs.write(decoded->data(), decoded->size());
    ofs.flush();
    ofs.close();

    // Clean up
    delete[] buffer;
    delete[] buffer2;
    delete freq_table;
    delete Codes;
    delete encoded;
    delete decoded;

    std::stack<TreeNode*> S;
    S.push(HuffmanTree);
    while(!S.empty()) {
        TreeNode* cur = S.top();
        S.pop();

        if(cur->left) S.push(cur->left);
        if(cur->right) S.push(cur->right);

        delete cur;
    }

    return 0;
}
