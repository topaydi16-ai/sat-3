#include <iostream>
#include <vector>
#include <bit>
#include <cstdint>
#include <iomanip>
#include <map>

inline uint16_t step_ring_16(uint16_t w_prev2, uint16_t w_prev1) {
    uint32_t val = 0;
    for (int step = 0; step < 48; ++step) {
        int t = step & 15;
        val = ((w_prev2 >> t) & 1) ^ (((w_prev1 >> t) & 1) | val);
    }
    uint16_t res = 0;
    for (int step = 0; step < 16; ++step) {
        int t = step & 15;
        val = ((w_prev2 >> t) & 1) ^ (((w_prev1 >> t) & 1) | val);
        if (val) res |= (1 << ((t + 1) & 15));
    }
    return res;
}

inline uint16_t integrate_16(uint16_t w_prev, int init_bit) {
    uint16_t res = 0;
    int curr = init_bit;
    if (curr) res |= 1;
    for (int t = 0; t < 15; ++t) {
        curr ^= (w_prev >> t) & 1;
        if (curr) res |= (1 << (t + 1));
    }
    return res;
}

inline uint32_t step_ring_32(uint32_t w_prev2, uint32_t w_prev1) {
    uint32_t val = 0;
    for (int step = 0; step < 96; ++step) {
        int t = step & 31;
        val = ((w_prev2 >> t) & 1) ^ (((w_prev1 >> t) & 1) | val);
    }
    uint32_t res = 0;
    for (int step = 0; step < 32; ++step) {
        int t = step & 31;
        val = ((w_prev2 >> t) & 1) ^ (((w_prev1 >> t) & 1) | val);
        if (val) res |= (1U << ((t + 1) & 31));
    }
    return res;
}

inline uint32_t integrate_32(uint32_t w_prev, int init_bit) {
    uint32_t res = 0;
    int curr = init_bit;
    if (curr) res |= 1U;
    for (int t = 0; t < 31; ++t) {
        curr ^= (w_prev >> t) & 1;
        if (curr) res |= (1U << (t + 1));
    }
    return res;
}

int main() {
    uint16_t d400_arr[16] = {0,0,0,0,0,1,1,0,1,1,1,1,1,0,0,1};
    uint16_t w_400 = 0;
    for (int i = 0; i < 16; ++i) if (d400_arr[i]) w_400 |= (1 << i);
    uint16_t w_401 = 0xFFFF;

    uint16_t p2 = w_400;
    uint16_t p1 = w_401;
    for (int j = 402; j <= 53207; ++j) {
        uint16_t w = step_ring_16(p2, p1);
        p2 = p1; p1 = w;
    }
    // Branch 0
    uint16_t d_53208 = integrate_16(p2, 0);
    p2 = 0; p1 = d_53208;
    for (int j = 53209; j <= 58286; ++j) {
        uint16_t w = step_ring_16(p2, p1);
        p2 = p1; p1 = w;
    }
    // Branch 01
    uint16_t d_58287 = integrate_16(p2, 1);
    p2 = 0; p1 = d_58287;
    for (int j = 58288; j <= 87866; ++j) {
        uint16_t w = step_ring_16(p2, p1);
        p2 = p1; p1 = w;
    }

    uint32_t w_32 = ((uint32_t)p2) | (((uint32_t)p2) << 16);
    
    std::vector<int> targets = { 100000, 150000, 200000, 250000 };

    for (int init_bit = 0; init_bit <= 1; ++init_bit) {
        uint32_t d_plus1 = integrate_32(w_32, init_bit);
        uint32_t d_plus2 = 0xFFFFFFFFU;
        uint32_t q2 = d_plus1;
        uint32_t q1 = d_plus2;

        std::cout << "=== Init Bit = " << init_bit << " ===\n";
        for (int j = 87869; j <= 250000; ++j) {
            uint32_t w = step_ring_32(q2, q1);
            for (int t : targets) {
                if (j == t) {
                    std::cout << "  j = " << std::setw(6) << j 
                              << " | Period = 32"
                              << " | Weight = " << std::setw(2) << std::popcount(w)
                              << " | Hex Word = 0x" << std::hex << std::setw(8) << std::setfill('0') << w << std::dec << std::setfill(' ')
                              << "\n";
                }
            }
            q2 = q1;
            q1 = w;
        }
    }

    return 0;
}
