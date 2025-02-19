#pragma once
#include <random>

void initSeed();
double getRandomNumber(int low, int high);

template <typename T> 
void swap(T* a, T* b)
{
	T temp = *a;
	*a = *b;
	*b = temp;
}