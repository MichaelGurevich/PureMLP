#include "Matrix.h"
#include "iostream"


int main()
{

	Matrix mat(4, 3, true);
	mat.printMat();
	std::cout << "\n\n";
	mat.transpose();
	mat.printMat();

	return 0;
}