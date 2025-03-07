#pragma once
#include <random>

extern unsigned long int seed;
extern std::mt19937 global_rng;


void initSeed();
double getRandomNumber(double low, double high);

template <typename T> 
void swap(T* a, T* b)
{
	T temp = *a;
	*a = *b;
	*b = temp;
}

template <typename T>
int maxIndex(const T* arr, int size)
{
	// TODO: handle size 0

	int maxIndex = 0;
	T max = arr[0];

	for (int i = 1; i < size; ++i)
	{
		if (arr[i] > max)
		{
			maxIndex = i;
			max = arr[i];
		}
	}
	return maxIndex;
}

