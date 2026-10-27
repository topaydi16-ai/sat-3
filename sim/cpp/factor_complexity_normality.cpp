#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <unordered_set>
#include <algorithm>

int main() {
    const int N = 262144; // 2^18 = 262,144 steps
    std::vector<uint8_t> prev(2 * N + 3, 0);
    std::vector<uint8_t> curr(2 * N + 3, 0);
    std::vector<uint8_t> center(N + 1, 0);

    const int origin = N + 1;
    prev[origin] = 1;
    center[0] = 1;

    for (int t = 1; t <= N; ++t) {
        for (int x = origin - t; x <= origin + t; ++x) {
            uint8_t p = prev[x - 1];
            uint8_t q_val = prev[x];
            uint8_t r = prev[x + 1];
            curr[x] = p ^ (q_val | r);
        }
        center[t] = curr[origin];
        prev = curr;
    }

    std::cout << "========================================================\n";
    std::cout << "FACTOR COMPLEXITY & BOREL NORMALITY (N = " << N << ")\n";
    std::cout << "Testing Morse-Hedlund, Disjunctivity, and Block Entropy\n";
    std::cout << "========================================================\n";

    // 1. Measure p(k) for k = 1 to 18
    std::cout << std::setw(4) << "k"
              << std::setw(12) << "Observed p(k)"
              << std::setw(14) << "Max 2^k"
              << std::setw(16) << "Completeness %"
              << std::setw(16) << "Morse-Hedlund > k"
              << "\n";
    std::cout << "--------------------------------------------------------\n";

    for (int k = 1; k <= 18; ++k) {
        std::vector<uint8_t> seen(1 << k, 0);
        int unique_count = 0;
        uint32_t mask = (1 << k) - 1;
        uint32_t current_window = 0;

        for (int i = 0; i < k && i < N; ++i) {
            current_window = (current_window << 1) | center[i];
        }
        if (N >= k) {
            seen[current_window] = 1;
            unique_count = 1;
        }

        for (int i = k; i < N; ++i) {
            current_window = ((current_window << 1) & mask) | center[i];
            if (!seen[current_window]) {
                seen[current_window] = 1;
                unique_count++;
            }
        }

        int max_possible = 1 << k;
        double completeness = 100.0 * unique_count / max_possible;

        std::cout << std::setw(4) << k
                  << std::setw(12) << unique_count
                  << std::setw(14) << max_possible
                  << std::setw(15) << std::fixed << std::setprecision(2) << completeness << "%"
                  << std::setw(14) << (unique_count > k ? "PASS (APERIODIC)" : "FAIL")
                  << "\n";
    }

    // 2. Measure block frequency distribution for k = 1, 2, 3, 4
    std::cout << "\n========================================================\n";
    std::cout << "BLOCK FREQUENCIES FOR k = 1, 2, 3, 4 (BOREL NORMALITY)\n";
    std::cout << "========================================================\n";

    for (int k = 1; k <= 4; ++k) {
        int num_blocks = 1 << k;
        std::vector<int> counts(num_blocks, 0);
        uint32_t mask = (1 << k) - 1;
        uint32_t current_window = 0;

        for (int i = 0; i < k; ++i) {
            current_window = (current_window << 1) | center[i];
        }
        counts[current_window]++;

        for (int i = k; i < N; ++i) {
            current_window = ((current_window << 1) & mask) | center[i];
            counts[current_window]++;
        }

        int total_windows = N - k + 1;
        double expected_freq = 1.0 / num_blocks;
        double max_dev = 0.0;
        double chi2 = 0.0;

        std::cout << "--- Word Length k = " << k << " (Expected frequency: " 
                  << std::fixed << std::setprecision(5) << expected_freq << ") ---\n";

        for (int w = 0; w < num_blocks; ++w) {
            double freq = (double)counts[w] / total_windows;
            double dev = std::abs(freq - expected_freq);
            if (dev > max_dev) max_dev = dev;

            double expected_count = (double)total_windows / num_blocks;
            chi2 += (counts[w] - expected_count) * (counts[w] - expected_count) / expected_count;

            std::string binary_str = "";
            for (int b = k - 1; b >= 0; --b) {
                binary_str += ((w >> b) & 1) ? '1' : '0';
            }

            if (k <= 3 || w < 8) {
                std::cout << "  Word [" << binary_str << "]: count = " << std::setw(6) << counts[w]
                          << " | freq = " << std::setprecision(5) << freq
                          << " | dev = " << std::showpos << (freq - expected_freq) << std::noshowpos
                          << "\n";
            }
        }
        if (k == 4) std::cout << "  ... (8 remaining words omitted for brevity)\n";

        std::cout << "  Summary for k=" << k << ": Max Deviation = " << std::fixed << std::setprecision(5) << max_dev
                  << " | Chi2 = " << std::setprecision(2) << chi2 
                  << " (dof = " << (num_blocks - 1) << ")\n\n";
    }

    return 0;
}
