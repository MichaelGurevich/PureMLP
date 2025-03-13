#include "MLP.h"
#include "DataLoader.h"
#include "utils.h"

/*
c'tor
initialized weights and bais to random numbers according to the parmeters recived
*/
MLP::MLP(int numFeatures, int numHidden, int numClasses, bool filesInit)
	: numFeatures(numFeatures), numHidden(numHidden), numClasses(numClasses) 
{
	
	if (!filesInit)
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
	else
	{
		this->initFromFile();
	}
	
}


std::array<Matrix, 2> MLP::forward(const Matrix& X) const
{

	// X [ num examples x 784 ]
	// W_H [ Num hidden x 784 ]
	// X * W_H [num Examples x num hidden ] 
	// b_h [1 x num hidden]


	Matrix z_h = X * Matrix::transpose(w_h) + b_h;
	//Matrix a_h = Matrix::applyFunc(z_h, sigmoid);
	Matrix a_h = Matrix::applyFunc(z_h, [](double x) {return x > 0 ? x : 0; });


	Matrix z_o = a_h * Matrix::transpose(w_o) + b_o;
	Matrix a_o = z_o;
	//Matrix a_o = Matrix::applyFunc(z_o, sigmoid);



	for (int i = 0; i < z_o.getRows(); ++i)
	{
		double denominator = 0;
		for (int j = 0; j < z_o.getColumns(); ++j)
		{
			denominator += std::pow(e, z_o[i][j]);
		}

		for (int k = 0; k < z_o.getColumns(); ++k)
		{
			a_o.getMatrix()[i][k] = std::pow(e, z_o[i][k]) / denominator;
		}
	}
	

	return { a_h, a_o };
}




std::array<Matrix, 4> MLP::backward(const Matrix& X, const Matrix& y, const Matrix& a_h, const Matrix& a_o)
{



	//double acc = calcAcc(y, predictedLabels(a_o));
	//std::cout << "Acc: " << acc << "%" << std::endl;

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

	//Matrix d_a_h__d_z_h = a_h & (1 - a_h); // [ n examples x n hidden ] -
	Matrix d_a_h__d_z_h = Matrix::applyFunc(a_h, [](double x) {return x > 0 ? 1 : 0; }); // ReLU derivative
	 
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

void MLP::fit(Matrix& X, const Matrix& y,
	 Matrix& validX, const Matrix& validY,  Matrix& testX, const Matrix& testY, int numEpochs, double learningRate)
{

	X = ((X / 255) - 0.5) * 2; // Normalize data to [-1, 1]
	testX = ((testX / 255) - 0.5) * 2; // Normalize data to [-1, 1]
	validX = ((validX / 255) - 0.5) * 2; // Normalize data to [-1, 1]
	
	double logMSE, logTrainAcc, logValidAcc;
	logMSE = logTrainAcc = logValidAcc = -1;

	for (int i = 0; i < numEpochs; ++i)
	{
	
		std::vector<pair<Matrix, Matrix>> stream = DataLoader::miniBatchGenerator(64, std::make_pair(X, y));

		for (pair<Matrix, Matrix>& trainPair : stream)
		{

			auto [a_h, a_o] = forward(trainPair.first); 

			//std::array<Matrix, 2> forwardOuput = forward(trainPair.first);
		

			auto [d_loss__d_w_o, d_loss__d_b_o, d_loss__d_w_h, d_loss__d_b_h] = 
				backward(trainPair.first, trainPair.second, a_h, a_o);


			w_h = w_h - learningRate * d_loss__d_w_h;
			b_h = b_h - learningRate * d_loss__d_b_h;
			w_o = w_o - learningRate * d_loss__d_w_o;
			b_o = b_o - learningRate * d_loss__d_b_o;

			
		}
		
		// Epoch logging
		auto [trainMse, trainAcc] = computeMseAndAcc(*this, X, y, 64);
		auto [validMse, validAcc] = computeMseAndAcc(*this, validX, validY, 64);
		trainAcc *= 100;
		validAcc *= 100;

		


		cout << "Epoch: " << i + 1
			<< " | Train Mse: " << trainMse
			<< " | Train Acc: " << trainAcc << "%"
			<< " | Valid Acc: " << validAcc << "%" << endl;
		
		logMSE = trainMse;
		logTrainAcc = trainAcc;
		logValidAcc = validAcc;
	}

	

	ofstream MyFile("log.txt");

	// Write to the file
	MyFile << "Train MSE: " << logMSE << " | Train accuracy: " << logTrainAcc << " | Valid accuracy: " << logValidAcc;

	// Close the file
	MyFile.close();

	writeWeightsBiasToFile();
}

double MLP::mseLoss(const Matrix& targets, const Matrix& predicts) 
{
	Matrix oneHotTargets = DataLoader::labelsToOneHot(targets);
	return Matrix::mean(Matrix::applyFunc(oneHotTargets - predicts, [](double a) {return a * a; }));
}

int MLP::correctPredictions(const Matrix& targets, const Matrix& predicts)
{
	int numExamples = predicts.getRows();
	int numClasses = predicts.getColumns();
	int cnt = 0;


	for (int i = 0; i < numExamples; ++i)
	{
		if (targets[i][0] == maxIndex(predicts[i], numClasses))
			++cnt;
	}

	return cnt;
}



std::array<double, 2> MLP::computeMseAndAcc(const MLP& mlp, const Matrix & X, const Matrix & y, int miniBatchSize)
{
	double loss, mse, acc;
	int numExamples, correctPredCnt;
	numExamples = correctPredCnt = 0;
	mse = 0;


	std::vector<pair<Matrix, Matrix>> stream = DataLoader::miniBatchGenerator(miniBatchSize, std::make_pair(X, y));

	for (pair<Matrix, Matrix>& validationPair: stream)
	{
		auto [_, predictions] = mlp.forward(validationPair.first); // get mlps preditions for the given batch

		loss = mseLoss(validationPair.second, predictions);


		correctPredCnt += correctPredictions(validationPair.second, predictions);
		numExamples += validationPair.first.getRows();

		mse += loss;
	}

	mse /= stream.size();
	acc = static_cast<double>(correctPredCnt) / numExamples;


	return { mse, acc };

}


void MLP::writeWeightsBiasToFile()
{
	ofstream files[4];

	std::array<const Matrix*, 4> matrices = { &w_h, &b_h, &w_o, &b_o };

	std::array<std::string, 4> filenames = {
		"hidden_weights.weights",
		"hidden_bias.bias",
		"output_weights.weights",
		"output_bias.bias",
	};
	
	for (int i = 0; i < 4; ++i)
	{
		files[i].open(filenames[i]);

		if (!files[i].is_open())
			throw - 1;

		files[i] << *matrices[i];

		files[i].close();
	}
}

void MLP::initFromFile()
{
	std::ifstream files[4];

	std::array<Matrix*, 4> matrices = { &w_h, &b_h, &w_o, &b_o };

	std::array<std::string, 4> filenames = {
		"hidden_weights.weights",
		"hidden_bias.bias",
		"output_weights.weights",
		"output_bias.bias",
	};

	for (int i = 0; i < 4; ++i)
	{
		files[i].open(filenames[i]);

		if (!files[i].is_open())
			throw - 1;

		files[i] >> *matrices[i];

		files[i].close();
	}
}


int MLP::predict(std::vector<int> examp) const
{
	Matrix input(examp);

	input = ((input / 255) - 0.5) * 2;

	auto [_, a_o] = forward(input);
	
	return maxIndex(a_o[0], 10);
}


double MLP::softmax(double x, double* z_o_i, unsigned int size)
{
	double denominator = 0;

	for (int i = 0; i < size; ++i)
		denominator += std::pow(e, z_o_i[i]);

	return std::pow(e, x) / denominator;
}