#include <iostream>
#include "Matrix.h"
#include "utils.h"

int main() {
    initSeed();
    std::cout << "======== Testing Constructors ========\n\n";

    // Default constructor
    Matrix mat1;
    std::cout << "mat1 (default constructor):\n\n";
    mat1.printMat();

    // Constructor with size
    Matrix mat2(3, 3, false);
    std::cout << "mat2 (3x3 zero matrix):\n\n";
    mat2.printMat();

    // Constructor with random initialization
    Matrix mat3(3, 3, true);
    std::cout << "mat3 (3x3 random matrix):\n\n";
    mat3.printMat();

    // Copy constructor
    Matrix mat4(mat3);
    std::cout << "mat4 (copy of mat3):\n\n";
    mat4.printMat();

    // Move constructor
    Matrix mat5(std::move(mat4));
    std::cout << "mat5 (moved from mat4):\n\n";
    mat5.printMat();

    std::cout << "======== Testing Operators ========\n\n";

    // Addition
    Matrix mat6 = mat2 + mat3;
    std::cout << "mat6 = mat2 + mat3:\n\n";
    mat6.printMat();

    mat6 += mat2;
    std::cout << "mat6 += mat2:\n\n";
    mat6.printMat();

    // Scalar Addition
    Matrix mat7 = mat2 + 5.0;
    std::cout << "mat7 = mat2 + 5.0:\n\n";
    mat7.printMat();

    mat7 += 2.0;
    std::cout << "mat7 += 2.0:\n\n";
    mat7.printMat();

    // Subtraction
    Matrix mat8 = mat3 - mat2;
    std::cout << "mat8 = mat3 - mat2:\n\n";
    mat8.printMat();

    mat8 -= mat2;
    std::cout << "mat8 -= mat2:\n\n";
    mat8.printMat();

    // Scalar Subtraction
    Matrix mat9 = mat3 - 3.0;
    std::cout << "mat9 = mat3 - 3.0:\n\n";
    mat9.printMat();

    mat9 -= 1.0;
    std::cout << "mat9 -= 1.0:\n\n";
    mat9.printMat();

    // Multiplication
    Matrix mat10 = mat2 * mat3;
    std::cout << "mat10 = mat2 * mat3:\n\n";
    mat10.printMat();

    mat10 *= mat2;
    std::cout << "mat10 *= mat2:\n\n";
    mat10.printMat();

    // Scalar Multiplication
    Matrix mat11 = mat3 * 2.0;
    std::cout << "mat11 = mat3 * 2.0:\n\n";
    mat11.printMat();

    mat11 *= 2.0;
    std::cout << "mat11 *= 2.0:\n\n";
    mat11.printMat();

    // Assignment operator
    Matrix mat12 = mat11;
    std::cout << "mat12 (assigned from mat11):\n\n";
    mat12.printMat();

    // Move assignment
    Matrix mat13 = std::move(mat12);
    std::cout << "mat13 (moved from mat12):\n\n";
    mat13.printMat();

    std::cout << "======== Testing Other Functions ========\n\n";

    // Transpose
    Matrix mat14 = Matrix::transpose(mat3);
    std::cout << "mat14 (transpose of mat3):\n\n";
    mat14.printMat();

    mat3.transpose();
    std::cout << "mat3 (transposed in place):\n\n";
    mat3.printMat();

    // Accessing elements using []
    std::cout << "Accessing element mat3[1][1]: " << mat3[1][1] << "\n\n";

    // Getters
    std::cout << "mat3 Rows: " << mat3.getRows() << ", Columns: " << mat3.getColumns() << "\n\n";

    std::cout << "======== All Tests Completed Successfully ========\n\n";
    return 0;
}
