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

const char END_TEXT = 3;

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

    if(temp == node)
        return nullptr;
    return temp;
}

void update_freq(TreeNode* node, TreeNode* root) {
    while(!node->rootNode) {
        TreeNode* replacement = new_spot(node,root);
        
        if(replacement && node->parent != replacement) {
            std::swap(replacement->order, replacement->order);
            
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

        node->freq++;
        node = node->parent;
    }
    node->freq++;
}

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
        // std::cout<< "CUR CHAR: "<<cur<<std::endl;

        if(symbolTable[cur]) {
            // std::cout << "Found in table" << std::endl;
            std::string code = genCode(symbolTable[cur]);
            // std::cout << "Code: " << code << std::endl;
    
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
            
            update_freq(symbolTable[cur], hfTree);
        }
        else {
            // std::cout << "Not Found in table" << std::endl;
            // Get code of zero node followed by full symbol
            std::string code = genCode(zeroNode);
            for(int i=7; i>=0; i--)
                code += (cur & (1<<i)) ? "1" : "0";
            // std::cout << "Code: " << code << std::endl;
    
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

            // Create new leaf node
            TreeNode* left = new TreeNode(0,0,zeroNode->order - 2,zeroNode,1,0);  
            TreeNode* right = new TreeNode(cur,1,zeroNode->order - 1,zeroNode,0,0);  
            TreeNode* parent = zeroNode;

            zeroNode->leafNode = 0;
            zeroNode->zeroNode = 0;
            zeroNode->left = left;
            zeroNode->right = right;

            symbolTable[cur] = right;
            zeroNode = left;

            update_freq(parent, hfTree);
        }
    }

    // Flush remaining buffer
    output->push_back(buffer);

    return output;
}

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
        // std::cout << "BUFFER: " << buffer << std::endl;
        TreeNode* cur = hfTree;
        while(!cur->leafNode) {
            if(buffer & (1<<bitIdx--)) {
                // std::cout<<"traverse right"<<std::endl;
                cur = cur->right;
            }
            else {
                // std::cout<<"traverse left"<<std::endl;
                cur = cur->left;
            }

            if(bitIdx < 0) {
                bitIdx = 7;
                buffer = data[++dataPos];
            }
        }

        char temp = 0;
        if(cur->zeroNode) {
            // std::cout<<"Reached zero node"<<std::endl;
            for(int i=0; i<8; i++) {
                // std::cout<<"i: "<<i<<" bit: "<<((buffer&(1<<bitIdx))?"1 ":"0 ")<<std::bitset<8>((int)temp)<<std::endl;
                if(buffer&(1<<bitIdx--)) temp |= 1;
                if(i<7) temp <<= 1;

                if(bitIdx < 0) {
                    bitIdx = 7;
                    buffer = data[++dataPos];
                    // std::cout<<"Shift. New buffer: "<<buffer<<std::endl;
                }
            }
            // std::cout<<"new char: "<<temp<<"("<<(int)(unsigned char)temp<<")"<<std::endl;
            
            // Create new leaf node
            TreeNode* left = new TreeNode(0,0,zeroNode->order - 2,zeroNode,1,0);  
            TreeNode* right = new TreeNode(temp,1,zeroNode->order - 1,zeroNode,0,0);  
            cur = zeroNode;

            zeroNode->leafNode = 0;
            zeroNode->zeroNode = 0;
            zeroNode->left = left;
            zeroNode->right = right;

            symbolTable[temp] = right;
            zeroNode = left;
        }
        else {
            temp = cur->c;    
        }

        // std::cout << "SYMBOL: " << temp << std::endl;
        if(temp == END_TEXT) return output;
        output->push_back(temp);
        update_freq(cur,hfTree);
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

    // for(char c:*encoded)
    //     std::cout << std::bitset<8>((int)c) << " ";
    // std::cout<<std::endl;

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
 
    // for(char c:*decoded)
    //     std::cout << std::bitset<8>((int)c) << " ";
    // std::cout<<std::endl;
    
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

    // std::stack<TreeNode*> S;
    // S.push(HuffmanTree);
    // while(!S.empty()) {
    //     TreeNode* cur = S.top();
    //     S.pop();
    //
    //     if(cur->left) S.push(cur->left);
    //     if(cur->right) S.push(cur->right);
    //
    //     delete cur;
    // }

    return 0;
}
