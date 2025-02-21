#pragma once
#include <vector>
#include <functional>

class Matrix 
{
private:
	unsigned int rows;
	unsigned int columns;
	double** matrix;

	static void freeMatrix(Matrix& mat);
	static Matrix transpose(const Matrix& mat);
	static Matrix applyScalarOperation(const Matrix& mat, double scalar, std::function<double(double, double)> op);

public:
	Matrix(unsigned int rows = 0, unsigned int cols = 0, bool initRandom = false); // c.tor, the initRandom flag is to whether to init the matrix with random numbers
	Matrix() : rows(0), columns(0), matrix(nullptr) {} // empty c'tor
	Matrix(unsigned int rows = 0, unsigned int cols = 0, double** matrix=nullptr) : rows(rows), columns(cols), matrix(matrix) {} 
	Matrix(Matrix&& mat) noexcept;

	Matrix(const Matrix& mat); // copy c'tor
	
	~Matrix() { freeMatrix(*this); } // d'tor

	void transpose(); // transposes the matrix on which called on
	 // transposes the matrix on which called on
	

	// Operators
		// + operator
	friend Matrix operator+(const Matrix& mat1, const Matrix& mat2);
	Matrix& operator+=(const Matrix& mat);
	Matrix& operator+=(double scalar);

	friend Matrix operator+(const Matrix& mat, double scalar)
	{ return Matrix::applyScalarOperation(mat, scalar, [](double a, double b) {return a + b; }); }

	friend Matrix operator+(double scalar, const Matrix& mat)
	{ return Matrix::applyScalarOperation(mat, scalar, [](double a, double b) {return a + b; }); }

	
		// * operator
	friend Matrix operator*(const Matrix& mat1, const Matrix& mat2);
	friend Matrix operator*(const Matrix& mat, double scalar)
	{ return Matrix::applyScalarOperation(mat, scalar, [](double a, double b) {return a * b; }); }
	friend Matrix operator*(double scalar, const Matrix& mat)
	{ return Matrix::applyScalarOperation(mat, scalar, [](double a, double b) {return a * b; }); }


	const double* operator[](unsigned int index) const { return matrix[index]; }

	Matrix& operator=(const Matrix& mat);
	Matrix& operator=(Matrix&& mat) noexcept;
	


	int getRows() const { return rows; } // Getter for matrix's number of rows
	int getColumns() const { return columns; } // Getter for matrix's number of columns
	double** getMatrix() const { return matrix; }

	void setRows(int _rows) { rows = _rows; }
	void setColumns(int _cols) { columns = _cols; }

	void printMat();
};