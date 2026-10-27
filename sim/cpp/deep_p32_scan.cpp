#include <iostream>
#include <vector>
#include <string>
#include <bit>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cstdint>

inline uint16_t step_ring_16(uint16_t w_prev2, uint16_t w_prev1) {
    uint32_t val = 0;
    for (int step = 0; step < 48; ++step) {
        int t = step & 15;
        uint32_t b2 = (w_prev2 >> t) & 1;
        uint32_t b1 = (w_prev1 >> t) & 1;
        val = b2 ^ (b1 | val);
    }
    uint16_t res = 0;
    for (int step = 0; step < 16; ++step) {
        int t = step & 15;
        uint32_t b2 = (w_prev2 >> t) & 1;
        uint32_t b1 = (w_prev1 >> t) & 1;
        val = b2 ^ (b1 | val);
        int next_t = (t + 1) & 15;
        if (val) res |= (1 << next_t);
    }
    return res;
}

inline uint16_t integrate_16(uint16_t w_prev, int init_bit) {
    uint16_t res = 0;
    int curr = init_bit;
    if (curr) res |= 1;
    for (int t = 0; t < 15; ++t) {
        int b = (w_prev >> t) & 1;
        curr ^= b;
        if (curr) res |= (1 << (t + 1));
    }
    return res;
}

inline uint32_t step_ring_32(uint32_t w_prev2, uint32_t w_prev1) {
    uint32_t val = 0;
    for (int step = 0; step < 96; ++step) {
        int t = step & 31;
        uint32_t b2 = (w_prev2 >> t) & 1;
        uint32_t b1 = (w_prev1 >> t) & 1;
        val = b2 ^ (b1 | val);
    }
    uint32_t res = 0;
    for (int step = 0; step < 32; ++step) {
        int t = step & 31;
        uint32_t b2 = (w_prev2 >> t) & 1;
        uint32_t b1 = (w_prev1 >> t) & 1;
        val = b2 ^ (b1 | val);
        int next_t = (t + 1) & 31;
        if (val) res |= (1U << next_t);
    }
    return res;
}

inline uint32_t integrate_32(uint32_t w_prev, int init_bit) {
    uint32_t res = 0;
    int curr = init_bit;
    if (curr) res |= 1U;
    for (int t = 0; t < 31; ++t) {
        int b = (w_prev >> t) & 1;
        curr ^= b;
        if (curr) res |= (1U << (t + 1));
    }
    return res;
}

int main() {
    auto t_start = std::chrono::high_resolution_clock::now();
    uint16_t w_400 = 0;
    int d400_arr[16] = {0,0,0,0,0,1,1,0,1,1,1,1,1,0,0,1};
    for (int i = 0; i < 16; ++i) if (d400_arr[i]) w_400 |= (1 << i);
    uint16_t w_401 = 0xFFFF;

    uint16_t prev2 = w_400;
    uint16_t prev1 = w_401;
    for (int j = 402; j <= 53207; ++j) {
        uint16_t w = step_ring_16(prev2, prev1);
        prev2 = prev1;
        prev1 = w;
    }
    // Now at 53207, prev1 = D_53207 = 0. prev2 = D_53206.
    // Branch "0": D_53208 = integrate_16(D_53206, 0)
    uint16_t d_53208 = integrate_16(prev2, 0);
    prev2 = 0;
    prev1 = d_53208;
    for (int j = 53209; j <= 58286; ++j) {
        uint16_t w = step_ring_16(prev2, prev1);
        prev2 = prev1;
        prev1 = w;
    }
    // Now at 58286, D_58286 = 0.
    // Branch "01": D_58287 = integrate_16(prev2, 1)
    uint16_t d_58287 = integrate_16(prev2, 1);
    prev2 = 0;
    prev1 = d_58287;
    for (int j = 58288; j <= 87866; ++j) {
        uint16_t w = step_ring_16(prev2, prev1);
        prev2 = prev1;
        prev1 = w;
    }
    // Now at 87866: Leaf "01"!
    // prev1 = D_87866 = 0. prev2 = D_87865 (weight 5).
    std::cout << "Reached Leaf 01 at j = 87866. Weight of D_87865 = " << std::popcount(prev2) << "\n";
    
    // Launch deep R_5 simulation up to +5,000,000 steps!
    uint32_t w_prev_32 = ((uint32_t)prev2) | (((uint32_t)prev2) << 16);
    uint32_t p2 = integrate_32(w_prev_32, 0);
    uint32_t p1 = 0xFFFFFFFFU;

    int max_steps = 5000000;
    int found_z = -1;
    for (int j = 87869; j <= 87869 + max_steps; ++j) {
        uint32_t w = step_ring_32(p2, p1);
        if (w == 0) {
            found_z = j;
            int weight = std::popcount(p1);
            int parity = weight % 2;
            std::cout << ">>> WOW! P=32 ZERO DIAGONAL FOUND AT j = " << found_z 
                      << " (Delta = " << found_z - 87866 << "), Weight = " << weight 
                      << ", Parity = " << parity << " (" << (parity ? "ODD -> Period DOUBLES to 64!" : "EVEN -> Silent bifurcation") << ")!\n";
            break;
        }
        p2 = p1;
        p1 = w;
    }

    if (found_z == -1) {
        std::cout << "Deep scan: No zero diagonal found up to j = " << 87869 + max_steps 
                  << " (" << max_steps << " steps tested on R_5)!\n";
        std::cout << "The super-exponential plateau length L(P) scales as L(32) > 5 * 10^6!\n";
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_s = std::chrono::duration<double>(t_end - t_start).count();
    std::cout << "Scan finished in " << elapsed_s << " s.\n";
    return 0;
}
