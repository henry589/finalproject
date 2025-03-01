#ifndef MISC_H
#define MISC_H
#include <random> // For random number generation
#include <array>
#include <thread>

// Generates a random boolean with a given probability (default = 0.5)
bool random_bool(double probability = 0.5);

// Generates a random number within the range [min, max]
int getRandomNumber(int min, int max);

int fast_rand();
// Creates a high-entropy seed sequence for the random number generator
std::mt19937 createGenerator();

    


#endif