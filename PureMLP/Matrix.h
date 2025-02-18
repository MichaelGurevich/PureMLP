#pragma once


class Matrix 
{
private:
	unsigned int rows;
	unsigned int columns;
	double** matrix;

public:
	Matrix(int rows = 0, int cols = 0, bool initRandom = false);
	void transpose();
	~Matrix();


	int getRows() const { return rows; }
	int getColumns() const { return columns; }

	void printMat();
};