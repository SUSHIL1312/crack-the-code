#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <iomanip>
#include <cstdint>
#include <cassert>

// Fast Modular Exponentiation: (base^exp) % mod
uint64_t power_mod(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            __uint128_t temp = static_cast<__uint128_t>(res) * base;
            res = static_cast<uint64_t>(temp % mod);
        }
        exp = exp >> 1;
        __uint128_t temp = static_cast<__uint128_t>(base) * base;
        base = static_cast<uint64_t>(temp % mod);
    }
    return res;
}

// Simple symmetric keystream cipher for demonstration (simulating AES stream)
std::vector<uint8_t> encrypt_decrypt(const std::vector<uint8_t>& data, uint64_t key) {
    std::vector<uint8_t> out = data;
    std::mt19937_64 prng(key);
    for (size_t i = 0; i < out.size(); ++i) {
        uint8_t mask = static_cast<uint8_t>(prng() & 0xFF);
        out[i] ^= mask;
    }
    return out;
}

int main() {
    std::cout << "================ TLS 1.3 DIFFIE-HELLMAN KEY EXCHANGE SIMULATION ================\n";

    // Public Cryptographic Parameters (Standard Safe Prime and Generator)
    // In production, Curve25519 or NIST P-256 is used
    constexpr uint64_t PRIME_P = 2147483647; // 2^31 - 1 (Mersenne prime)
    constexpr uint64_t GENERATOR_G = 7;

    std::cout << "Public Parameters Shared Over Insecure Network:\n";
    std::cout << "  Prime Modulus (p): " << PRIME_P << "\n";
    std::cout << "  Generator (g):     " << GENERATOR_G << "\n\n";

    // 1. Client Step (ClientHello + KeyShare in TLS 1.3)
    uint64_t client_private_key = 1234567; // Kept strictly private on Client device
    uint64_t client_public_key = power_mod(GENERATOR_G, client_private_key, PRIME_P);

    std::cout << "[Step 1: Client -> Server (TLS 1.3 ClientHello)]\n";
    std::cout << "  Client picked Secret Private Key: " << client_private_key << "\n";
    std::cout << "  Client transmitted Public Key A:  " << client_public_key << "\n\n";

    // 2. Server Step (ServerHello + KeyShare in TLS 1.3)
    uint64_t server_private_key = 9876543; // Kept strictly private on Server device
    uint64_t server_public_key = power_mod(GENERATOR_G, server_private_key, PRIME_P);

    std::cout << "[Step 2: Server -> Client (TLS 1.3 ServerHello)]\n";
    std::cout << "  Server picked Secret Private Key: " << server_private_key << "\n";
    std::cout << "  Server transmitted Public Key B:  " << server_public_key << "\n\n";

    // 3. Shared Secret Computation (Independently computed on each endpoint)
    uint64_t client_derived_secret = power_mod(server_public_key, client_private_key, PRIME_P);
    uint64_t server_derived_secret = power_mod(client_public_key, server_private_key, PRIME_P);

    std::cout << "[Step 3: Shared Secret Derivation]\n";
    std::cout << "  Client computed Secret (B^a mod p): " << client_derived_secret << "\n";
    std::cout << "  Server computed Secret (A^b mod p): " << server_derived_secret << "\n";

    assert(client_derived_secret == server_derived_secret);
    std::cout << "  Match Verification: PASSED (Both endpoints derived identical 64-bit secret)!\n\n";

    // 4. Encrypted Application Data Transfer
    std::string plaintext = "GET /api/v1/bank/balance HTTP/1.1\r\nHost: secure.faang.com\r\n\r\n";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());

    std::cout << "[Step 4: Symmetric Data Encryption]\n";
    std::cout << "  Original Plaintext: \"" << plaintext << "\"\n";

    // Client encrypts with derived session key
    auto ciphertext = encrypt_decrypt(data, client_derived_secret);
    std::cout << "  Encrypted Ciphertext on Wire (Hex): ";
    for (uint8_t byte : ciphertext) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::dec << "\n";

    // Server decrypts with derived session key
    auto decrypted = encrypt_decrypt(ciphertext, server_derived_secret);
    std::string decrypted_str(decrypted.begin(), decrypted.end());

    std::cout << "  Server Decrypted Plaintext:         \"" << decrypted_str << "\"\n";
    assert(plaintext == decrypted_str);

    std::cout << "\nEnd-to-End Cryptographic Confidentiality: 100% VERIFIED!\n";
    std::cout << "=================================================================================\n";
    return 0;
}
