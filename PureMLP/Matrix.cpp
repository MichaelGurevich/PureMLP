#include "Matrix.h"
#include "iostream"
#include "utils.h"
#include <iomanip>
#include <string>


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
	if (rows == 0 and cols == 0) // empty c'tor
	{
		matrix = nullptr;
	} 
	else if ( rows == 0 or cols == 0) // illegal input
	{
		throw - 1;
	}
	else
	{
		matrix = new double* [rows]; // Create new matrix size of rows x columns
		int sum = 0;
		if (initRandom)  // if init random is true
		{
			for (int i = 0; i < rows; ++i)
			{
				matrix[i] = new double[columns]; // create columns for each rows
				for (int j = 0; j < columns; ++j)
				{
					
					double bound = std::sqrt(6) / (std::sqrt(rows + cols));

					matrix[i][j] = getRandomNumber(-bound, bound); // get random number for each entry in the matrix
				}


			}
		}
		else // otherwise, init the matrix with random numbers
		{
			for (int i = 0; i < rows; ++i)
				matrix[i] = new double[columns] {0};
		}
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
Matrix operator-(double scalar, const Matrix& mat)
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
			newMat[i][j] = scalar - matMat[i][j];
		}
	}
	//Matrix m(matRows, matCols, newMat);
	//m.printMat();
	return Matrix(matRows, matCols, newMat);
}


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

Receives: reference to a matrix
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



std::istream& operator>>(std::istream& in, Matrix& mat)
{
	if (&in == &std::cin)
	{
		return in;
	}
	else
	{
		Matrix::freeMatrix(mat);
		std::string row;
		double num;

		int columnSize = 0;
		int rowsLogicalSize, rowsPhysicalSize;
		rowsLogicalSize = 0;
		rowsPhysicalSize = 1;

		int colLogicalSize, colPhysicalSize;
		colLogicalSize = 0;
		colPhysicalSize = 1;
		
		double** matFromFile = new double* [rowsPhysicalSize];
		
		//int ROW_CNT = 0; // TODO: delete

		int cnt = 0;
		while (std::getline(in, row) and cnt < 500)
		{
			cnt++;
			std::stringstream ss(row);

			
			if (rowsLogicalSize >= rowsPhysicalSize)
			{
				rowsPhysicalSize *= 2;
				double** newMatSize = new double* [rowsPhysicalSize];
				for (int i = 0; i < rowsLogicalSize; ++i)
				{
					newMatSize[i] = matFromFile[i];
				}
				delete[]matFromFile;
				matFromFile = newMatSize;
			}

			colLogicalSize = 0;
			colPhysicalSize = 1;

			if (columnSize != 0)
			{
				matFromFile[rowsLogicalSize] = new double[columnSize];
			} 
			else
			{
				matFromFile[rowsLogicalSize] = new double[colPhysicalSize];
			}

			
			while (ss >> num)
			{
			

				if (columnSize != 0)
				{
					matFromFile[rowsLogicalSize][colLogicalSize++] = num;
				}
				else
				{
					if (colLogicalSize >= colPhysicalSize)
					{
						colPhysicalSize *= 2;
						double* newColSize = new double[colPhysicalSize];

						for (int i = 0; i < colLogicalSize; ++i)
						{
							newColSize[i] = matFromFile[rowsLogicalSize][i];
						}
						delete [] matFromFile[rowsLogicalSize];
						matFromFile[rowsLogicalSize] = newColSize;
					}

					
					matFromFile[rowsLogicalSize][colLogicalSize++] = num;
					
				}
			}
			columnSize = colLogicalSize;
			rowsLogicalSize++;
		}
		mat.rows = rowsLogicalSize;
		mat.columns = colLogicalSize;
		mat.matrix = matFromFile;
	}

	
	return in;

}

// 
std::ostream& operator<<(std::ostream& os, const Matrix& mat)
{
	for (int i = 0; i < mat.rows; ++i)
	{
		os << mat.matrix[i][0];
		for (int j = 1; j < mat.columns; ++j)
		{
			os << ' ' << mat.matrix[i][j];
		}
		os << std::endl;
	}
	return os;
}


// ==================== End of Operators ====================

void Matrix::printMat() const
{
	//std::cout << std::fixed << std::setprecision(1);
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < columns; ++j)
		{
			if (matrix[i][j] >= 0)
				std::cout << " ";
			std::cout << /*std::setw(5) << std::left <<*/ matrix[i][j] << " ";
		}
		std::cout << std::endl;
	}
}


void Matrix::freeMatrix(Matrix& mat)
{
	// free prev matrix
	for (int i = 0; i < mat.rows; ++i)
		delete[]mat.matrix[i];
	delete []mat.matrix;
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
For use in operation +/- between to matrices
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
			{
				outputMat[i][j] = op(mat1.matrix[i][j], mat2.matrix[i][j]);
				outputMat[i][j] = 1;
				//std::cout << outputMat[i][j] ;
			}
			
		}
		//Matrix m(newRows, newCols, outputMat);
		//m.printMat();
		
	}

	else if ((mat1.rows == 1 or mat2.rows == 1) and mat1.columns == mat2.columns) // matrix and row vector -> broadcast the vector 
	{

		newRows = mat1.rows != 1 ? mat1.rows : mat2.rows;
		newCols = mat1.columns;

		const double* rowVector = mat1.rows == 1 ? mat1[0] : mat2[0];
		double** matrix = mat1.rows == 1 ? mat2.getMatrix() : mat1.getMatrix();

		outputMat = new double* [newRows];
		for (int i = 0; i < newRows; ++i)
		{
			outputMat[i] = new double[newCols];
			for (int j = 0; j < newCols; ++j)
				//outputMat[i][j] = op(matrix[i][j], rowVector[j]);
				outputMat[i][j] = 1;
			
		}
		

	}
	else if ((mat1.columns == 1 or mat2.columns == 1) and mat1.rows == mat2.rows) // matrix and column vector -> broadcast the vector
	{

		newCols = mat1.columns == 1 ? mat2.columns : mat1.columns;
		newRows = mat1.rows;

		double** columnVector = mat1.columns == 1 ? mat1.getMatrix() : mat2.getMatrix();
		double** matrix = mat1.columns == 1 ? mat2.getMatrix() : mat1.getMatrix();

		outputMat = new double* [newRows];
		for (int i = 0; i < newCols; ++i)
		{
			outputMat[i] = new double[newCols];
			for (int j = 0; j < newRows; ++j)
				outputMat[i][j] = op(matrix[j][i], columnVector[j][0]);
		}

	}
	else {
		// TODO:
			// throw and exception
		throw - 1;
	}

	
	//Matrix m(newRows, newCols, outputMat);
	//m.printMat();
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

Matrix Matrix::applyFunc(const Matrix& mat, std::function<double(double)> func)
{

	int matRows, matCols;
	matRows = mat.getRows();
	matCols = mat.getColumns();

	double** matMatrix = mat.getMatrix();

	double** newMatMatrix = new double* [matRows];

	for (int i = 0; i < matRows; ++i)
	{
		newMatMatrix[i] = new double[matCols];
		for (int j = 0; j < matCols; ++j)
			newMatMatrix[i][j] = func(matMatrix[i][j]);
	}

	return Matrix(matRows, matCols, newMatMatrix);
}

Matrix Matrix::slice(int start, int end, const Matrix& mat)
{
	int size = end - start;
	double** slicedMat = new double*[size];
	int index = 0;

	for (int i = start; i < end; ++i)
	{
		slicedMat[index] = new double[mat.columns];
		std::memcpy(slicedMat[index++], mat.matrix[i], mat.columns * sizeof(double));
	}

	return Matrix(size, mat.columns, slicedMat);
}

Matrix Matrix::reduce(const Matrix& mat, int axis, char op)
{

	switch (op)
	{
	case '+':
		return reduceHelper(mat, axis, additionReduce);
		break;
	case '*':
		return reduceHelper(mat, axis, multiplication);
		break;
	default:
		throw - 1;
		break;
	}

}

Matrix Matrix::reduceHelper(const Matrix& mat, int axis, std::function<void(double&, double)> op)
{
	if (axis == 0)
	{
		double** newMat = new double* [1];
		newMat[0] = new double[mat.columns];

		for (int i = 0; i < mat.columns; ++i)
		{
			double sum = 0;
			for (int j = 0; j < mat.rows; ++j)
			{
				op(sum, mat.matrix[i][j]);
			}
			newMat[0][i] = sum;
		}
		
		//Matrix m(1, mat.columns, newMat);
		//m.printMat();
		return Matrix(1, mat.columns, newMat);
	}
	else if (axis == 1)
	{
		double** newMat = new double* [mat.rows];

		for (int i = 0; i < mat.rows; ++i)
		{
			newMat[i] = new double[1];
			double sum = 0;
			for (int j = 0; j < mat.columns; ++j)
			{
				op(sum, mat.matrix[i][j]);
			}
			newMat[i][0] = sum;
		}

		//Matrix(mat.rows, 1, newMat).printMat();
		return Matrix(mat.rows, 1, newMat);
	}
	else
		throw - 1;
}

double Matrix::mean(const Matrix& mat) 
{
	double sum = 0;
	for (int i = 0; i < mat.rows; ++i)
		for (int j = 0; j < mat.columns; ++j)
			sum += mat.matrix[i][j];

	return sum / (mat.rows * mat.columns);
}


void Matrix::setMatrix(double** mat, int rows, int cols)
{
	Matrix::freeMatrix(*this);
	matrix = mat;
	this->rows = rows;
	this->columns = cols;
}