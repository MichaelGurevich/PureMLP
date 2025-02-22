#pragma once
#include <vector>
#include <functional>
#include <iostream>
#include <sstream>

class Matrix 
{
private:

	unsigned int rows;
	unsigned int columns;
	double** matrix;

	static constexpr double addtion(double a, double b) { return a + b; }
	static constexpr double subraction(double a, double b) { return a - b; }
	static constexpr double multiplication(double a, double b) { return a * b; }

	static void freeMatrix(Matrix& mat);
	
	static Matrix applyScalarOperation(const Matrix& mat, double scalar, std::function<double(double, double)> op);
	static Matrix linearOperation(const Matrix& mat1, const Matrix& mat2, std::function<double(double, double)> op);
	void linearOperation(const Matrix& mat, std::function<double(double, double)> op);
	void applyScalarOperation(double scalar, std::function<double(double, double)> op);
	

public:
	Matrix(unsigned int rows = 0, unsigned int cols = 0, bool initRandom=false); // c.tor, the initRandom flag is to whether to init the matrix with random numbers
	Matrix(unsigned int rows, unsigned int cols, double** matrix) : rows(rows), columns(cols), matrix(matrix) {} 

	Matrix(Matrix&& mat) noexcept; // move c'tor

	Matrix(const Matrix& mat); // copy c'tor
	
	~Matrix() { freeMatrix(*this); } // d'tor

	// Operators
		// + operator
	friend Matrix operator+(const Matrix& mat1, const Matrix& mat2) { return Matrix::linearOperation(mat1, mat2, Matrix::addtion); }
	friend Matrix operator+(const Matrix& mat, double scalar) { return Matrix::applyScalarOperation(mat, scalar, addtion); }
	friend Matrix operator+(double scalar, const Matrix& mat) { return Matrix::applyScalarOperation(mat, scalar, addtion); }
	Matrix& operator+=(const Matrix& mat);
	Matrix& operator+=(double scalar);
	
	
		// - operator
	friend Matrix operator-(const Matrix& mat1, const Matrix& mat2) { return Matrix::linearOperation(mat1, mat2, Matrix::addtion); }
	friend Matrix operator-(const Matrix& mat, double scalar) { return Matrix::applyScalarOperation(mat, scalar, subraction); }
	friend Matrix operator-(double scalar, const Matrix& mat) { return Matrix::applyScalarOperation(mat, scalar, subraction); }
	Matrix& operator-=(const Matrix& mat);
	Matrix& operator-=(double scalar);

	
		// * operator
	friend Matrix operator*(const Matrix& mat1, const Matrix& mat2);
	friend Matrix operator*(const Matrix& mat, double scalar) { return Matrix::applyScalarOperation(mat, scalar, multiplication); }
	friend Matrix operator*(double scalar, const Matrix& mat) { return Matrix::applyScalarOperation(mat, scalar, multiplication); }
	Matrix& operator*=(const Matrix& mat);
	Matrix& operator*=(double scalar);
	

		// [ ] Operator
	const double* operator[](unsigned int index) const { return matrix[index]; }


		// = Operator
	Matrix& operator=(const Matrix& mat);
	Matrix& operator=(Matrix&& mat) noexcept;

		// >> opeartor
	friend std::istream& operator>>(std::istream& in, Matrix& mat);
	

	void transpose(); // transposes the matrix on which called on
	static Matrix transpose(const Matrix& mat);

	void applyFunc(std::function<double(double)>& func);

	int getRows() const { return rows; } // Getter for matrix's number of rows
	int getColumns() const { return columns; } // Getter for matrix's number of columns
	double** getMatrix() const { return matrix; }

	//void setRows(int _rows) { rows = _rows; }
	//void setColumns(int _cols) { columns = _cols; }

	void printMat();
	void printSize() { std::cout << rows << " x " << columns << std::endl; }

};