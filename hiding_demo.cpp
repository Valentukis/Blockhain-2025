// hiding_demo.cpp
// Demonstrates hiding / puzzle-friendliness with custom_hash256(input + salt)
// Compile with: g++ -std=c++17 -O3 hiding_demo.cpp hash_ai.cpp -o hiding_demo

#include "hash_ai.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// --- Utilities ---
std::string to_hex(const std::vector<uint8_t>& bytes) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t b : bytes) ss << std::setw(2) << (int)b;
    return ss.str();
}

std::string gen_salt_hex(size_t nbytes = 8) {
    static std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    std::vector<uint8_t> v(nbytes);
    for (size_t i = 0; i < nbytes; ++i) v[i] = static_cast<uint8_t>(dist(rng) & 0xFF);
    return to_hex(v); // hex string of salt
}

// charset for brute force (small for demo)
const std::string BF_CHARSET = "abcdefghijklmnopqrstuvwxyz012345";

// Recursively brute-force strings up to max_len (lexicographic)
bool brute_force_known_salt_recursive(const std::string& target_hash,
    const std::string& salt,
    size_t max_len,
    std::string& out_found,
    std::string& cur) {
    if (cur.size() > 0) {
        std::string candidate = cur;
        std::string h = custom_hash256(candidate + salt);
        if (h == target_hash) {
            out_found = candidate;
            return true;
        }
    }
    if (cur.size() == max_len) return false;
    for (char c : BF_CHARSET) {
        cur.push_back(c);
        if (brute_force_known_salt_recursive(target_hash, salt, max_len, out_found, cur)) return true;
        cur.pop_back();
    }
    return false;
}

// Wrapper: brute force when salt is known (search inputs up to max_len)
bool brute_force_known_salt(const std::string& target_hash,
    const std::string& salt,
    size_t max_len,
    std::string& found,
    double& elapsed_ms) {
    auto t0 = std::chrono::high_resolution_clock::now();
    std::string cur;
    bool ok = brute_force_known_salt_recursive(target_hash, salt, max_len, found, cur);
    auto t1 = std::chrono::high_resolution_clock::now();
    elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return ok;
}

// Brute force when salt is unknown: iterate small salt-space and inputs
// NOTE: we keep salt-space tiny for demo (e.g., 2-char salts from BF_CHARSET)
bool brute_force_unknown_salt_small(const std::string& target_hash,
    size_t salt_len_chars,
    size_t max_input_len,
    std::string& found_input,
    std::string& found_salt,
    double& elapsed_ms) {
    auto t0 = std::chrono::high_resolution_clock::now();
    std::string salt_candidate(salt_len_chars, BF_CHARSET[0]);
    // iterate salts lexicographically
    size_t charset_n = BF_CHARSET.size();
    // total salt combinations = charset_n ^ salt_len_chars (will be small for demo)
    // We'll do nested loops according to salt_len_chars (support up to 3 reasonably)
    std::vector<size_t> idx(salt_len_chars, 0);
    bool finished = false;
    while (!finished) {
        // build salt string
        for (size_t i = 0; i < salt_len_chars; ++i) salt_candidate[i] = BF_CHARSET[idx[i]];

        // for each salt, brute-force inputs up to max_input_len
        std::string cur;
        std::string candidate_found;
        if (brute_force_known_salt_recursive(target_hash, salt_candidate, max_input_len, candidate_found, cur)) {
            found_input = candidate_found;
            found_salt = salt_candidate;
            auto t1 = std::chrono::high_resolution_clock::now();
            elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            return true;
        }

        // increment idx vector
        for (size_t pos = 0; pos < salt_len_chars; ++pos) {
            idx[pos]++;
            if (idx[pos] < charset_n) break;
            idx[pos] = 0;
            if (pos + 1 == salt_len_chars) finished = true;
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return false;
}

// --- Demo main ---
int main() {
    // 1) pick a secret (small demo)
    std::string secret = "dog";           // the thing we want to hide
    std::string salt = gen_salt_hex(4);   // e.g., 4 bytes -> 8 hex chars
    std::string target_hash = custom_hash256(secret + salt);

    std::cout << "=== Hiding demo ===\n";
    std::cout << "Secret (kept private): " << secret << "\n";
    std::cout << "Salt (shown to verifier here): " << salt << "\n";
    std::cout << "Hash = H(secret + salt): " << target_hash << "\n\n";

    // 2) Brute-force when salt IS known (attacker knows salt): try all inputs up to length 4
    {
        std::cout << "[Brute-force test] Known salt scenario (search inputs up to length 4)\n";
        std::string found;
        double ms = 0.0;
        bool ok = brute_force_known_salt(target_hash, salt, 4, found, ms);
        if (ok) {
            std::cout << " Found input: \"" << found << "\" in " << ms << " ms\n";
        }
        else {
            std::cout << " Not found among inputs length <=4 (took " << ms << " ms)\n";
        }
        std::cout << "\n";
    }

    // 3) Brute-force when salt is UNKNOWN (very small salt-space for demo)
    {
        std::cout << "[Brute-force test] Unknown salt scenario (salt len 2 chars from small charset, inputs up to len 3)\n";
        std::string found_input, found_salt;
        double ms = 0.0;
        bool ok = brute_force_unknown_salt_small(target_hash, 2, 3, found_input, found_salt, ms);
        if (ok) {
            std::cout << " Found input: \"" << found_input << "\" with salt \"" << found_salt << "\" in " << ms << " ms\n";
        }
        else {
            std::cout << " Not found in small salt-space+input-space (took " << ms << " ms)\n";
        }
        std::cout << "\n";
    }

    // 4) Demonstration point
    std::cout << "=== Notes ===\n";
    std::cout << "- If the salt is known and input space is small, attackers can brute-force quickly (demo above).\n";
    std::cout << "- If salt is unknown or large (e.g., 16 random bytes), the attacker must search both salt and input, which becomes infeasible.\n";
    std::cout << "- For real hiding/puzzle-friendliness, use per-secret random salts + expensive KDFs (Argon2/PBKDF2) for rate-limiting.\n";

    return 0;
}
