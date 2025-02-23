
#include "utils.h"
#include "DataLoader.h"


int main() {
    initSeed();

    DataLoader dl("mnist_data.txt", "mnist_labels.txt");

    //dl.shuffleData();

    //Matrix oneHotLabels = DataLoader::labelsToOneHot(dl.getLabels());


    std::vector<pair<Matrix, Matrix>> splits = dl.trainValidTestSplit(2000, 1000, 1000);

    std::vector<pair<Matrix, Matrix>> stream = DataLoader::miniBatchGenerator(500, splits[0]);


    for (int i = 0; i < stream.size(); ++i)
    {
        stream[i].first.printSize();
        std::cout << "\n\n";
    }
    
   
    return 0;
}
