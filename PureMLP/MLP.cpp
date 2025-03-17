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
		w_h1 = Matrix(numHidden, numFeatures, true);
		b_h1 = Matrix(1, numHidden, false);

		/*
		w_h2 = Matrix(numHidden2, numHidden1, true);
		b_h2 = Matrix(numHidden2, numHidden1);

		w_o = Matrix(numClasses, numHidden2, true);
		b_o = Matrix(1, numClasses, false);
		*/



		w_o = Matrix(numClasses, numHidden, true);
		b_o = Matrix(1, numClasses, false);
	}
	else
	{
		this->initFromFile();
	}
	
}


MLP::MLP(int numFeatures, int numHidden1, int numHidden2, int numClasses, bool filesInit)
	: numFeatures(numFeatures), numHidden1(numHidden1), numHidden2(numHidden2), numClasses(numClasses)
{
	if (!filesInit)
	{
		// 1st hidden layer
		w_h1 = Matrix(numHidden1, numFeatures, true);
		b_h1 = Matrix(1, numHidden1, false);

		// 2nd hidden layer
		w_h2 = Matrix(numHidden2, numHidden1, true);
		b_h2 = Matrix(1, numHidden2, false);
		
		// output layer
		w_o = Matrix(numClasses, numHidden2, true);
		b_o = Matrix(1, numClasses, false);

	}
	else
	{
		this->initFromFile();
	}
}


std::array<Matrix, 3> MLP::forward(const Matrix& X) const
{
	// 1st hidden layer
	Matrix z_h1 = X * Matrix::transpose(w_h1) + b_h1;
	Matrix a_h1 = Matrix::applyFunc(z_h1, [](double x) {return x > 0 ? x : 0; });
	
	// 2nd hidden layer
	Matrix z_h2 = a_h1 * Matrix::transpose(w_h2) + b_h2;
	Matrix a_h2 = Matrix::applyFunc(z_h2, [](double x) {return x > 0 ? x : 0; });

	// output layer
	Matrix z_o = a_h2 * Matrix::transpose(w_o) + b_o;
	Matrix a_o = softmax(z_o);	

	return { a_h1, a_h2, a_o };
}




std::array<Matrix, 6> MLP::backward(const Matrix& X, const Matrix& y, const Matrix& a_h1, const Matrix& a_h2, const Matrix& a_o)
{

	// Encode labels into one hot
	Matrix y_onehot = DataLoader::labelsToOneHot(y);
	
	
	//  output layer 

	Matrix d_z_o__d_w_o = a_h2;
	Matrix d_a_o__d_z_o = a_o & (1 - a_o);
	Matrix d_loss__d_a_o = 2 * (a_o - y_onehot) / y_onehot.getColumns();
	Matrix delta_out = d_loss__d_a_o & d_a_o__d_z_o; // [ n examples x n output ]

	Matrix d_loss__d_w_o = Matrix::transpose(delta_out) * d_z_o__d_w_o;
	Matrix d_loss__d_b_o = Matrix::reduce(delta_out, 0, '+');

	

	// hidden 2 layer
	
	Matrix d_z_h2__d_w_h2 = a_h1;
	Matrix d_z_o__d_a_h2 = w_o; 
	Matrix d_a_h2__d_z_h2 = Matrix::applyFunc(a_h2, [](double x) {return x > 0 ? 1 : 0; }); // ReLU derivative
	Matrix d_loss__d_a_h2 = delta_out * d_z_o__d_a_h2;

	Matrix d_loss__d_w_h2 = Matrix::transpose((d_loss__d_a_h2 & d_a_h2__d_z_h2)) * d_z_h2__d_w_h2;
	Matrix d_loss__d_b_h2 = Matrix::reduce(d_loss__d_a_h2 & d_a_h2__d_z_h2, 0, '+');



	// hidden 1 layer
	
	Matrix d_z_h2__d_a_h1 = w_h2;
	Matrix delta_hidden = (delta_out * d_z_o__d_a_h2) & d_a_h2__d_z_h2;
	Matrix d_loss__d_a_h1 = delta_hidden * d_z_h2__d_a_h1;
	Matrix d_a_h1__d_z_h1 = Matrix::applyFunc(a_h1, [](double x) {return x > 0 ? 1 : 0; });
	Matrix d_z_h1__d_w_h1 = X;

	Matrix d_loss__d_w_h1 = Matrix::transpose(d_loss__d_a_h1 & d_a_h1__d_z_h1) * d_z_h1__d_w_h1;
	Matrix d_loss__d_b_h1 = Matrix::reduce(d_loss__d_a_h1 & d_a_h1__d_z_h1, 0, '+');
	
	
	


	return {
		d_loss__d_w_o,
		d_loss__d_b_o,
		d_loss__d_w_h2,
		d_loss__d_b_h2,
		d_loss__d_w_h1,
		d_loss__d_b_h1
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

			auto [a_h1, a_h2, a_o] = forward(trainPair.first); 

			//std::array<Matrix, 2> forwardOuput = forward(trainPair.first);
		

			auto [d_loss__d_w_o, d_loss__d_b_o, d_loss__d_w_h2, d_loss__d_b_h2, d_loss__d_w_h1, d_loss__d_b_h1] =
				backward(trainPair.first, trainPair.second, a_h1, a_h2, a_o);


			w_h1 = w_h1 - learningRate * d_loss__d_w_h1;
			b_h1 = b_h1 - learningRate * d_loss__d_b_h1;
			w_h2 = w_h2 - learningRate * d_loss__d_w_h2;
			b_h2 = b_h2 - learningRate * d_loss__d_b_h2;
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
		auto [_1,_2, predictions] = mlp.forward(validationPair.first); // get mlps preditions for the given batch

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
	ofstream files[6];

	std::array<const Matrix*, 6> matrices = { &w_h1, &b_h1,&w_h2, &b_h2, &w_o, &b_o };

	std::array<std::string, 6> filenames = {
		"hidden1_weights.weights",
		"hidden1_bias.bias",
		"hidden2_weights.weights",
		"hidden2_bias.bias",
		"output_weights.weights",
		"output_bias.bias",
	};
	
	for (int i = 0; i < filenames.size(); ++i)
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
	std::ifstream files[6];

	std::array<Matrix*, 6> matrices = { &w_h1, &b_h1,&w_h2, &b_h2, &w_o, &b_o };

	std::array<std::string, 6> filenames = {
		"hidden1_weights.weights",
		"hidden1_bias.bias",
		"hidden2_weights.weights",
		"hidden2_bias.bias",
		"output_weights.weights",
		"output_bias.bias",
	};

	for (int i = 0; i < filenames.size(); ++i)
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

	auto [_1, _2, a_o] = forward(input);
	

	return maxIndex(a_o[0], 10);
}

double MLP::calcDenominator(const double* arr, int size)
{
	double denominator = 0;

	for (int i = 0; i < size; ++i)
		denominator += std::pow(e, arr[i]);
	
	return denominator;
}


Matrix MLP::softmax(const Matrix& mat)
{
	int rows = mat.getRows();
	int cols = mat.getColumns();
	double** matMatrix = mat.getMatrix();
	
	double denominator;

	double** softmaxMat = new double*[rows];
	for (int i = 0; i < mat.getRows(); ++i)
	{
		softmaxMat[i] = new double[cols];
		denominator = calcDenominator(matMatrix[i], cols);

		for (int j = 0; j < cols; ++j)
			softmaxMat[i][j] = std::pow(e, matMatrix[i][j]) / denominator;
	}

	return Matrix(rows, cols, softmaxMat);
}