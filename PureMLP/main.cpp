
#include "utils.h"
#include "DataLoader.h"


int main() {
    initSeed();

    DataLoader dl("mnist_data.txt", "mnist_labels.txt");

    //dl.shuffleData();

    //Matrix oneHotLabels = DataLoader::labelsToOneHot(dl.getLabels());


    std::vector<pair<Matrix, Matrix>> splits = dl.trainValidTestSplit(100, 100, 100);

    splits[0].first.printSize();
    
   
    return 0;
}
