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
Copy c'tor
Receives:
	const ref to another matrix

Performs a deep copy of the recieved matrix and assigns it to the object 
on which it was called on
*/
Matrix::Matrix(const Matrix& mat) : rows(mat.rows), columns(mat.columns)
{
	
	double** copiedMat = new double*[this->rows];

	// perform a deep copy of the received matrix
	for (int i = 0; i < this->rows; ++i)
	{
		copiedMat[i] = new double[this->columns];
		for (int j = 0; j < this->columns; ++j)
			copiedMat[i][j] = mat[i][j];
	}

	this->matrix = copiedMat;
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

Matrix Matrix::transpose(const Matrix& mat)
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

	return Matrix(matTrows, matTcolumns, matT);
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
	else
		throw -1;
	/*
	TODO:
		handle exeption
	*/
}


Matrix operator+(const Matrix& mat1, const Matrix& mat2)
{
	double** outputMat = nullptr;
	int newRows, newCols;
	//Matrixes are the same size
	if (mat1.rows == mat2.rows and mat1.columns == mat2.columns)
	{
		newRows = mat1.rows;
		newCols = mat1.columns;

		outputMat = new double*[newRows];
		for (int i = 0; i < newRows; ++i)
		{
			outputMat[i] = new double[newCols];
			for (int j = 0; j < newCols; ++j)
				outputMat[i][j] = mat1[i][j] + mat2[i][j];
		}
	}

	else if ((mat1.rows == 1 or mat2.rows == 1) and mat1.columns == mat2.columns) // matrix and row vector
	{

		int rows = mat1.rows !=  1 ? mat1.rows : mat2.rows;
		int columns = mat1.columns;

		const double* rowVector = mat1.rows == 1 ? mat1[0] : mat2[0];
		double** matrix = mat1.rows == 1 ? mat2.getMatrix() : mat1.getMatrix();

		outputMat = new double* [rows];
		for (int i = 0; i < rows; ++i)
		{
			outputMat[i] = new double[columns];
			for (int j = 0; j < columns; ++j)
				outputMat[i][j] = matrix[i][j] + rowVector[j];
		}
			
	}
	else if ((mat1.columns == 1 or mat2.columns == 1) and mat1.rows == mat2.rows) // matrix and column vector
	{

		int columns = mat1.columns == 1 ? mat2.columns : mat1.columns;
		int rows = mat1.rows;

		const double** columnVector = mat1.columns == 1 ? mat1.getMatrix() : mat2.getMatrix();
		double** matrix = mat1.columns == 1 ? mat2.getMatrix() : mat1.getMatrix();

		outputMat = new double* [rows];
		for (int i = 0; i < rows; ++i)
		{
			outputMat[i] = new double[columns];
			for (int j = 0; j < columns; ++j)
				outputMat[i][j] = matrix[i][j] + columnVector[j][0];
		}

	}
	else {
		// TODO:
			// throw and exception
		throw - 1;
		}

	return Matrix(newRows, newCols, outputMat);
}


Matrix& Matrix::operator=(const Matrix& mat)
{
	if (this == &mat)
		return *this;

	freeMatrix(*this);
	this->rows = mat.rows;
	this->columns = mat.columns;

	double** copiedMat = new double* [this->rows];

	// perform a deep copy of the received matrix
	for (int i = 0; i < this->rows; ++i)
	{
		copiedMat[i] = new double[this->columns];
		for (int j = 0; j < this->columns; ++j)
			copiedMat[i][j] = mat[i][j];
	}

	this->matrix = copiedMat;

	return *this;
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