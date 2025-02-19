#pragma once


class Matrix 
{
private:
	unsigned int rows;
	unsigned int columns;
	double** matrix;

	static void freeMatrix(Matrix& mat);
	static double** transpose(const Matrix& mat);

public:
	Matrix(unsigned int rows = 0, unsigned int cols = 0, bool initRandom = false); // c.tor, the initRandom flag is to whether to init the matrix with random numbers
	Matrix() : rows(0), columns(0), matrix(nullptr) {} // empty c'tor
	void transpose(); // transposes the matrix on which called on
	 // transposes the matrix on which called on
	~Matrix() { freeMatrix(*this);  } // d'tor

	// Operators
	void operator+(const Matrix& matToAdd);
	friend double** operator+(const Matrix& mat1, const Matrix& mat2);
	const double* operator[](unsigned int index) const { return matrix[index]; };


	int getRows() const { return rows; } // Getter for matrix's number of rows
	int getColumns() const { return columns; } // Getter for matrix's number of columns
	double** getMatrix() const { return matrix; }

	void setRows(int _rows) { rows = _rows; }
	void setColumns(int _cols) { columns = _cols; }

	void printMat();
};