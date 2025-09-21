// v0.2 — simple 256-bit hash (toy, non-cryptographic)
// Build: g++ -O3 -std=c++17 hash.cpp -o hash
#include <cstdint>
#include <string>
#include <string_view>
#include <iomanip>
#include <sstream>
#include <vector>

// ---------- tiny helpers ----------
static inline uint64_t rotl64(uint64_t x, unsigned r) {
    return (x << r) | (x >> (64u - r));
}

// MurmurHash3-style avalanche for a 64-bit word
static inline uint64_t fmix64(uint64_t k) {
    k ^= k >> 33;  k *= 0xff51afd7ed558ccdll;
    k ^= k >> 33;  k *= 0xc4ceb9fe1a85ec53ull;
    k ^= k >> 33;
    return k;
}

// format 32 bytes (4x u64) to 64 hex chars
static std::string to_hex256(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    std::ostringstream os;
    os << std::hex << std::uppercase << std::setfill('0');
    os << std::setw(16) << a
       << std::setw(16) << b
       << std::setw(16) << c
       << std::setw(16) << d;
    return os.str();
}

// ---------- the hash (keeps your simple spirit, but stronger) ----------
std::string hash256(std::string_view s,
                    uint64_t seed = 371928463890165017ull,
                    std::string_view salt = {}) {
    // Four 64-bit lanes → 256-bit state
    // Distinct, odd, well-mixed init constants (golden ratio / xxHash-ish)
    constexpr uint64_t P0 = 11400714819323198485ull; // 0x9E3779B97F4A7C15
    constexpr uint64_t P1 = 14029467366897019727ull; // 0xC2B2AE3D27D4EB4F
    constexpr uint64_t P2 =  9650029242287828579ull; // 0x85EBCA77C2B2AE63
    constexpr uint64_t P3 =  2870177450012600261ull; // random-ish odd

    // Mix salt (if any) into seed first (simple fold)
    uint64_t ss = seed ^ (uint64_t)s.size() * 0x9E3779B185EBCA87ull;
    for (unsigned char ch : salt) {
        ss ^= (uint64_t)ch;
        ss = rotl64(ss * 0x100000001B3ull, 23) ^ 0xBF58476D1CE4E5B9ull;
    }

    uint64_t h0 = ss ^ P0 ^ (uint64_t)s.size();
    uint64_t h1 = (ss + P1) ^ 0xD6E8FEB86659FD93ull;
    uint64_t h2 = (ss + P2) ^ 0xA5A3564E2B9CC8D5ull;
    uint64_t h3 = (ss + P3) ^ 0x9E3779B97F4A7C15ull;

    // Per-byte mix: XOR → rotate → multiply (cheap + good diffusion)
    // Each lane uses a slightly different schedule to avoid symmetry
    size_t i = 0;
    for (unsigned char byte : s) {
        uint64_t x = (uint64_t)byte;

        h0 ^= x + (i * 0x9E + s.size());
        h0  = rotl64(h0, 13) * P0;

        h1 ^= x + i + 0x9E3779B1u;
        h1  = rotl64(h1, 17) * P1;

        h2 ^= x + (s.size() << (i & 7));
        h2  = rotl64(h2, 43) * P2;

        h3 ^= x + (i * 2u) + (uint64_t)s.size();
        h3  = rotl64(h3, 29) * P3;

        ++i;
    }

    // If empty input, still move state (avoid trivial output)
    if (s.empty()) {
        h0 ^= 0x243F6A8885A308D3ull; h1 ^= 0x13198A2E03707344ull;
        h2 ^= 0xA4093822299F31D0ull; h3 ^= 0x082EFA98EC4E6C89ull;
    }

    // Cross-lane fold then strong avalanching on each lane
    // (simple, branchless, fast)
    uint64_t a = h0 + rotl64(h2, 17);
    uint64_t b = h1 + rotl64(h3, 21);
    uint64_t c = h2 + rotl64(h0, 32);
    uint64_t d = h3 + rotl64(h1, 37);

    // One more round of mixing across lanes
    a ^= b >> 1;  b ^= c >> 3;  c ^= d >> 5;  d ^= a >> 7;
    a += d;       b += a;       c += b;       d += c;

    // Finalize each lane (Murmur fmix64) and also inter-mix once
    a = fmix64(a ^ (b + c + d));
    b = fmix64(b ^ (a + c + d));
    c = fmix64(c ^ (a + b + d));
    d = fmix64(d ^ (a + b + c));

    // 256-bit digest as 64 hex chars
    return to_hex256(a, b, c, d);
}

// ---------- (optional) 64-bit variant if you want to compare ----------
uint64_t hash64(std::string_view s,
                uint64_t seed = 371928463890165017ull,
                std::string_view salt = {}) {
    auto hex = hash256(s, seed, salt);         // reuse 256
    // collapse 256→64 (xor-fold): fast and keeps avalanche decent
    // parse 4x16-hex chunks back to u64, then xor them (simple)
    uint64_t w[4]{};
    for (int k = 0; k < 4; ++k) {
        uint64_t v = 0;
        for (int j = 0; j < 16; ++j) {
            char c = hex[k*16 + j];
            v <<= 4;
            v |= (c <= '9') ? (c - '0') : (10 + c - 'A');
        }
        w[k] = v;
    }
    return w[0] ^ w[1] ^ w[2] ^ w[3];
}

// ------------------ example usage ------------------

#include <iostream>
int main() {
    std::string s;
    std::getline(std::cin, s);
    std::cout << hash256(s) << "\n";
}
