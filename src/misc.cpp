#include "../include/misc.h"

thread_local bool seeded = false;
thread_local uint64_t gState;
thread_local uint64_t gInc;

void seedPcg() {
	if (!seeded) {
		// for instance, combine time + random_device
		std::random_device rd;
		gState = ((uint64_t(rd()) << 32) ^ rd()) ^ (static_cast<uint64_t>(time(nullptr)) << 1);
		gInc = ((uint64_t(rd()) << 32) ^ rd()) | 1ULL;  // ensure increment is odd
		seeded = true;
	}
}

// Advance PCG state and return a 32-bit random value
static inline uint32_t pcg32() {
	uint64_t oldstate = gState;
	// Advance internal state
	gState = oldstate * 6364136223846793005ULL + (gInc | 1ULL);
	// Calculate output function (XSH RR), uses old state
	uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
	uint32_t rot = static_cast<uint32_t>(oldstate >> 59u);
	return (xorshifted >> rot) | (xorshifted << ((-static_cast<int>(rot)) & 31));
}

bool random_bool(double p)
{
	// Clamp to [0,1]
	if (p <= 0.0) return false;
	if (p >= 1.0) return true;

	seedPcg(); // Ensure PCG is seeded one time

	// Draw 32 bits
	uint32_t r = pcg32();

	// Scale probability to [0, 2^32 - 1]
	uint32_t cutoff = static_cast<uint32_t>(p * 4294967295.0);

	return (r <= cutoff);
}

// ------------------------------------------------
// fast_rand() replacement for rand()
// Returns value in [0, RAND_MAX]
// ------------------------------------------------
int fast_rand() {
	// Seed PCG once per thread
	seedPcg();

	// Generate 32 bits of randomness
	const uint32_t& r = pcg32();

	// Map 32-bit result into [0, RAND_MAX]
	// NOTE: This introduces a tiny modulo bias unless (RAND_MAX+1) divides 2^32
	return static_cast<int>(r % (RAND_MAX + 1U));
}

std::mt19937 createGenerator() {
	std::random_device rd;
	// Collect multiple 32-bit values from the random_device
	std::array<std::uint32_t, 8> seed_data;
	for (auto& val : seed_data) {
		val = rd();
	}
	std::seed_seq seedSeq(seed_data.begin(), seed_data.end());
	return std::mt19937(seedSeq);
}

// 2) Thread-local engine - each thread gets its own RNG, no locking
thread_local std::mt19937 generator = createGenerator();

// 3) Super-fast integer generator, with a tiny modulo bias
int getRandomNumber(int min, int max) {
	// One RNG call -> 32-bit random number
	const std::uint32_t& r = generator();

	// Range size
	const int& range = (max - min + 1);

	// Slight bias if range doesn't divide 2^32
	// For most ranges, this bias is negligible.
	const int& val = r % range;

	return min + val;
}