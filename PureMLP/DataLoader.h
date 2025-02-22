#pragma once
#include <vector>
#include <string>
#include "Matrix.h"

using namespace std;

class DataLoader
{
private:
	Matrix data;
	Matrix labels;

public:
	DataLoader(string dataFileName, string labelsFileName);
};