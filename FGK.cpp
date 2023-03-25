// Dynamic/Adaptive Huffman Encoding (FGK Algorithm)
// Author: Dustin Ward
// Date: March 24th, 2023
//
// This is an implementation of dynamic huffman encoding for my CPSC4660
// (Database Managment Systems) class. The goal of this project is to
// demonstrate the performance of various text compression methods.
//
// The dynamic version of Huffman encoding removes the need to perform an
// initial scan to generate the frequency table and Huffman tree. We instead
// create and update the Huffman tree as we encode the source file. This means
// we no longer have to encode the entire tree along with the file, as it can
// be generated during decoding process.
//
// This file (when supplied with a source file as the first argument) will
// encode it using this dynamic huffman algorithm, and write it to file. The 
// file has the name "compr_fgk.dat". Then the file will be decoded and written
// to disk again as "orig_fgk.dat". We can diff the original source file with
// "orig_huffman.dat" to ensure the process has not lost any data.

#include <bits/stdc++.h>

// Representation of a node in the Huffman tree
struct TreeNode {
    char c;
    int freq;
    int order;

    TreeNode* left = nullptr;
    TreeNode* right = nullptr;
    TreeNode* parent = nullptr;
    
    bool zeroNode;
    bool rootNode;
    bool leafNode = 1;

    TreeNode(char c, int f, int order, TreeNode* parent, bool zero, bool root)
        : c(c), freq(f), order(order), parent(parent), zeroNode(zero), rootNode(root) {};
};

const char END_TEXT = -1;

// Start from node in the tree and follow path to root. Reversing this order
// gives us the huffman code for the given symbol.
std::string genCode(TreeNode* node) {
    std::string code = "";
    while(!node->rootNode) {
        code += (node->parent->right == node) ? "1" : "0";
        node = node->parent;
    }
    std::reverse(code.begin(), code.end());
    return code;
}

// Check throughout the tree to ensure that the sibling property is maintained.
// If we determine our node is out of order, return the replacement spot for it
TreeNode* new_spot(TreeNode* node, TreeNode* root) {
    TreeNode* temp = node;

    if(root->freq > temp->freq && !root->leafNode) {
        TreeNode* left = new_spot(temp, root->left);
        if(left)
            temp = left;
        TreeNode* right = new_spot(temp, root->right);
        if(right)
            temp = right;
    }
    else if(root->freq == temp->freq && root->order > temp->order)
        temp = root;

    if(temp == node)    // No change needs to be made to the tree
        return nullptr;
    return temp;
}

// Update frequencies from node up the the root of the Huffman tree
void update_freq(TreeNode* node, TreeNode* root) {
    while(!node->rootNode) {
        // Check for sibling property
        TreeNode* replacement = new_spot(node,root);

        // Do we need to make adjustments to the tree?
        if(replacement && node->parent != replacement) {
            // Ensure order is maintained after swapping
            std::swap(node->order, replacement->order);
            
            // Check if siblings
            TreeNode* parent = node->parent;
            if((parent->left == node && parent->right == replacement)
                    || (parent->left == replacement && parent->right == node)) {

                TreeNode* temp = parent->left;
                parent->left = parent->right;
                parent->right = temp;

            } else {
                // Swap pointers
                if(node->parent->left == node)
                    node->parent->left = replacement;
                else
                    node->parent->right = replacement;
                if(replacement->parent->left == replacement)
                    replacement->parent->left = node;
                else
                    replacement->parent->right = node;

                std::swap(node->parent, replacement->parent);
            }

        }

        // Update frequency and move up tree
        node->freq++;
        node = node->parent;
    }
    node->freq++;
}

// Dynamically encode data to binary format
std::vector<char>* encode(char* data, int N) {
    std::vector<char>* output = new std::vector<char>;
    TreeNode* hfTree = new TreeNode(0,0,INT_MAX,nullptr,1,1);

    // Root node is initial zero node
    TreeNode* zeroNode = hfTree;

    // Lookup table to find the node associated with each symbol
    std::map<char,TreeNode*> symbolTable;

    char buffer = 0;
    int counter = 0;
    int dataPos = 0;
    while(dataPos < N) {
        char cur = data[dataPos++];

        // Does the current sybol already exist in the tree?
        if(symbolTable[cur]) {
            // Generate the code for this symbol
            std::string code = genCode(symbolTable[cur]);
    
            // Write code to output
            for(char c:code) {
                buffer |= (c=='1');
                counter++;
                
                if(counter == 8) {
                    output->push_back(buffer);
                    counter = 0;
                    buffer = 0;
                }

                buffer <<= 1;
            }
            
            // Perform any operations on the tree to maintain sibling property
            update_freq(symbolTable[cur], hfTree);
        }
        else {
            // Get code of zero node followed by full symbol
            std::string code = genCode(zeroNode);
            for(int i=7; i>=0; i--)
                code += (cur & (1<<i)) ? "1" : "0";
    
            // Write code to output
            for(char c:code) {
                buffer |= (c=='1');
                counter++;
                
                if(counter == 8) {
                    output->push_back(buffer);
                    counter = 0;
                    buffer = 0;
                }

                buffer <<= 1;
            }

            // Create 2 new leaf nodes from the current zero node.
            // The new zero node will be on the left, while the new symbol node
            // will be on the right.
            // std::cout<<"Splitting zero node"<<std::endl;
            TreeNode* left = new TreeNode(0,0,zeroNode->order - 2,zeroNode,1,0);  
            TreeNode* right = new TreeNode(cur,1,zeroNode->order - 1,zeroNode,0,0);  
            TreeNode* parent = zeroNode;

            // Old zero node converted to internal node
            zeroNode->leafNode = 0;
            zeroNode->zeroNode = 0;
            zeroNode->left = left;
            zeroNode->right = right;

            // Update entry in the sybol table to point to node in tree
            symbolTable[cur] = right;
            zeroNode = left;

            // Perform any operations on the tree to maintain sibling property
            update_freq(parent, hfTree);
        }
    }

    // Flush remaining buffer
    output->push_back(buffer);

    // Deallocate tree
    std::stack<TreeNode*> S;
    S.push(hfTree);
    while(!S.empty()) {
        TreeNode* cur = S.top();
        S.pop();

        if(cur->left) S.push(cur->left);
        if(cur->right) S.push(cur->right);

        delete cur;
    }
    
    return output;
}

// Dynamically decode data from binary format
std::vector<char>* decode(char* data, int N) {
    std::vector<char>* output = new std::vector<char>;
    TreeNode* hfTree = new TreeNode(0,0,INT_MAX,nullptr,1,1);

    // Root node is initial zero node
    TreeNode* zeroNode = hfTree;

    // Lookup table to find the node associated with each symbol
    std::map<char,TreeNode*> symbolTable;

    char buffer = data[0];
    int bitIdx = 7;
    int dataPos = 0;
    while(dataPos < N) {
        // Start from root of Huffman tree and traverse downwards until we 
        // reach a leaf node. We traverse left for every '0' we read in the
        // binary, and right for every '1'.
        TreeNode* cur = hfTree;
        while(!cur->leafNode) {
            if(buffer & (1<<bitIdx--))
                cur = cur->right;
            else
                cur = cur->left;

            // Finished with this byte
            if(bitIdx < 0) {
                bitIdx = 7;
                buffer = data[++dataPos];
            }
        }

        // Write the decoded character to output to buffer. If we ended on the
        // zero node, we read the next byte which will correspond to a new
        // character to be added to the tree.
        char temp = 0;
        if(cur->zeroNode) {
            // Read next 8 bits
            for(int i=0; i<8; i++) {
                if(buffer&(1<<bitIdx--)) temp |= 1;
                if(i<7) temp <<= 1;

                // Finished with this byte
                if(bitIdx < 0) {
                    bitIdx = 7;
                    buffer = data[++dataPos];
                }
            }
            
            // Create new leaf nodes. Same process as encoding
            TreeNode* left = new TreeNode(0,0,zeroNode->order - 2,zeroNode,1,0);  
            TreeNode* right = new TreeNode(temp,1,zeroNode->order - 1,zeroNode,0,0);  
            cur = zeroNode;

            // Convert old zero node into internal node
            zeroNode->leafNode = 0;
            zeroNode->zeroNode = 0;
            zeroNode->left = left;
            zeroNode->right = right;

            // Update symbol table to point to entry in the tree
            symbolTable[temp] = right;
            zeroNode = left;
        }
        else {
            // The character already exists in the tree, so we can just output
            // it to the buffer.
            temp = cur->c;    
        }

        // Check for encoded EOF character
        if(temp == END_TEXT) return output;

        // Write to output buffer and update frequencies in the tree
        output->push_back(temp);
        update_freq(cur,hfTree);
    }

    // Deallocate tree
    std::stack<TreeNode*> S;
    S.push(hfTree);
    while(!S.empty()) {
        TreeNode* cur = S.top();
        S.pop();

        if(cur->left) S.push(cur->left);
        if(cur->right) S.push(cur->right);

        delete cur;
    }

    return output;
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


    // Encode file
    std::vector<char>* encoded = encode(buffer, data_size+1);

    std::cout << "Writing to disk..." << std::endl;

    // Write compressed data to file
    std::ofstream ofs("compr_fgk.dat", std::ios::out | std::ios::binary);
    ofs.write(encoded->data(), encoded->size());
    ofs.flush();

    // Open compressed file
    ifs.open("compr_fgk.dat",
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
    std::vector<char>* decoded = decode(buffer2, data_size2);
 

    std::cout << "Testing files..." << std::endl;

    // Check for inconsistencies
    bool matching = true;
    for(int i=0; i<data_size; i++) {
        if(buffer[i] != (*decoded)[i]) {
            std::cerr << "Mismatching character at pos:" << i << std::endl;
            std::cerr << "Original: " << std::hex << buffer[i] << std::endl;
            std::cerr << "Decoded: " << std::hex << (*decoded)[i] << std::endl;

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
    ofs.open("orig_fgk.txt",
            std::ios::out | std::ios::binary);
    ofs.write(decoded->data(), decoded->size());
    ofs.flush();
    ofs.close();

    // Clean up
    delete[] buffer;
    delete[] buffer2;
    delete encoded;
    delete decoded;

    return 0;
}
