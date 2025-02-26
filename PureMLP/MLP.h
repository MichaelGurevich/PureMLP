#include "Matrix.h"
#include <array>
#include <numbers>
#include <cmath>

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

public:
	MLP(int numFeatures, int numHidden, int numClasses);

	std::array<Matrix, 2> forward(const Matrix& X) const;
	std::array<Matrix, 4> backward(const Matrix& X, const Matrix& y, const Matrix& a_h, const Matrix& a_o);

	void fit(Matrix& X, const Matrix& y, int numEpochs = 20, int learningRate = 0.1);


};