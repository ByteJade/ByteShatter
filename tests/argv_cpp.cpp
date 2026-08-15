#include <iostream>

int main(int argc, char** argv) {
    char* new_arg = new char[256];
    for (int i = 0; i < argc; i++) {
        int argp = 0;
        char* arg = argv[i];
        do {
            new_arg[argp] = arg[argp];
        } while (arg[argp++]);
        std::cout << "arg " << i << ": " << new_arg << std::endl;
    }
    delete[] new_arg;
    return 0;
}