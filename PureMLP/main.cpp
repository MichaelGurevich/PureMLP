
#include "utils.h"
#include "DataLoader.h"
#include "MLP.h"


int main() {
    initSeed();

  
    MLP model (784, 10, 10);

    DataLoader dl("mnist_data.txt", "mnist_labels.txt");

    std::vector<pair<Matrix, Matrix>> splits = dl.trainValidTestSplit(10000, 2000, 100);


    model.fit(splits[0].first, splits[0].second, 30);
    
    std::cout << "\n\n";
    
    
   
    return 0;
}
