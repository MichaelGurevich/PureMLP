#include "utils.h"
#include "Matrix.h"
#include "iostream"



int main()
{
	initSeed();
	Matrix mat(4, 3, true);
	Matrix mat1(4, 3, true);
	mat.printMat();
	std::cout << "\n\n" <<  mat[2][0] ;
	/*
	std::cout << "\n\n";
	mat1.printMat();
	std::cout << "\n\n";
	mat + mat1;
	mat.printMat();
	std::cout << "\n\n";*/
	return 0;
}