#include "DataLoader.h"
#include <fstream>
#include <iostream>
#include "utils.h"


DataLoader::DataLoader(string dataFileName, string labelsFileName)
{
    std::ifstream labelsFile(labelsFileName);

    if (labelsFile.is_open())
        labelsFile >> labels;
    labelsFile.close();

   // std::cout << labels[1200][0];

    std::ifstream dataFile(dataFileName);

    if (dataFile.is_open())
        dataFile >> data;
    dataFile.close();

}

Matrix DataLoader::labelsToOneHot(const Matrix& labels)
{
    int rows;

    rows = labels.getRows();

    double** oneHotMat = new double* [rows];

    double** labelsMat = labels.getMatrix();

    for (int i = 0; i < rows; ++i)
    {
        oneHotMat[i] = new double[10] {};
        
        int label = labelsMat[i][0];

        oneHotMat[i][label] = 1;
    }

    return Matrix(rows, 10, oneHotMat);
}

void DataLoader::shuffleData()
{

    double** labelsMat = labels.getMatrix();
    double** dataMat = data.getMatrix();

    int rows = labels.getRows();
    int cols = data.getColumns();

    std::vector<int> indicies(rows);

    for (int i = 0; i < rows; ++i)
        indicies[i] = i;

    std::shuffle(indicies.begin(), indicies.end(), global_rng);


    double** shuffledData, ** shuffledLabels;

    shuffledData = new double* [rows];
    shuffledLabels = new double* [rows];

    int index;

    
    for (int i = 0; i < rows; ++i)
    {
        shuffledData[i] = new double[cols];
        shuffledLabels[i] = new double[1];

        index = indicies[i];

        std::memcpy(shuffledData[i], dataMat[index], cols * sizeof(double));
        shuffledLabels[i][0] = labelsMat[index][0];
    }

    labels.setMatrix(shuffledLabels, rows, 1);
    data.setMatrix(shuffledData, rows, cols);

}



void DataLoader::shuffleData(Matrix& X, Matrix& y)
{
    double** labelsMat = y.getMatrix();
    double** dataMat = X.getMatrix();

    int rows = y.getRows();
    int cols = X.getColumns();

    std::vector<int> indicies(rows);

    for (int i = 0; i < rows; ++i)
        indicies[i] = i;

    std::shuffle(indicies.begin(), indicies.end(), global_rng);


    double** shuffledData, ** shuffledLabels;

    shuffledData = new double* [rows];
    shuffledLabels = new double* [rows];

    int index;


    for (int i = 0; i < rows; ++i)
    {
        shuffledData[i] = new double[cols];
        shuffledLabels[i] = new double[1];

        index = indicies[i];

        std::memcpy(shuffledData[i], dataMat[index], cols * sizeof(double));
        shuffledLabels[i][0] = labelsMat[index][0];
    }

    y.setMatrix(shuffledLabels, rows, 1);
    X.setMatrix(shuffledData, rows, cols);
}


vector<pair<Matrix, Matrix>> DataLoader::trainValidTestSplit(unsigned int trainSize, unsigned int validSize, unsigned int testSize)
{
    int totalSize = trainSize + validSize + testSize;
    

  /*  if (totalSize > labels.getRows())
    {
        // TODO: Handle exception
        throw - 1;
    }*/
 
    shuffleData();

    int randomStartRow = getRandomNumber(0, labels.getRows() - totalSize - 1);

    pair<Matrix, Matrix> train;
    pair<Matrix, Matrix> valid;
    pair<Matrix, Matrix> test;
    vector<pair<Matrix, Matrix>> splits = { train, valid, test };
    vector<unsigned int> sizes = { trainSize, validSize, testSize };
    int start = randomStartRow;
    int end = start;

    for (int i = 0; i < 3; ++i)
    {
        start = end;
        end += sizes.at(i);
        splits.at(i).first = Matrix::slice(start, end, data);
        splits.at(i).second = Matrix::slice(start, end, labels);
        
    }
    
    return splits;
}

/*
Splits training data into smaller batches to avoid vanishing gradients
Receives:
    unsigned int batchSize - the size of each mini-batch
    const ref to pair of data and its corresponding labels

Returns:
    vector of pairs data and labels each size of batchSize
*/
vector<pair<Matrix, Matrix>> DataLoader::miniBatchGenerator(unsigned int batchSize, const pair<Matrix, Matrix>& data)
{



    Matrix X = data.first;
    Matrix y = data.second;

    shuffleData(X, y);

    vector<pair<Matrix, Matrix>> stream;

    unsigned int dataSize, miniBatchAmount, featuresSize;

    miniBatchAmount = X.getRows() / batchSize; // how many minibatches can be created
    featuresSize = X.getColumns(); // data feature size

    int batchIndex = 0;
    int rowIndex = 0;

    double** dataMatrix, ** labelsMatrix;
    dataMatrix = X.getMatrix();
    labelsMatrix = y.getMatrix();

    


    for (int i = 0; i < miniBatchAmount; ++i)
    {

        // create new 2D double array for the mini-batches
        double** miniBatchData = new double* [batchSize];
        double** miniBatchLabels = new double* [batchSize];

        for (int j = batchIndex; j < batchIndex + batchSize; ++j)
        {
            miniBatchData[rowIndex] = new double[featuresSize];
            miniBatchLabels[rowIndex] = new double[1];

            std::memcpy(miniBatchData[rowIndex], dataMatrix[j], featuresSize * sizeof(double));
            miniBatchLabels[rowIndex][0] = labelsMatrix[j][0];
            rowIndex++;
        }
        batchIndex += batchSize;
        rowIndex = 0;
        stream.emplace_back( // create and push mini-batch into the stream
            Matrix(batchSize, featuresSize, miniBatchData),
            Matrix(batchSize, 1, miniBatchLabels)
        );
    }

    /* TODO: account for situation where there is a remainder
    i.e columns 1000 batch size 400 - 200 data examples remained */

    return stream;
}