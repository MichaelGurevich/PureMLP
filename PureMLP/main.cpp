#include <iostream>
#include "Matrix.h"
#include "utils.h"
#include <fstream>

int main() {
    initSeed();

    Matrix lables;
    Matrix data;

    std::ifstream file1("mnist_labels.txt");

    if (file1.is_open())
    {
        file1 >> lables;
    }
    file1.close();

    lables.printSize();
    


    std::ifstream file2("mnist_data.txt");

    if (file2.is_open())
    {
        file2 >> data;
    }
    file2.close();

    data.printSize();

   
    return 0;
}
