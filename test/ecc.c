#include <test/base.h>
#include <qr/types.h>
#include <stdio.h>

// Include the source file directly to test static functions
#include "../qr/ecc.c"
#include "../qr/qr.c"

/**
 * Initialize the test environment.
 * Sets up the Galois Field log/antilog tables.
 */
BEFORE() {
    gf_init_log_antilog();
    return 0;
}

/**
 * Test basic Galois Field (2^8) arithmetic operations.
 * Verifies multiplication, addition, and log/antilog table lookups.
 */
TEST(gf_arithmetic) {
    // Test multiplication
    if (gf_mul(2, 3) != 6) return 1;     // 2 * 3 = 6
    if (gf_mul(0, 5) != 0) return 2;     // 0 * 5 = 0
    if (gf_mul(7, 1) != 7) return 3;     // 7 * 1 = 7
    
    // Test addition (XOR in GF)
    if (gf_add(5, 3) != 6) return 4;     // 5 + 3 = 6 (XOR)
    if (gf_add(0, 4) != 4) return 5;     // 0 + 4 = 4
    
    // Test multiplication with log/antilog tables
    if (gf_mul(0x03, 0x0E) != 18) return 6;  // Example from QR spec
    if (gf_mul(0x1A, 0x0B) != 254) return 7; // Example from QR spec
    
    return 0;
}

/**
 * Test the generator polynomial creation.
 * Verifies that the generated polynomials match expected values from the QR spec.
 */
TEST(generator_polynomial) {
    word poly[30];  // Large enough for testing
    
    // Test case 1: Degree 5 generator polynomial (6 coefficients)
    // g(x) = (x-α^0)(x-α^1)(x-α^2)(x-α^3)(x-α^4)
    generator_polynomial(poly, 5);
    
    // Expected exponents for the antilog table
    word expected5_exponents[6] = {0, 113, 164, 166, 119, 10};
    
    for (int i = 0; i <= 5; i++) {
        if (poly[i] != gf_antilog[expected5_exponents[i]]) {
            return 1;  // Coefficient mismatch
        }
    }
    
    // Test case 2: Degree 16 generator polynomial (17 coefficients)
    // Used in version 1-M QR codes
    generator_polynomial(poly, 16);
    
    word expected16_exponents[17] = {
        0, 120, 104, 107, 109, 102, 161, 76, 3, 91,
        191, 147, 169, 182, 194, 225, 120
    };
    
    for (int i = 0; i <= 16; i++) {
        if (poly[i] != gf_antilog[expected16_exponents[i]]) {
            return 2;  // Coefficient mismatch
        }
    }
    
    return 0;
}

/**
 * Test ECC generation for a simple case.
 * Verifies that the generated error correction codes match expected values.
 */
TEST(ecc_generation) {
    // Simple test case: Version 1-L (7 data codewords, 10 ECC codewords)
    word data[7] = {40, 88, 12, 6, 46, 77, 36};
    word ecc[10] = {0};
    word g[10 + 1] = {0};  // +1 for leading coefficient
    
    // Generate the generator polynomial for 10 ECC codewords
    generator_polynomial(g, 10);
    
    // Generate ECC (skip the leading coefficient in g)
    ecc_generate(data, 7, ecc, 10, g + 1);
    
    // Expected ECC values for the test data
    word expected_ecc[10] = {214, 246, 18, 193, 38, 69, 160, 197, 199, 15};
    
    // Compare generated ECC with expected values
    for (int i = 0; i < 10; i++) {
        if (ecc[i] != expected_ecc[i]) {
            return i + 1;  // Return position + 1 as error code
        }
    }
    
    return 0;
}

/**
 * Test the consistency of error correction tables.
 * Verifies that:
 * 1. When BLOCK_COUNT is 0, both TOTAL_CODEWORD_COUNT and DATA_CODEWORD_COUNT are 0
 * 2. BLOCK_COUNT * TOTAL_CODEWORD_COUNT matches the total codeword count
 * 3. The difference between TOTAL_CODEWORD_COUNT and DATA_CODEWORD_COUNT is consistent
 */
TEST(ecc_table_consistency) {
    for (int level = 0; level < QR_EC_LEVEL_COUNT; level++) {
        for (int version = 0; version < QR_VERSION_COUNT; version++) {
            size_t total_blocks = 0;
            size_t total_codewords = 0;
            size_t total_data_codewords = 0;
            
            // Check each block type
            for (int block_type = 0; block_type < BLOCK_TYPES_PER_VERSION; block_type++) {
                size_t block_count = BLOCK_COUNT[level][version][block_type];
                size_t total_cw = TOTAL_CODEWORD_COUNT[level][version][block_type];
                size_t data_cw = DATA_CODEWORD_COUNT[level][version][block_type];
                
                // Test 1: If block_count is 0, both total_cw and data_cw should be 0
                if (block_count == 0) {
                    if (total_cw != 0 || data_cw != 0) {
                        return 10000 + (level * 1000) + (version * 10) + block_type;
                    }
                    continue;
                }
                
                // Test 2: total_cw should be >= data_cw
                if (total_cw < data_cw) {
                    return 20000 + (level * 1000) + (version * 10) + block_type;
                }
                
                total_blocks += block_count;
                total_codewords += block_count * total_cw;
                total_data_codewords += block_count * data_cw;
            }
            
            // Test 3: Total data codewords should match the precomputed value
            if (total_data_codewords != TOTAL_DATA_CODEWORD_COUNT[level][version]) {
                return 30000 + (level * 1000) + (version * 10);
            }
            
            // Test 4: Total codewords should match the version's capacity
            if (total_codewords != CODEWORD_COUNT[version]) {
                return 40000 + (level * 1000) + (version * 10);
            }
        }
    }
    
    return 0;
}
