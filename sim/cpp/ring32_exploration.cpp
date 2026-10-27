#include <iostream>
#include <vector>
#include <string>
#include <bit>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cstdint>
#include <map>

// Representation of cyclic word over R_4 (P = 16)
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

// Representation of cyclic word over R_5 (P = 32)
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

struct LeafInfo {
    std::string path;
    int leaf_j;
    uint16_t w_minus1; // period 16 word
};

std::vector<LeafInfo> all_leaves;
int internal_nodes = 0;
int leaf_nodes = 0;
int min_leaf_j = 1000000000;
int max_leaf_j = 0;

void explore_branch(int start_j, uint16_t w_m2, uint16_t w_m1, const std::string& path) {
    uint16_t prev2 = w_m2;
    uint16_t prev1 = w_m1;
    
    for (int j = start_j + 1; j < 2000000; ++j) {
        uint16_t w = step_ring_16(prev2, prev1);
        if (w == 0) {
            int weight = std::popcount(prev1);
            int parity = weight % 2;
            
            if (parity == 1) {
                leaf_nodes++;
                if (j < min_leaf_j) min_leaf_j = j;
                if (j > max_leaf_j) max_leaf_j = j;
                
                std::cout << "[LEAF " << std::setw(2) << leaf_nodes << "] Path " << std::setw(12) << path 
                          << " hits ODD zero diagonal at j = " << std::setw(6) << j 
                          << " (Weight = " << weight << ", Parity = 1) -> Period DOUBLES to 32!\n";
                          
                all_leaves.push_back({path, j, prev1});
                return;
            } else {
                internal_nodes++;
                uint16_t b0 = integrate_16(prev1, 0);
                uint16_t b1 = integrate_16(prev1, 1);
                
                explore_branch(j + 1, 0, b0, path + "0");
                explore_branch(j + 1, 0, b1, path + "1");
                return;
            }
        }
        prev2 = prev1;
        prev1 = w;
    }
}

int main() {
    std::cout << "=======================================================================\n";
    std::cout << "EXPEDITION 1 (FULL): Ring Branch Tree Solver & R_5 (P=32) Exploration\n";
    std::cout << "Author: Umidjon Rasulov | Inha University in Tashkent (SOCIE)\n";
    std::cout << "=======================================================================\n\n";

    auto t_start = std::chrono::high_resolution_clock::now();

    uint16_t w_400 = 0;
    int d400_arr[16] = {0,0,0,0,0,1,1,0,1,1,1,1,1,0,0,1};
    for (int i = 0; i < 16; ++i) {
        if (d400_arr[i]) w_400 |= (1 << i);
    }
    uint16_t w_401 = 0xFFFF;

    uint16_t prev2 = w_400;
    uint16_t prev1 = w_401;
    int z5 = -1;
    
    for (int j = 402; j < 100000; ++j) {
        uint16_t w = step_ring_16(prev2, prev1);
        if (w == 0) {
            z5 = j;
            break;
        }
        prev2 = prev1;
        prev1 = w;
    }

    std::cout << "Confirmed z_5 = " << z5 << ". Exploring complete binary tree...\n";
    internal_nodes = 1;
    uint16_t b0_53208 = integrate_16(prev1, 0);
    uint16_t b1_53208 = integrate_16(prev1, 1);

    explore_branch(53208, 0, b0_53208, "0");
    explore_branch(53208, 0, b1_53208, "1");

    std::cout << "\nFound " << all_leaves.size() << " leaves in P=16 tree.\n";
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << "Now launching P=32 (R_5) exploration from first 4 leaves (+500,000 steps)...\n";
    std::cout << "-----------------------------------------------------------------------\n";

    int p32_leaves_with_zero = 0;
    int count = 0;

    for (const auto& leaf : all_leaves) {
        count++;
        // test all 16 leaves
        uint32_t w_prev_32 = ((uint32_t)leaf.w_minus1) | (((uint32_t)leaf.w_minus1) << 16);
        
        uint32_t d_plus1 = integrate_32(w_prev_32, 0);
        uint32_t d_plus2 = 0xFFFFFFFFU;

        uint32_t p2 = d_plus1;
        uint32_t p1 = d_plus2;

        int found_z = -1;
        int max_search = leaf.leaf_j + 500000;
        for (int j = leaf.leaf_j + 3; j <= max_search; ++j) {
            uint32_t w = step_ring_32(p2, p1);
            if (w == 0) {
                found_z = j;
                int weight = std::popcount(p1);
                int parity = weight % 2;
                std::cout << "[P=32 HIT!] Leaf " << leaf.path << " (j=" << leaf.leaf_j << ") hits ZERO at j = " 
                          << found_z << " (Delta = " << found_z - leaf.leaf_j << "), Weight = " << weight 
                          << ", Parity = " << parity << " -> " << (parity ? "DOUBLES to 64!" : "SILENT (stays 32)") << "\n";
                p32_leaves_with_zero++;
                break;
            }
            p2 = p1;
            p1 = w;
        }

        if (found_z == -1) {
            std::cout << "[P=32 STABLE " << std::setw(2) << count << "/16] Leaf " << std::setw(12) << leaf.path 
                      << " (j=" << std::setw(6) << leaf.leaf_j << ") has NO zero diagonal in next 500,000 steps (P=32 persists to at least j = " 
                      << max_search << ")!\n";
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_s = std::chrono::duration<double>(t_end - t_start).count();

    std::cout << "\n=======================================================================\n";
    std::cout << "EXPEDITION 1 (R_5) COMPLETE:\n";
    std::cout << "Total Runtime: " << elapsed_s << " seconds\n";
    std::cout << "=======================================================================\n";

    return 0;
}
