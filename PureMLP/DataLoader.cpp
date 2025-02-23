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
    int index1, index2, high;

    high = labels.getRows() - 1;

    double** labelsMat = labels.getMatrix();
    double** dataMat = data.getMatrix();

    for (int i = 0; i < high+1; ++i)
    {
        index1 = getRandomNumber(0, high);
        index2 = getRandomNumber(0, high);

        swap(labelsMat[index1], labelsMat[index2]);
        swap(dataMat[index1], dataMat[index2]);
    }
}

vector<pair<Matrix, Matrix>> DataLoader::trainValidTestSplit(unsigned int trainSize, unsigned int validSize, unsigned int testSize)
{
    int totalSize = trainSize + validSize + testSize;
    

    if (totalSize > labels.getRows())
    {
        // TODO: Handle exception
        throw - 1;
    }
 
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


vector<pair<Matrix, Matrix>> DataLoader::miniBatchGenerator(unsigned int batchSize, const pair<Matrix, Matrix>& data)
{
    unsigned int dataSize = data.second.getRows();

    unsigned int miniBatchAmnt = dataSize / batchSize;

    unsigned int featuresSize = data.first.getColumns();

    

    int batchIndex = 0;
    int rowIndex = 0;

    double** dataMatrix, ** labelsMatrix;

    dataMatrix = data.first.getMatrix();
    labelsMatrix = data.second.getMatrix();

    vector<pair<Matrix, Matrix>> miniBatches;

    for (int i = 0; i < miniBatchAmnt; ++i)
    {
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
        miniBatches.emplace_back(
            Matrix(batchSize, featuresSize, miniBatchData),
            Matrix(batchSize, 1, miniBatchLabels)
        );
    }

    return miniBatches;
}