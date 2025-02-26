
#include "utils.h"
#include "DataLoader.h"
#include "MLP.h"


int main() {
    initSeed();

    DataLoader dl("mnist_data.txt", "mnist_labels.txt");

    //dl.shuffleData();

    //Matrix oneHotLabels = DataLoader::labelsToOneHot(dl.getLabels());

    MLP model(28 * 28, 10, 10);
    
    std::vector<pair<Matrix, Matrix>> splits = dl.trainValidTestSplit(200, 20, 20);

    std::vector<pair<Matrix, Matrix>> stream = DataLoader::miniBatchGenerator(100, splits[0]);

    auto thing =  model.forward(stream[0].first);
    auto otherthing = model.backward(stream[0].first, stream[0].second, thing[0], thing[1]);
    
    std::cout << "\n\n";
    
    
   
    return 0;
}
