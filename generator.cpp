// Random Byte Generator
// Author: Dustin Ward
// Date: March 24th, 2023
//
// This is a random data generator for my CPSC4660 (Database Management
// systems) class. It generates files of pre-defined sizes consisting of random
// information. These files were used to test various compression algorithms.

#include <bits/stdc++.h>
using namespace std;

const string filepath = "./testing_data/random";

vector<int> sizes = {100, 1000, 10000, 100000};

int main() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,254);

    for(int i:sizes) {
        string new_filepath = filepath + to_string(i) + ".txt";

        char* buffer = new char[i];
        for(int j=0; j<i; j++)
            buffer[j] = dist(rng);

        ofstream ofs(new_filepath, ios::out | ios::binary);
        ofs.write(buffer, i);
    }
}
