#include "Matrix.h"
#include <array>
#include <numbers>
#include <cmath>
#include <vector>

#include <iostream>
#include <fstream>
#include <algorithm>

class MLP
{
private:
	int numFeatures;
	int numHidden;
	int numClasses;

	int numHidden1;
	int numHidden2;


	Matrix w_h1; // weights hidden
	Matrix b_h1; // bias hidden
	Matrix w_h2;
	Matrix b_h2;
	Matrix w_o; // weights output
	Matrix b_o; // bias output

	static constexpr double e = 2.71828182845904523536;
	static double sigmoid(double x) { return std::pow(e, x) / (1 + std::pow(e,x)); }

	static double ReLU(double x) { return std::max(0.0, x); }
	static Matrix softmax(const Matrix& mat);
	static double calcDenominator(const double* arr, int size);

	

	static double mseLoss(const Matrix& targets, const Matrix& predicts);
	static int correctPredictions(const Matrix& targets, const Matrix& predicts);

	static std::array<double, 2> computeMseAndAcc(const MLP& mlp, const Matrix& X, const Matrix& y, int miniBatchSize = 100);
	

public:
	MLP(int numFeatures, int numHidden, int numClasses, bool filesInit=false);
	MLP(int numFeatures, int numHidden1, int numHidden2, int numClasses, bool filesInit);

	void writeWeightsBiasToFile();
	void initFromFile();

	int predict(std::vector<int> examp) const;


	std::array<Matrix, 3> forward(const Matrix& X) const;
	//std::tuple<Matrix, Matrix> forward(const Matrix& X) const;
	std::array<Matrix, 6> backward(const Matrix& X, const Matrix& y, const Matrix& a_h1, const Matrix& a_h2, const Matrix& a_o);

	void fit(Matrix& X, const Matrix& y,
		 Matrix& validX, const Matrix& validY,  Matrix& testX, const Matrix& testY, int numEpochs=30, double learningRate=0.01);


};