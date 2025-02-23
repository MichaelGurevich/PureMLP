#pragma once
#include <vector>
#include <string>
#include "Matrix.h"


using namespace std;

class DataLoader
{
private:
	Matrix data;
	Matrix labels;


	
public:
	DataLoader(string dataFileName, string labelsFileName);
	static Matrix labelsToOneHot(const Matrix& labels);
	void shuffleData();

	vector<pair<Matrix, Matrix>> trainValidTestSplit(unsigned int trainSize, unsigned int validSize, unsigned int testSize);
	static vector<pair<Matrix, Matrix>> miniBatchGenerator(unsigned int batchSize, const pair<Matrix, Matrix>& data);

	const Matrix& getLabels() const { return labels; }

	


	
};