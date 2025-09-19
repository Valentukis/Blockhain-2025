
#include "hash_ai.hpp"


// rotate left for uint64_t
static inline uint64_t rotl64(uint64_t x, unsigned r) {
    return (x << r) | (x >> (64 - r));
}

// Simple constants derived from fractional parts of some irrational (just for variety)
constexpr std::array<uint64_t, 12> ROUND_CONSTS = {
    0x243F6A8885A308D3ULL, 0x13198A2E03707344ULL,
    0xA4093822299F31D0ULL, 0x082EFA98EC4E6C89ULL,
    0x452821E638D01377ULL, 0xBE5466CF34E90C6CULL,
    0xC0AC29B7C97C50DDULL, 0x3F84D5B5B5470917ULL,
    0x9216D5D98979FB1BULL, 0xD1310BA698DFB5ACULL,
    0x2FFD72DBD01ADFB7ULL, 0xB8E1AFED6A267E96ULL
};

// Permutation/mixing function: mixes the 256-bit state using 16 words of the message block
void permute_state(std::array<uint64_t, 4>& state, const uint64_t block_words[8]) {
    // We'll run 12 rounds
    for (int r = 0; r < 12; ++r) {
        // inject constants
        for (int i = 0; i < 4; ++i) {
            state[i] ^= ROUND_CONSTS[r];
        }

        // mixing step: pairwise nonlinear mixing
        uint64_t a = state[0];
        uint64_t b = state[1];
        uint64_t c = state[2];
        uint64_t d = state[3];

        // combine with block words in different ways
        a = rotl64((a + (block_words[(r + 0) % 8] ^ b)), (7 + r) % 64);
        b = rotl64((b ^ (c + block_words[(r + 1) % 8])), (13 + r) % 64);
        c = rotl64((c + (d ^ block_words[(r + 2) % 8])), (17 + r) % 64);
        d = rotl64((d ^ (a + block_words[(r + 3) % 8])), (23 + r) % 64);

        // small diffusion & permutation among lanes
        state[0] = a + b;
        state[1] = b ^ c;
        state[2] = c + d;
        state[3] = d ^ a;

        // xor-rotate shuffle
        state[0] ^= rotl64(state[1], 11);
        state[2] ^= rotl64(state[3], 19);
    }
}

// Pad message as: message || 0x80 || 0x00* || 128-bit big-endian bit-length
std::vector<uint8_t> pad_message(const std::vector<uint8_t>& msg) {

    uint64_t bit_len_low = static_cast<uint64_t>(msg.size()) * 8ULL;
    // For simplicity assume message length < 2^64 bits (practical for this assignment)
    uint64_t bit_len_high = 0;

    std::vector<uint8_t> out = msg;
    // append 0x80
    out.push_back(0x80);

    // append zeros until final 16 bytes fit
    while ((out.size() + 16) % 64 != 0) {
        out.push_back(0x00);
    }

    // append 128-bit length as big-endian: high (8 bytes) then low (8 bytes)
    for (int i = 7; i >= 0; --i) out.push_back(static_cast<uint8_t>((bit_len_high >> (i * 8)) & 0xFF));
    for (int i = 7; i >= 0; --i) out.push_back(static_cast<uint8_t>((bit_len_low >> (i * 8)) & 0xFF));

    return out;
}

std::string to_hex256(const std::array<uint64_t, 4>& state) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 4; ++i) {
        ss << std::setw(16) << state[i];
    }
    return ss.str();
}

// Convert 64-byte block (or smaller) to 8 uint64_t words (big-endian)
void block_to_words(const uint8_t* block, size_t len, uint64_t out_words[8]) {
    // zero words first
    for (int i = 0; i < 8; ++i) out_words[i] = 0ULL;

    size_t bytes_to_read = (len > 64 ? 64 : len);
    for (size_t i = 0; i < bytes_to_read; ++i) {
        size_t word_idx = i / 8;
        out_words[word_idx] = (out_words[word_idx] << 8) | block[i];
    }

    // if the final word bytes were fewer than 8, we left-shifted too much; compensate:
    // Actually the code above builds big-endian contiguous; for remaining words it is okay.
    // To ensure consistent endianness: if less than 8 bytes were read into a word, we need to shift left to fill 8 bytes
    size_t last_word_bytes = bytes_to_read % 8;
    if (last_word_bytes != 0) {
        // For the last partially filled word, shift remaining to the left so the value sits in high bytes.
        int idx = (bytes_to_read - 1) / 8;
        out_words[idx] <<= (8 * (8 - last_word_bytes));
    }
}

// Main hash function
std::string custom_hash256(const std::string& input) {
    // initialize state with some IV constants (like SHA does)
    std::array<uint64_t, 4> state = {
        0x6A09E667F3BCC908ULL,
        0xBB67AE8584CAA73BULL,
        0x3C6EF372FE94F82BULL,
        0xA54FF53A5F1D36F1ULL
    };

    // convert input to bytes
    std::vector<uint8_t> bytes(input.begin(), input.end());
    auto padded = pad_message(bytes);

    // process each 64-byte block
    for (size_t pos = 0; pos < padded.size(); pos += 64) {
        const uint8_t* block_ptr = padded.data() + pos;
        uint64_t words[8];
        block_to_words(block_ptr, 64, words);

        // XOR block into state in 64-bit lanes (simple feed)
        for (int i = 0; i < 4; ++i) {
            state[i] ^= words[i % 8];
        }

        // apply permutation/mixing
        permute_state(state, words);
    }

    // finalization: run a few extra permutation rounds with zero block to diffuse
    uint64_t zero_block[8] = { 0 };
    for (int i = 0; i < 4; ++i) state[i] ^= 0x0123456789ABCDEFULL ^ state[(i + 1) % 4];
    permute_state(state, zero_block);
    permute_state(state, zero_block);

    return to_hex256(state);
}

#include <string>
#include <random>

std::string random_string(size_t length) {
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*()-_=+[]{}|;:,.<>?";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string s(length, ' ');
    for (size_t i = 0; i < length; ++i) {
        s[i] = charset[dist(rng)];
    }
    return s;
}

void test_collisions(size_t string_length, size_t pairs) {
    size_t collision_count = 0;

    std::unordered_set<std::string> seen_hashes;

    for (size_t i = 0; i < pairs; ++i) {
        std::string a = random_string(string_length);
        std::string b = random_string(string_length);

        std::string hash_a = custom_hash256(a);
        std::string hash_b = custom_hash256(b);

        if (hash_a == hash_b) {
            ++collision_count;
            std::cout << "Collision found!\n";
        }

        // optional: store hashes to detect duplicates in the whole set
        seen_hashes.insert(hash_a);
        seen_hashes.insert(hash_b);
    }

    std::cout << "String length: " << string_length
        << ", collisions in " << pairs << " pairs: "
        << collision_count << "\n";
}