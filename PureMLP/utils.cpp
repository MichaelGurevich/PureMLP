#include "utils.h"

unsigned long int seed;
std::mt19937 global_rng;

void initSeed()
{
	std::random_device rd;
	seed = rd();
	global_rng.seed(seed);
}

double getRandomNumber(double low, double high)
{
	std::uniform_real_distribution<double> dist(low, high);
	return dist(global_rng);
}