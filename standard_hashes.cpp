#include "hash_ai.hpp"
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <bitset>
#include <numeric>

// Valentino hash funkcija

uint64_t hash_own_raw(const std::string& ivestis) {
    uint64_t seed = 371928463890165017ull;
    std::vector<std::bitset<8>> separate_bytes;

    for (unsigned char c : ivestis) {
        separate_bytes.push_back(std::bitset<8>(c));
    }

    //Permaisyt byetus
    if (!separate_bytes.empty()) {
        int k = (ivestis.size() * 781928401873 + separate_bytes.front().count()) % separate_bytes.size();
        if (k == 0 && separate_bytes.size() > 1) k = 1;     // Butinas persukimas
        rotate(separate_bytes.begin(), separate_bytes.begin() + k, separate_bytes.end());
    }

    std::bitset<64> hash(seed); //xor kiekviena bit'a is seed ir input
    int index = 0;
    for (const auto& byte : separate_bytes) {

        for (int b = 0; b < 8; ++b, ++index) {
            int pos = index % 64;
            hash[pos] = hash[pos] ^ byte[b];
        }
    }

    uint64_t h = hash.to_ullong();
    h *= 0xFEEDFACECAFEBEEFull; //paskutinis pramaisymas
    h ^= (h >> 29);
    h *= 0x9E3779B97F4A7C15ull;

    return h;
}

//--- Valentino patobulinta funkcija ---

static inline uint64_t rotl64(uint64_t x, unsigned r) {
    return (x << r) | (x >> (64u - r));
}
static inline uint64_t fmix64(uint64_t k) {
    k ^= k >> 33;  k *= 0xff51afd7ed558ccdll;
    k ^= k >> 33;  k *= 0xc4ceb9fe1a85ec53ull;
    k ^= k >> 33;  return k;
}

static bool read_file_all(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    out.assign((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    return true;
}

static std::string hash256(std::string_view s,
    uint64_t seed = 371928463890165017ull) {
    constexpr uint64_t P0 = 11400714819323198485ull; // 0x9E3779B97F4A7C15
    constexpr uint64_t P1 = 14029467366897019727ull; // 0xC2B2AE3D27D4EB4F
    constexpr uint64_t P2 = 9650029242287828579ull; // 0x85EBCA77C2B2AE63
    constexpr uint64_t P3 = 2870177450012600261ull; // odd

    uint64_t h0 = (seed ^ P0) ^ (uint64_t)s.size();
    uint64_t h1 = (seed + P1) ^ 0xD6E8FEB86659FD93ull;
    uint64_t h2 = (seed + P2) ^ 0xA5A3564E2B9CC8D5ull;
    uint64_t h3 = (seed + P3) ^ 0x9E3779B97F4A7C15ull;

    size_t i = 0;
    for (unsigned char x : s) {
        h0 ^= (uint64_t)x + (i * 0x9E + s.size());  h0 = rotl64(h0, 13) * P0;
        h1 ^= (uint64_t)x + i + 0x9E3779B1u;        h1 = rotl64(h1, 17) * P1;
        h2 ^= (uint64_t)x + ((uint64_t)s.size() << (i & 7)); h2 = rotl64(h2, 43) * P2;
        h3 ^= (uint64_t)x + (i * 2u) + (uint64_t)s.size();   h3 = rotl64(h3, 29) * P3;
        ++i;
    }
    if (s.empty()) { 
        h0 ^= 0x243F6A8885A308D3ull; h1 ^= 0x13198A2E03707344ull;
        h2 ^= 0xA4093822299F31D0ull; h3 ^= 0x082EFA98EC4E6C89ull;
    }

    uint64_t a = h0 + rotl64(h2, 17);
    uint64_t b = h1 + rotl64(h3, 21);
    uint64_t c = h2 + rotl64(h0, 32);
    uint64_t d = h3 + rotl64(h1, 37);

    a ^= b >> 1;  b ^= c >> 3;  c ^= d >> 5;  d ^= a >> 7;
    a += d;       b += a;       c += b;       d += c;

    a = fmix64(a ^ (b + c + d));
    b = fmix64(b ^ (a + c + d));
    c = fmix64(c ^ (a + b + d));
    d = fmix64(d ^ (a + b + c));

    std::ostringstream os;
    os << std::hex << std::uppercase << std::setfill('0')
        << std::setw(16) << a
        << std::setw(16) << b
        << std::setw(16) << c
        << std::setw(16) << d;
    return os.str();
}
// -------------------------
    
// Reikiamu hash'u wrapper'iai

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

std::string hash_own(const std::string& input) {
    uint64_t raw = hash_own_raw(input);
    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << raw;
    return ss.str();
}

std::string hash_own_ai(const std::string& input) {
    return hash256(input);
}

// Avalanche testas
template<typename HashFunc>
void test_avalanche(HashFunc hash_func, const std::string& name, size_t string_len, size_t pairs = 10000) {
    std::vector<int> bit_diffs;
    std::vector<int> hex_diffs;
    size_t hash_bits = 0;
    size_t hash_bytes = 0; // bitu skaicius hashe

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

        // Bitu lygio skirtumas
        int bit_diff = 0;
        for (size_t j = 0; j < hash_bytes; ++j) {
            unsigned char x = ha[j] ^ hb[j];
            for (int k = 0; k < 8; ++k)
                if (x & (1 << k)) ++bit_diff;
        }
        bit_diffs.push_back(bit_diff);

        // Hex'u lygio skirtumas
        int hex_diff = 0;
        for (size_t j = 0; j < hash_bytes; ++j)
            if (ha[j] != hb[j]) ++hex_diff;
        hex_diffs.push_back(hex_diff);
    }

    auto minmax_bit = std::minmax_element(bit_diffs.begin(), bit_diffs.end());
    double avg_bit = std::accumulate(bit_diffs.begin(), bit_diffs.end(), 0.0) / bit_diffs.size();
    double avg_bit_pct = (avg_bit / hash_bits) * 100.0;

    auto minmax_hex = std::minmax_element(hex_diffs.begin(), hex_diffs.end());
    double avg_hex = std::accumulate(hex_diffs.begin(), hex_diffs.end(), 0.0) / hex_diffs.size();
    double avg_hex_pct = (avg_hex / hash_bytes) * 100.0;

    std::cout << "Avalanche (" << name << ", length " << string_len << "):\n";
    std::cout << "Bit diff: min=" << *minmax_bit.first << " max=" << *minmax_bit.second
        << " avg=" << avg_bit << " (" << avg_bit_pct << "%)\n";
    std::cout << "Hex diff: min=" << *minmax_hex.first << " max=" << *minmax_hex.second
        << " avg=" << avg_hex << " (" << avg_hex_pct << "%)\n\n";
}



// Koliziju testas
template<typename HashFunc>
void test_collisions(HashFunc hash_func, const std::string& name, size_t string_len, size_t pairs) {
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
/// Konstitucijos testas
template<typename HashFunc>
void benchmark_file(HashFunc hash_func, const std::string& name, const std::string& filename, int repeats = 5) {
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Cannot open file: " << filename << "\n";
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fin, line)) {
        lines.push_back(line + "\n"); 
    }

    std::cout << "\nBenchmarking " << name << " on file: " << filename << "\n";
    std::cout << "Total lines: " << lines.size() << "\n";
    std::cout << "Lines\tBytes\tAvgTime(ms)\n";

    size_t step = 1;
    while (step <= lines.size()) {
        std::ostringstream buffer;
        for (size_t i = 0; i < step; ++i) buffer << lines[i];
        std::string text = buffer.str();

        double total_ms = 0.0;
        for (int r = 0; r < repeats; ++r) {
            auto start = std::chrono::high_resolution_clock::now();
            hash_func(text);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ms = end - start;
            total_ms += ms.count();
        }
        double avg_ms = total_ms / repeats;

        std::cout << step << "\t" << text.size() << "\t" << avg_ms << "\n";

        step *= 2;
    }
}

// Spartos testas
template<typename HashFunc>
double measure_speed(HashFunc hash_func, const std::string& input) {
    auto start = std::chrono::high_resolution_clock::now();
    hash_func(input);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// negriztamumo testas
// Improved Hiding / Irreversibility Test
template<typename HashFunc>
void test_hiding(HashFunc hash_func, const std::string& name) {
    std::string input = "secret_password";
    std::string salt1 = "salt_one";
    std::string salt2 = "salt_two";

    std::cout << "[Hiding Test] " << name << "\n";

    // 1. Determinism
    std::string h1 = hash_func(input + salt1);
    std::string h2 = hash_func(input + salt1);
    std::cout << "Determinism:\n"
              << "  Hash1: " << h1 << "\n"
              << "  Hash2: " << h2 << "\n";

    // 2. Hiding (different salts)
    std::string h3 = hash_func(input + salt2);
    std::cout << "Hiding (different salts):\n"
              << "  Input+salt1: " << h1 << "\n"
              << "  Input+salt2: " << h3 << "\n";

    // 3. Avalanche inside hiding (small input change)
    std::string input2 = "secret_passw0rd"; // one char changed
    std::string h4 = hash_func(input2 + salt1);
    std::cout << "Avalanche (small input change):\n"
              << "  Original: " << h1 << "\n"
              << "  Modified: " << h4 << "\n";

    // 4. Optional brute-force attempt
    std::vector<std::string> guesses = {"1234", "password", "hello"};
    bool recovered = false;
    for (const auto& g : guesses) {
        if (hash_func(g + salt1) == h1) {
            std::cout << "Recovered input guess: " << g << "\n";
            recovered = true;
            break;
        }
    }
    if (!recovered) {
        std::cout << "Brute-force test: None of the guesses matched. Irreversible.\n";
    }

    std::cout << "\n";
}




int main(int argc, char** argv) {
    using HashFunc = std::string(*)(const std::string&);
    std::vector<std::pair<std::string, HashFunc>> hashes = {
        {"Custom", custom_hash256},
        {"Valentino", hash_own},
        {"Valentino patobulintas", hash_own_ai},
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
        test_avalanche(h.second, h.first, 64, 100000);
    }

    std::cout << "--- Collision Test ---\n";
    std::vector<size_t> lengths = { 10, 100, 500, 1000 };

    for (auto& h : hashes) {
        for (size_t len : lengths) {
            test_collisions(h.second, h.first, len, 100000);
        }
    }

    std::cout << "--- Hiding / Irreversibility Test ---\n";
    for (auto& h : hashes) {
        test_hiding(h.second, h.first);
    }


    if (argc == 2) {
        std::string filename = argv[1];
        for (auto& h : hashes) {
            benchmark_file(h.second, h.first, filename);
        }
    }

    return 0;
}
