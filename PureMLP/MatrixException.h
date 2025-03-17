#pragma once
#ifndef MATRIX_EXCEPTION_H
#define MATRIX_EXCEPTION_H

#include <stdexcept>
#include <string>

// Base exception class for matrix errors
class MatrixException : public std::runtime_error {
public:
    explicit MatrixException(const std::string& msg)
        : std::runtime_error(msg) {}
};

// Thrown when matrix dimensions are invalid (e.g., zero rows or columns)
class InvalidDimensionException : public MatrixException {
public:
    explicit InvalidDimensionException(const std::string& msg)
        : MatrixException(msg) {}
};

// Thrown when matrix operations have mismatched dimensions (e.g., multiplication, zipWith)
class DimensionMismatchException : public MatrixException {
public:
    explicit DimensionMismatchException(const std::string& msg)
        : MatrixException(msg) {}
};

// Thrown when an invalid axis is provided for a reduction operation
class InvalidAxisException : public MatrixException {
public:
    explicit InvalidAxisException(const std::string& msg)
        : MatrixException(msg) {}
};

// Thrown when an unsupported reduction operator is provided
class InvalidReductionOperatorException : public MatrixException {
public:
    explicit InvalidReductionOperatorException(const std::string& msg)
        : MatrixException(msg) {}
};

#endif // MATRIX_EXCEPTION_H
