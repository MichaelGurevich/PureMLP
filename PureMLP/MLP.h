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

	Matrix w_h; // weights hidden
	Matrix b_h; // bias hidden
	Matrix w_o; // weights output
	Matrix b_o; // bias output

	static constexpr double e = 2.71828182845904523536;
	static double sigmoid(double x) { return std::pow(e, x) / (1 + std::pow(e,x)); }

	static double ReLU(double x) { return std::max(0.0, x); }
	static double softmax(double x, double* z_o_i, unsigned int size);
	

	static double mseLoss(const Matrix& targets, const Matrix& predicts);
	static int correctPredictions(const Matrix& targets, const Matrix& predicts);

	static std::array<double, 2> computeMseAndAcc(const MLP& mlp, const Matrix& X, const Matrix& y, int miniBatchSize = 100);
	

public:
	MLP(int numFeatures, int numHidden, int numClasses, bool filesInit=false);

	void writeWeightsBiasToFile();
	void initFromFile();

	int predict(std::vector<int> examp) const;


	std::array<Matrix, 2> forward(const Matrix& X) const;
	//std::tuple<Matrix, Matrix> forward(const Matrix& X) const;
	std::array<Matrix, 4> backward(const Matrix& X, const Matrix& y, const Matrix& a_h, const Matrix& a_o);

	void fit(Matrix& X, const Matrix& y,
		 Matrix& validX, const Matrix& validY,  Matrix& testX, const Matrix& testY, int numEpochs=30, double learningRate=0.01);


};