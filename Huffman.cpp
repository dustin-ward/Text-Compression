#include <bits/stdc++.h>

struct TreeNode {
    char c;
    int freq;

    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    TreeNode(char c, int f) : c(c), freq(f) {};
};

struct CompareTreeNode {
    bool operator()(TreeNode* a, TreeNode* b) {
        return a->freq >= b->freq;
    }
};

TreeNode* build_huffman_tree(std::map<char,int>* freq) {
    // MinHeap built on frequency
    std::priority_queue<TreeNode*, std::vector<TreeNode*>, CompareTreeNode> PQ;

    // Initialize leaf node for each char
    for(auto &[c,f] : *freq)
        PQ.push(new TreeNode(c,f));

    // Tree merging step
    while((int)PQ.size() > 1) {
        TreeNode* left = PQ.top();
        PQ.pop();

        TreeNode* right = PQ.top();
        PQ.pop();

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

void printTree(TreeNode* root, std::string code="") {
    if(!root) return;

    if(root->c != 0) {
        if(root->c == '\n')
            std::cout << "\\n";
        else if(root->c == '\t')
            std::cout << "\\t";
        else
            std::cout << root->c;
        std::cout << " : " << code << std::endl;
    }

    printTree(root->left, code + '0');
    printTree(root->right, code + '1');
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
    char* buffer = new char[data_size];
    ifs.read(buffer, data_size);
    ifs.close();


    // Generate frequency table
    std::map<char,int>* freq_table;
    freq_table = gen_freq_table(buffer, data_size);

    // Create the huffman coding tree
    TreeNode* HuffmanTree = build_huffman_tree(freq_table);

    std::cout<<"CODES=============="<<std::endl;
    printTree(HuffmanTree);   

    delete[] buffer;
    delete freq_table;
    return 0;
}
