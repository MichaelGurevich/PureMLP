#include "Matrix.h"
#include "iostream"
#include "utils.h"
#include <iomanip>


Matrix::Matrix(int rows, int cols, bool initRandom) : rows(rows), columns(cols)
{
	matrix = new double*[rows];
	if (!initRandom)
		for (int i = 0; i < rows; ++i)
			matrix[i] = new double[columns] {0};
	else
	{
		int cnt = 0;
		for (int i = 0; i < rows; ++i)
		{
			matrix[i] = new double[columns];
			for (int j = 0; j < columns; ++j)	
				matrix[i][j] = getRandomNumber(-1, 1);						
		}	
	}
}

void Matrix::transpose()
{
	double** matT = new double*[columns];
	
	for (int i = 0; i < columns; ++i) // rows of transpose
	{
		matT[i] = new double[rows];

		for (int j = 0; j < rows; ++j) // columns of transpose
			matT[i][j] = matrix[j][i];
	}

	// free prev matrix
	for (int i = 0; i < rows; ++i)
		delete[]matrix[i];
	delete matrix;
	
	swap(&rows, &columns);
	matrix = matT;
}

Matrix::~Matrix()
{
	for (int i = 0; i < rows; ++i)
		delete[]matrix[i];
	delete matrix;
}

void Matrix::printMat()
{
	std::cout << std::fixed << std::setprecision(1);
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < columns; ++j)
		{
			if (matrix[i][j] >= 0)
				std::cout << " ";
			std::cout << matrix[i][j] << " ";
		}
		std::cout << std::endl;
	}
}
