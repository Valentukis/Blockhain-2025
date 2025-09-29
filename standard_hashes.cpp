#include "hash_ai.hpp"
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_set>
#include <random>
#include <algorithm>

// --- Standard Hash Wrappers ---

std::string md5_hash(const std::string& input) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(input.data()), input.size(), digest);
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) ss << std::setw(2) << (int)digest[i];
    return ss.str();
}

std::string sha1_hash(const std::string& input) {
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(input.data()), input.size(), digest);
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) ss << std::setw(2) << (int)digest[i];
    return ss.str();
}

std::string sha256_hash(const std::string& input) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.data()), input.size(), digest);
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) ss << std::setw(2) << (int)digest[i];
    return ss.str();
}

// --- Generic Avalanche Test ---
template<typename HashFunc>
void test_avalanche(HashFunc hash_func, const std::string& name, size_t string_len, size_t pairs = 10000) {
    std::vector<int> bit_diffs;
    std::vector<int> hex_diffs;
    size_t hash_bits = 0;
    size_t hash_bytes = 0; // number of bytes in the hash

    for (size_t i = 0; i < pairs; ++i) {
        std::string a = random_string(string_len);
        std::string b = a;
        b[string_len / 2] = (b[string_len / 2] == 'a') ? 'b' : 'a';

        std::string ha = hash_func(a);
        std::string hb = hash_func(b);

        if (hash_bits == 0) {
            hash_bytes = ha.size();
            hash_bits = hash_bytes * 8;
        }

        // Bit-level difference
        int bit_diff = 0;
        for (size_t j = 0; j < hash_bytes; ++j) {
            unsigned char x = ha[j] ^ hb[j];
            for (int k = 0; k < 8; ++k)
                if (x & (1 << k)) ++bit_diff;
        }
        bit_diffs.push_back(bit_diff);

        // Hex-level difference
        int hex_diff = 0;
        for (size_t j = 0; j < hash_bytes; ++j)
            if (ha[j] != hb[j]) ++hex_diff;
        hex_diffs.push_back(hex_diff);
    }

    auto minmax_bit = std::minmax_element(bit_diffs.begin(), bit_diffs.end());
    double avg_bit = std::accumulate(bit_diffs.begin(), bit_diffs.end(), 0.0) / bit_diffs.size();
    double avg_bit_pct = (avg_bit / hash_bits) * 100.0; // percentage

    auto minmax_hex = std::minmax_element(hex_diffs.begin(), hex_diffs.end());
    double avg_hex = std::accumulate(hex_diffs.begin(), hex_diffs.end(), 0.0) / hex_diffs.size();
    double avg_hex_pct = (avg_hex / hash_bytes) * 100.0; // percentage

    std::cout << "Avalanche (" << name << ", length " << string_len << "):\n";
    std::cout << "Bit diff: min=" << *minmax_bit.first << " max=" << *minmax_bit.second
        << " avg=" << avg_bit << " (" << avg_bit_pct << "%)\n";
    std::cout << "Hex diff: min=" << *minmax_hex.first << " max=" << *minmax_hex.second
        << " avg=" << avg_hex << " (" << avg_hex_pct << "%)\n\n";
}



// --- Generic Collision Test ---
template<typename HashFunc>
void test_collisions(HashFunc hash_func, const std::string& name, size_t string_len, size_t pairs = 10000) {
    size_t collision_count = 0;
    std::unordered_set<std::string> seen;

    for (size_t i = 0; i < pairs; ++i) {
        std::string a = random_string(string_len);
        std::string b = random_string(string_len);
        std::string ha = hash_func(a);
        std::string hb = hash_func(b);
        if (ha == hb) ++collision_count;
        seen.insert(ha);
        seen.insert(hb);
    }

    std::cout << "Collisions (" << name << ", length " << string_len << "): "
        << collision_count << " / " << pairs << " pairs\n\n";
}

// --- Speed Test ---
template<typename HashFunc>
double measure_speed(HashFunc hash_func, const std::string& input) {
    auto start = std::chrono::high_resolution_clock::now();
    hash_func(input);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    using HashFunc = std::string(*)(const std::string&);
    std::vector<std::pair<std::string, HashFunc>> hashes = {
        {"Custom", custom_hash256},
        {"MD5", md5_hash},
        {"SHA-1", sha1_hash},
        {"SHA-256", sha256_hash}
    };

    std::string test_input = random_string(1000);

    std::cout << "--- Speed Test ---\n";
    for (auto& h : hashes) {
        double t = measure_speed(h.second, test_input);
        std::cout << h.first << ": " << t << " ms\n";
    }
    std::cout << "\n";

    std::cout << "--- Avalanche Test ---\n";
    for (auto& h : hashes) {
        test_avalanche(h.second, h.first, 64, 10000);
    }

    std::cout << "--- Collision Test ---\n";
    for (auto& h : hashes) {
        test_collisions(h.second, h.first, 64, 10000);
    }

    return 0;
}
