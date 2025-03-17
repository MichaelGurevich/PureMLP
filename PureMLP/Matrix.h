#pragma once
#include <vector>
#include <functional>
#include <iostream>
#include <sstream>

#include <fstream>  // For file streams

class Matrix 
{
private:

	unsigned int rows;
	unsigned int columns;
	double** matrix;

	enum eRDUCE_AXIS { ALONG_Y, ALONG_X };

	
	static constexpr double addtion(double a, double b) { return a + b; }
	static constexpr double subraction(double a, double b) { return a - b; }
	static constexpr double multiplication(double a, double b) { return a * b; }
	static constexpr double division(double a, double b)
	{ 
		if (b == 0)
			throw - 1;
		return a / b;
	}

	static void freeMatrix(Matrix& mat); 
	
	static Matrix applyScalarOperation(const Matrix& mat, double scalar, std::function<double(double, double)> op);
	void applyScalarOperation(double scalar, std::function<double(double, double)> op);

	static Matrix zipWith(const Matrix& mat1, const Matrix& mat2, std::function<double(double, double)> op);
	void zipWith(const Matrix& mat, std::function<double(double, double)> op);
	
	static Matrix reduceHelper(const Matrix& mat, int axis, std::function<void(double&, const double)> op);
	

public:
	Matrix(unsigned int rows = 0, unsigned int cols = 0, bool initRandom=false); // c.tor, the initRandom flag is to whether to init the matrix with random numbers
	Matrix(unsigned int rows, unsigned int cols, double** matrix) : rows(rows), columns(cols), matrix(matrix) {} 
	

	// c'tor of a row vector for std::vector
	template<typename T> 
	Matrix(std::vector<T> vect)
	{
		this->columns = vect.size();

		double** newMatrix = new double* [1];
		newMatrix[0] = new double[this->columns];

		for (int i = 0; i < this->columns; ++i)
			newMatrix[0][i] = static_cast<double> (vect[i]);
		
		this->rows = 1;
		this->matrix = newMatrix;
	}

	// c'tor of a matrix form a nested std::vector
	template<typename T>
	Matrix(std::vector<std::vector<T>> mat)
	{
		this->rows = mat.size();
		this->columns = mat[0].size();

		double** newMatrix = new double* [this->rows];
		

		for (int i = 0; i < this->rows; ++i)
		{
			newMatrix[i] = new double[this->columns];
			for (int j = 0; j < this->columns; ++j)
			{
				newMatrix[i][j] = static_cast<double> (mat[i][j]);
			}
		}

		this->matrix = newMatrix;
	}



	Matrix(Matrix&& mat) noexcept; // move c'tor

	Matrix(const Matrix& mat); // copy c'tor
	
	~Matrix() { freeMatrix(*this); } // d'tor

	// Operators
		// + operator
	friend Matrix operator+(const Matrix& mat1, const Matrix& mat2) { return Matrix::zipWith(mat1, mat2, Matrix::addtion); }
	friend Matrix operator+(const Matrix& mat, double scalar) { return Matrix::applyScalarOperation(mat, scalar, addtion); }
	friend Matrix operator+(double scalar, const Matrix& mat) { return Matrix::applyScalarOperation(mat, scalar, addtion); }
	Matrix& operator+=(const Matrix& mat);
	Matrix& operator+=(double scalar);
	
	
		// - operator
	friend Matrix operator-(const Matrix& mat1, const Matrix& mat2) { return Matrix::zipWith(mat1, mat2, Matrix::subraction); }
	friend Matrix operator-(const Matrix& mat, double scalar) { return Matrix::applyScalarOperation(mat, scalar, subraction); }
	friend Matrix operator-(double scalar, const Matrix& mat);
	Matrix& operator-=(const Matrix& mat);
	Matrix& operator-=(double scalar);

	
		// * operator
	friend Matrix operator*(const Matrix& mat1, const Matrix& mat2);
	friend Matrix operator*(const Matrix& mat, double scalar) { return Matrix::applyScalarOperation(mat, scalar, multiplication); }
	friend Matrix operator*(double scalar, const Matrix& mat) { return Matrix::applyScalarOperation(mat, scalar, multiplication); }
	Matrix& operator*=(const Matrix& mat);
	Matrix& operator*=(double scalar);

		// '/' operator
	friend Matrix operator/(const Matrix& mat, double scalar) { return Matrix::applyScalarOperation(mat, scalar, division); }
	

		// & operator - element-wise multiplication
	friend Matrix operator& (const Matrix& mat1, const Matrix& mat2) { return Matrix::zipWith(mat1, mat2, multiplication); }

		// [ ] Operator
	const double* operator[](unsigned int index) const { return matrix[index]; }


		// = Operator
	Matrix& operator=(const Matrix& mat);
	Matrix& operator=(Matrix&& mat) noexcept;

		// >> Operator
	friend std::istream& operator>>(std::istream& in, Matrix& mat);

		// << Opeartor
	friend std::ostream& operator<<(std::ostream& os, const Matrix& mat);
	

	void transpose(); // in place transpose
	static Matrix transpose(const Matrix& mat); // returns transposed matrix

	static Matrix reduce(const Matrix& mat, int axis = ALONG_Y, char op = '+'); // reduce matrix to row/column vector

	void applyFunc(std::function<double(double)>& func); // in place applies given function on each element of the matrix 
	static Matrix applyFunc(const Matrix& mat ,std::function<double(double)> func); // return new matrix after applying given func on each of matrix elements

	int getRows() const { return rows; } // Getter for matrix's number of rows
	int getColumns() const { return columns; } // Getter for matrix's number of columns
	double** getMatrix() const { return matrix; }

	void setMatrix(double** mat, int rows, int cols);

	static double mean(const Matrix& mat); // returns - all matrix elemets sum divided by the number of elements 

	void printMat() const;
	void printSize() const { std::cout << rows << " x " << columns << std::endl; }

	static Matrix slice(int start, int end, const Matrix& mat); // returns specified portion of the matrix



	

};