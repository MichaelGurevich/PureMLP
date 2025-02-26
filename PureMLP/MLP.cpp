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
	
	
	//  output layer 

	Matrix d_z_o__d_w_o = a_h;
	Matrix d_a_o__d_z_o = a_o & (1 - a_o);
	Matrix d_loss__d_a_o = 2 * (a_o - y_onehot) / y_onehot.getColumns();


	Matrix delta_out = d_a_o__d_z_o & d_loss__d_a_o; // [ n examples x n output ]


	Matrix d_loss__d_w_o = Matrix::transpose(delta_out) * d_z_o__d_w_o;

	Matrix d_loss__d_b_o = Matrix::reduce(delta_out, 0, '+');

	


	// hidden layer
	// d_loss__d_w_h = d_z_h__d_w_h * d_a_h__d_z_h * d_z_o__d_a_o * delata out 
	
	Matrix d_z_h__d_w_h = X; // [ n examples x features ]

	Matrix d_a_h__d_z_h = a_h & (1 - a_h); // [ n examples x n hidden ] -
	 
	Matrix d_z_o__d_a_h = w_o; // [ n output x n hidden ] - 

	Matrix d_loss__d_a_h = delta_out * d_z_o__d_a_h; // [ n examples x n hidden ] -



	Matrix d_loss__d_w_h = Matrix::transpose((d_loss__d_a_h & d_a_h__d_z_h)) * d_z_h__d_w_h;
	Matrix d_loss__d_b_h = Matrix::reduce(d_loss__d_a_h & d_a_h__d_z_h, 0, '+');
	
	return {
		d_loss__d_w_o,
		d_loss__d_b_o,
		d_loss__d_w_h,
		d_loss__d_b_h
	};

	
}

void MLP::fit(const Matrix & X, const Matrix & y, int numEpochs, int learningRate)
{

	

	

	for (int i = 0; i < numEpochs; ++i)
	{
		
		std::cout << "epoch no. " << i + 1 << std::endl;
		std::vector<pair<Matrix, Matrix>> stream = DataLoader::miniBatchGenerator(100, std::make_pair(X, y));

		int cnt = 1;
		for (pair<Matrix, Matrix>& trainPair : stream)
		{

			
			
			std::array<Matrix, 2> forwardOuput = forward(trainPair.first);

			std::array<Matrix, 4> backwardOutput = backward(trainPair.first, trainPair.second, forwardOuput[1], forwardOuput[0]);

			w_h -= learningRate * backwardOutput[2];
			b_h -= learningRate * backwardOutput[3];
			w_o -= learningRate * backwardOutput[0];
			b_o -= learningRate * backwardOutput[1];

		}


	}
}