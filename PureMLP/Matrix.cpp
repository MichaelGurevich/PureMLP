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
	int sum = 0;
	if (initRandom)  // if init random is true
	{
		for (int i = 0; i < rows; ++i)
		{
			matrix[i] = new double[columns]; // create columns for each rows
			for (int j = 0; j < columns; ++j)
			{
				//matrix[i][j] = getRandomNumber(-1, 1); // get random number for each entry in the matrix
				matrix[i][j] = sum++;
			}
				
				
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

Matrix::Matrix(Matrix&& mat) noexcept
	:matrix(nullptr), rows(0), columns(0) // 
{
	// "Steal" attributes of the rvalue mat
	this->matrix = mat.matrix;
	this->rows = mat.rows;
	this->columns = mat.columns;

	// Nullify the rvalue mat
	mat.matrix = nullptr;
	mat.rows = 0;
	mat.columns = 0;
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


	// Operator +
Matrix& Matrix::operator+=(double scalar)
{
	this->applyScalarOperation(scalar, Matrix::addtion);
	return *this;

}


Matrix& Matrix::operator+=(const Matrix& mat)
{
	this->linearOperation(mat, Matrix::addtion);
	return *this;
}


	// Operator -
Matrix& Matrix::operator-=(double scalar)
{
	this->applyScalarOperation(scalar, Matrix::subraction);
	return *this;
}


Matrix& Matrix::operator-=(const Matrix& mat)
{
	this->linearOperation(mat, Matrix::subraction);
	return *this;
}


/*
Operator =

Receives: referece to a matrix
Updates the matrix on which the operator was called on 
Returns a ref to the left hand matrix
*/
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

Matrix& Matrix::operator=(Matrix&& mat) noexcept
{
	if (this == &mat)
		return *this;

	Matrix::freeMatrix(*this);
	
	// "Steal" attributes of the rvalue mat
	this->matrix = mat.matrix;
	this->rows = mat.rows;
	this->columns = mat.columns;

	// Nullify the rvalue mat
	mat.matrix = nullptr;
	mat.rows = 0;
	mat.columns = 0;

	return *this;

}


Matrix operator*(const Matrix& mat1, const Matrix& mat2)
{
	int mat1Rows, mat1Cols, mat2Rows, mat2Cols, matMulRows, matMulCols;
	double** mat1mat, **mat2mat;

	mat1Rows = mat1.getRows();
	mat2Rows = mat2.getRows();
	mat1Cols = mat1.getColumns();
	mat2Cols = mat2.getColumns();

	mat1mat = mat1.getMatrix();
	mat2mat = mat2.getMatrix();

	matMulRows = mat1Rows;
	matMulCols = mat2Cols;


	double** matMul = new double* [matMulRows];

	if (mat1Cols == mat2Rows)
	{
		for (int i = 0; i < mat1Rows; ++i)
		{
			matMul[i] = new double[matMulCols];
			for (int j = 0; j < mat2Cols; ++j)
			{
				double sum = 0;
				for (int k = 0; k < mat2Rows; ++k)
				{
					sum += mat1mat[i][k] * mat2mat[k][j];
				}
				matMul[i][j] = sum;
			}
		}

		return Matrix(matMulRows, matMulCols, matMul);
	} 

	// TODO:
		// Handle exception
	throw - 1;
}

Matrix& Matrix::operator*=(const Matrix& mat)
{	

	double** matMul = new double* [this->rows];

	if (this->columns == mat.rows)
	{
		for (int i = 0; i < this->rows; ++i)
		{
			matMul[i] = new double[mat.columns];
			for (int j = 0; j < mat.columns; ++j)
			{
				double sum = 0;
				for (int k = 0; k < mat.rows; ++k)
				{
					sum += this->matrix[i][k] * mat.matrix[k][j];
				}
				matMul[i][j] = sum;
			}
		}

		freeMatrix(*this); // delete previous matrix

		this->columns = mat.columns;
		this->matrix = matMul; // update matrix to the multiplied matrix
		return *this;
	}

	// TODO:
		// Handle exception
	throw - 1;
}

Matrix& Matrix::operator*=(double scalar)
{
	this->applyScalarOperation(scalar, Matrix::multiplication);
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
			std::cout << std::setw(5) << std::left << matrix[i][j] << " ";
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

Matrix Matrix::applyScalarOperation(const Matrix& mat, double scalar, std::function<double(double, double)> op)
{
	unsigned int matRows, matCols;

	matRows = mat.getRows();
	matCols = mat.getColumns();

	double** newMat = new double* [matRows];
	double** matMat = mat.getMatrix();
	for (int i = 0; i < matRows; ++i)
	{
		newMat[i] = new double[matCols];
		for (int j = 0; j < matCols; ++j)
		{
			newMat[i][j] =  op(matMat[i][j], scalar);
		}
	}

	return Matrix(matRows, matCols, newMat);
}


/*
For use in operaton +/- between to matrices
	Receives: reference to matrices

	Returns: new matrix which is the sum/subtraction

	If matrices are equal in size then adds element wise
	If matrix and vector (n x 1 / 1 x n) is recieved -> broadcasts the vector thru the matrix columns/rows
*/
Matrix Matrix::linearOperation(const Matrix& mat1, const Matrix& mat2, std::function<double(double, double)> op)
{
	double** outputMat = nullptr;
	int newRows, newCols;
	//Matrixes are the same size
	if (mat1.rows == mat2.rows and mat1.columns == mat2.columns)
	{
		newRows = mat1.rows;
		newCols = mat1.columns;

		outputMat = new double* [newRows];
		for (int i = 0; i < newRows; ++i)
		{
			outputMat[i] = new double[newCols];
			for (int j = 0; j < newCols; ++j)
				outputMat[i][j] = op(mat1[i][j], mat2[i][j]);
		}
	}

	else if ((mat1.rows == 1 or mat2.rows == 1) and mat1.columns == mat2.columns) // matrix and row vector -> broadcast the vector 
	{

		int rows = mat1.rows != 1 ? mat1.rows : mat2.rows;
		int columns = mat1.columns;

		const double* rowVector = mat1.rows == 1 ? mat1[0] : mat2[0];
		double** matrix = mat1.rows == 1 ? mat2.getMatrix() : mat1.getMatrix();

		outputMat = new double* [rows];
		for (int i = 0; i < rows; ++i)
		{
			outputMat[i] = new double[columns];
			for (int j = 0; j < columns; ++j)
				outputMat[i][j] = op(matrix[i][j], rowVector[j]);
		}

	}
	else if ((mat1.columns == 1 or mat2.columns == 1) and mat1.rows == mat2.rows) // matrix and column vector -> broadcast the vector
	{

		int columns = mat1.columns == 1 ? mat2.columns : mat1.columns;
		int rows = mat1.rows;

		double** columnVector = mat1.columns == 1 ? mat1.getMatrix() : mat2.getMatrix();
		double** matrix = mat1.columns == 1 ? mat2.getMatrix() : mat1.getMatrix();

		outputMat = new double* [rows];
		for (int i = 0; i < columns; ++i)
		{
			outputMat[i] = new double[columns];
			for (int j = 0; j < rows; ++j)
				outputMat[i][j] = op(matrix[j][i], columnVector[j][0]);
		}

	}
	else {
		// TODO:
			// throw and exception
		throw - 1;
	}

	return Matrix(newRows, newCols, outputMat);
}


void Matrix::linearOperation(const Matrix& mat, std::function<double(double, double)> op)
{
	
	int newRows, newCols;
	
	if (this->rows == mat.rows and this->columns == mat.columns) //Matrixes are the same size
	{
		newRows = this->rows;
		newCols = this->columns;
		for (int i = 0; i < newRows; ++i)
		{
			for (int j = 0; j < newCols; ++j)
				this->matrix[i][j] = op(this->matrix[i][j], mat.matrix[i][j]);
		}
	}

	else if (mat.rows == 1 and this->columns == mat.columns) // matrix and row vector -> broadcast the vector 
	{
		for (int i = 0; i < this->rows; ++i)
		{
			for (int j = 0; j < this->columns; ++j)
				this->matrix[i][j] = op(this->matrix[i][j], mat.matrix[0][j]);
		}

	}
	else if (mat.columns == 1 and this->rows == mat.rows) // matrix and column vector -> broadcast the vector
	{
		for (int i = 0; i < this->columns; ++i)
		{
			for (int j = 0; j < this->rows; ++j)
				this->matrix[i][j] = op(this->matrix[j][i], mat.matrix[j][0]);
		}

	}
	else {
		// TODO:
			// throw and exception
		throw - 1;
	}

}


void Matrix::applyScalarOperation(double scalar, std::function<double(double, double)> op)
{
	for (int i = 0; i < this->rows; ++i)
	{
		for (int j = 0; j < this->columns; ++j)
		{
			this->matrix[i][j] = op(this->matrix[i][j], scalar);
		}
	}
}

void Matrix::applyFunc(std::function<double(double)>& func)
{
	for (int i = 0; i < this->rows; ++i)
	{
		for (int j = 0; j < this->columns; ++j)
			this->matrix[i][j] = func(this->matrix[i][j]);
	}
}