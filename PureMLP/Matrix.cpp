#include "Matrix.h"
#include "iostream"
#include "utils.h"
#include <iomanip>

/*
C'tor
Receives: nubmer of rows, number of columns and a flag initRandom
unsigned int rows - defines the number of rows of the matrix 
unsigned int columns - defined the number of columds of the matrix
bool initRandom:
	true - init the matrix with random number in range (-1, 1)
	false - init the matrix with zeros
	default - false
*/
Matrix::Matrix(unsigned int rows, unsigned int cols, bool initRandom) : rows(rows), columns(cols)
{
	matrix = new double*[rows]; // Create new matrix size of rows x columns

	if (initRandom)  // if init random is true
	{
		for (int i = 0; i < rows; ++i)
		{
			matrix[i] = new double[columns]; // create columns for each rows
			for (int j = 0; j < columns; ++j)
				matrix[i][j] = getRandomNumber(-1, 1); // get random number for each entry in the matrix
		}
	}
	else // otherwise, init the matrix with random numbers
	{
		for (int i = 0; i < rows; ++i)
			matrix[i] = new double[columns] {0};
	}
}

/*
Transposes the matrix
Creates new matrix size of columns x rows 
Changes directly the matrix attribute of the object on which is called on
Frees the previous matrix
*/
void Matrix::transpose()
{
	double** matT = new double*[columns];
	
	for (int i = 0; i < columns; ++i) // rows of transpose
	{
		matT[i] = new double[rows];

		for (int j = 0; j < rows; ++j) // columns of transpose
			matT[i][j] = matrix[j][i];
	}

	freeMatrix(*this);
		
	swap(&rows, &columns); // swap rows and columns to accomedate the new size
	matrix = matT;
}

double** Matrix::transpose(const Matrix& mat)
{
	int matTrows, matTcolumns;
	matTrows = mat.getColumns();
	matTcolumns = mat.getRows();

	double** matT = new double* [matTrows];
	for (int i = 0; i < matTrows; ++i)
	{
		matT[i] = new double[matTcolumns];
		for (int j = 0; j < matTcolumns; ++j) // columns of transpose
			matT[i][j] = mat[j][i];
	}

	return matT;
}


// ==================== Operators ====================

void Matrix::operator+(const Matrix& matToAdd)
{

	if (matToAdd.getRows() == rows and matToAdd.getColumns() == columns) // if matrixes are of the same size
	{
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < columns; ++j)
				matrix[i][j] += matToAdd[i][j];
	}
	else if (matToAdd.getRows() == rows and matToAdd.getColumns() == 1) // if the matrix is a column vector matrix, add the vector to each column
	{
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < columns; ++j)
				matrix[i][j] += matToAdd[i][0];
	}
	else if (matToAdd.getColumns() == columns and matToAdd.getRows() == 1) // if the matrix a row vector matrix, add the vector to each row
	{
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < columns; ++j)
				matrix[i][j] += matToAdd[0][j];
	}
	/*
	TODO:
		handle exeption
	*/


}


double** operator+(const Matrix& mat1, const Matrix& mat2)
{
	double** outputMat = nullptr;
	//Matrixes are the same size
	if (mat1.rows == mat2.rows and mat1.columns == mat2.columns)
	{
		outputMat = new double*[mat1.rows];
		for (int i = 0; i < mat1.rows; ++i)
		{
			outputMat[i] = new double[mat1.columns];
			for (int j = 0; j < mat1.columns; ++j)
				outputMat[i][j] = mat1[i][j] + mat2[i][j];
		}
	}
	else if ((mat1.rows == 1 or mat2.rows == 1) and mat1.columns == mat2.columns)
	{
		int rows = mat1.rows !=  1 ? mat1.rows : mat2.rows;
		int columns = mat1.columns;

		const double* columnVector = mat1.rows == 1 ? mat1[0] : mat2[0];
		double** matrix = mat1.rows == 1 ? mat2.getMatrix() : mat1.getMatrix();

		outputMat = new double* [rows];
		for (int i = 0; i < rows; ++i)
		{
			outputMat[i] = new double[columns];
			for (int j = 0; j < columns; ++j)
				outputMat[i][j] = matrix[i][j] + columnVector[j];
		}
			
	}
	
		
	

	return outputMat;
}

// ==================== End of Operators ====================

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


void Matrix::freeMatrix(Matrix& mat)
{
	// free prev matrix
	for (int i = 0; i < mat.getRows(); ++i)
		delete[]mat.getMatrix()[i];
	delete mat.getMatrix();
}