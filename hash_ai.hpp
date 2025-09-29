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
void test_avalanche(size_t string_len = 100, size_t pairs = 100000);
inline std::string random_string(size_t length) {
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:,.<>?";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string s(length, ' ');
    for (size_t i = 0; i < length; ++i) s[i] = charset[dist(rng)];
    return s;
}

#endif
