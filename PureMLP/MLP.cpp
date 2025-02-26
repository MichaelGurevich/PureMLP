#include "MLP.h"
#include "DataLoader.h"


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


std::array<Matrix, 2> MLP::forward(const Matrix& X) const
{
	Matrix z_h = X * Matrix::transpose(w_h) + b_h;
	Matrix a_h = Matrix::applyFunc(z_h, sigmoid);

	Matrix z_o = a_h * Matrix::transpose(w_o) + b_h;
	Matrix a_o = Matrix::applyFunc(z_o, sigmoid);

	return { a_h, a_o };
}


std::array<Matrix, 4> MLP::backward(const Matrix& X, const Matrix& y, const Matrix& a_h, const Matrix& a_o)
{
	// Encode labels into one hot
	Matrix y_onehot = DataLoader::labelsToOneHot(y);
	
	
	/*
	Output layer weights and bias update
	*/

	Matrix d_z_o__d_w_o = a_h;
	Matrix d_a_o__d_z_o = a_o & (1 - a_o);
	Matrix d_loss__d_a_o = 2 * (a_o - y_onehot) / y_onehot.getColumns();


	Matrix delta_out = d_a_o__d_z_o & d_loss__d_a_o; // num examples x  num hidden -> num examples x 10

	Matrix d_loss__d_w_o = Matrix::transpose(delta_out) * d_z_o__d_w_o;

	Matrix d_loss__d_b_o = Matrix::reduce(delta_out, 0, '+');

	return {
		d_z_o__d_w_o,
		d_z_o__d_w_o,
		d_z_o__d_w_o,
		d_z_o__d_w_o
	};


	/*
	
	*/


	
}



// num examples x num features * num features x num hidden 
// num examples x num hidden