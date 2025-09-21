// hash_tool_min.cpp — minimal console app: manual or file → 256-bit hash
#include <cstdint>
#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>

// ==== low-level helpers ====
static inline uint64_t rotl64(uint64_t x, unsigned r) {
    return (x << r) | (x >> (64u - r));
}
static inline uint64_t fmix64(uint64_t k) {
    k ^= k >> 33;  k *= 0xff51afd7ed558ccdll;
    k ^= k >> 33;  k *= 0xc4ceb9fe1a85ec53ull;
    k ^= k >> 33;  return k;
}

// read entire file to string (binary-safe)
static bool read_file_all(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    out.assign((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
    return true;
}

// ==== the hash (toy, non-crypto) — returns 64 hex chars (256-bit) ====
static std::string hash256(std::string_view s,
                           uint64_t seed = 371928463890165017ull) {
    constexpr uint64_t P0 = 11400714819323198485ull; // 0x9E3779B97F4A7C15
    constexpr uint64_t P1 = 14029467366897019727ull; // 0xC2B2AE3D27D4EB4F
    constexpr uint64_t P2 =  9650029242287828579ull; // 0x85EBCA77C2B2AE63
    constexpr uint64_t P3 =  2870177450012600261ull; // odd

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
    if (s.empty()) { // non-trivial empty input
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

// ==== minimal UI: manual or file ====
int main() {
    std::ios::sync_with_stdio(false);

    int mode = -1;
    std::cout << "Kaip norėsite įvesti tekstą? [0 - rankinis įvedimas, 1 - iš failo]\n";
    if (!(std::cin >> mode)) return 0;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // clear '\n'

    std::string data;
    if (mode == 0) {
        std::cout << "Įveskite tekstą (viena eilutė):\n";
        std::getline(std::cin, data);
    } else if (mode == 1) {
        std::cout << "Failo kelias/pavadinimas:\n";
        std::string path;
        std::getline(std::cin, path);
        if (!read_file_all(path, data)) {
            std::cerr << "Nepavyko atidaryti: " << path << "\n";
            return 1;
        }
    } else {
        std::cout << "Neteisingas pasirinkimas.\n";
        return 0;
    }

    const std::string digest = hash256(data);
    std::cout << "Hash (256): " << digest << "\n";
    return 0;
}
