#ifndef MISC_H
#define MISC_H
#include <random> // For random number generation
#include <array>
#include <thread>
// Function to generate a random boolean with a given probability of being true
bool random_bool(double probability = 0.5) {
    static std::random_device rd; // Non-deterministic seed source
    static std::mt19937 generator(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<double> distribution(0.0, 1.0); // Uniform distribution [0.0, 1.0)

    return distribution(generator) < probability; // Return true if random value < probability
}

// Function to generate a high-entropy seed sequence
std::mt19937 createGenerator() {
    std::random_device rd;
    std::array<int, 8> seed_data;
    for (auto& seed : seed_data) {
        seed = rd();  // Collect entropy from hardware
    }
    std::seed_seq seed_seq(seed_data.begin(), seed_data.end());  // Create better randomness
    return std::mt19937(seed_seq);  // Return Mersenne Twister engine with strong seed
}

// Thread-local random number generator for thread safety
thread_local std::mt19937 generator = createGenerator();

// Function to generate a random number within [min, max]
int getRandomNumber(int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

#endif