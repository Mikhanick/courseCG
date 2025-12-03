#pragma once
#include <random>

// Global random seed variable
extern unsigned int randomSeed;

// Global random number generator
extern std::mt19937 globalRandomGenerator;

// Function to update random generators with new seed
void updateRandomGenerators(unsigned int newSeed);