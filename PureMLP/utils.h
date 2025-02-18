#pragma once
#include <random>

unsigned long int seed;
std::mt19937 global_rng;

void initSeed()
{
	std::random_device rd;
	seed = rd(); 
	global_rng.seed(seed);
}

double getRandomNumber(int low, int high)
{
	std::uniform_real_distribution<double> dist(low, high);
	return dist(global_rng);
}

template <typename T> 
void swap(T* a, T* b)
{
	T temp = *a;
	*a = *b;
	*b = temp;
}