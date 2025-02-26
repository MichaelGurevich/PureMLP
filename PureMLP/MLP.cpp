#include "MLP.h"


/*
c'tor
initialized weights and bais to random numbers according to the parmeters recived
*/
MLP::MLP(int numFeatures, int numHidden, int numClasses)
	: numFeatures(numFeatures), numHidden(numHidden), numClasses(numClasses) 
{
	
	/*
	Hidden layer - initialize weight and bais
		w_h - numHiddem x numFeatures
		b_h - 1 x numHidden
	*/
	w_h = Matrix(numHidden, numFeatures, true);
	b_h = Matrix(1, numHidden, false);


	w_o = Matrix(numClasses, numHidden, true);
	b_o = Matrix(1, numClasses, false);
}


std::array<Matrix, 2> MLP::forward(const Matrix& X)
{
	Matrix z_h = X * Matrix::transpose(w_h) + b_h;
	Matrix a_h = Matrix::applyFunc(z_h, sigmoid);

	Matrix z_o = a_h * Matrix::transpose(w_o) + b_h;
	Matrix a_o = Matrix::applyFunc(z_o, sigmoid);

	return { a_h, a_o };
}

// num examples x num features * num features x numhidden 
// num examples x num hidden