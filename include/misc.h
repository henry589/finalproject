#ifndef MISC_H
#define MISC_H
#include <random> // For random number generation

// Function to generate a random boolean with a given probability of being true
bool random_bool(double probability = 0.5) {
    static std::random_device rd; // Non-deterministic seed source
    static std::mt19937 generator(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<double> distribution(0.0, 1.0); // Uniform distribution [0.0, 1.0)

    return distribution(generator) < probability; // Return true if random value < probability
}

// Function to generate a random number within a range [min, max]
int getRandomNumber(int min, int max) {
    // Static random device and generator for efficiency
    static std::random_device rd; // Non-deterministic seed source
    static std::mt19937 generator(rd()); // Mersenne Twister engine

    // Uniform distribution within the range [min, max]
    std::uniform_int_distribution<int> distribution(min, max);

    // Generate and return the random number
    return distribution(generator);
}

#endif