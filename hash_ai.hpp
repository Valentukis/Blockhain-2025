#ifndef HASH_AI_HPP
#define HASH_AI_HPP

#include <chrono>
#include <fstream>
#include <array>
#include <cstdint>
#include <cstring>
#include <unordered_set>
#include <random>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string custom_hash256(const std::string& input);
std::string random_string(size_t length);
void test_collisions(size_t string_length, size_t pairs = 100000);

#endif
