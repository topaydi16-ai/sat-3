#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <chrono>

// Exact Green's Function Volterra Decomposition of Rule 30
// c(t, 0) = 1 ^ C(t)
// where C(t) = XOR_{s=0}^{t-1} XOR_i q(s, i) * trinomial(t - 1 - s, i)

int main() {
    const int T_MAX = 2000;
    std::cout << "========================================================================\n";
    std::cout << "  RULE 30 VOLTERRA-GREEN OPERATOR DECOMPOSITION ENGINE                  \n";
    std::cout << "  Superposition of Rule 150 Background & Non-Linear Carry Point Sources  \n";
    std::cout << "========================================================================\n\n";

    // 1. Simulate Rule 30 to collect exact grid
    std::vector<std::vector<uint8_t>> grid(T_MAX);
    for (int t = 0; t < T_MAX; ++t) {
        grid[t].resize(2 * t + 3, 0); // x in [-t, t], offset is t + 1
    }
    grid[0][1] = 1; // t=0, x=0 is index 1

    for (int t = 0; t < T_MAX - 1; ++t) {
        for (int x = -(t + 1); x <= (t + 1); ++x) {
            int prev_offset = t + 1;
            uint8_t l = (x - 1 < -t || x - 1 > t) ? 0 : grid[t][prev_offset + x - 1];
            uint8_t m = (x < -t || x > t) ? 0 : grid[t][prev_offset + x];
            uint8_t r = (x + 1 < -t || x + 1 > t) ? 0 : grid[t][prev_offset + x + 1];
            grid[t + 1][(t + 2) + x] = l ^ (m | r);
        }
    }

    // 2. Precompute Rule 150 trinomial coefficients: K(tau, i) = coeff of x^i in (x^-1 + 1 + x)^tau mod 2
    // For tau in [0, T_MAX], i in [-tau, tau]
    std::vector<std::vector<uint8_t>> K(T_MAX);
    for (int tau = 0; tau < T_MAX; ++tau) {
        K[tau].resize(2 * tau + 3, 0); // offset is tau + 1
    }
    K[0][1] = 1; // tau=0, i=0 is 1

    for (int tau = 0; tau < T_MAX - 1; ++tau) {
        for (int i = -(tau + 1); i <= (tau + 1); ++i) {
            int prev_offset = tau + 1;
            uint8_t l = (i - 1 < -tau || i - 1 > tau) ? 0 : K[tau][prev_offset + i - 1];
            uint8_t m = (i < -tau || i > tau) ? 0 : K[tau][prev_offset + i];
            uint8_t r = (i + 1 < -tau || i + 1 > tau) ? 0 : K[tau][prev_offset + i + 1];
            K[tau + 1][(tau + 2) + i] = l ^ m ^ r;
        }
    }

    std::cout << "[Step 1] Verifying Volterra-Green Theorem on first 100 steps of Rule 30...\n";
    int mismatches = 0;
    for (int t = 1; t <= 100; ++t) {
        uint8_t carry_flux = 0;
        for (int s = 0; s < t; ++s) {
            int tau = t - 1 - s;
            int s_offset = s + 1;
            int tau_offset = tau + 1;
            for (int i = -s; i < s; ++i) {
                uint8_t q = grid[s][s_offset + i] & grid[s][s_offset + i + 1];
                if (q) {
                    if (-i >= -tau && -i <= tau) {
                        carry_flux ^= K[tau][tau_offset - i];
                    }
                }
            }
        }
        uint8_t predicted = 1 ^ carry_flux;
        uint8_t actual = grid[t][t + 1];
        if (predicted != actual) {
            mismatches++;
            std::cerr << "Mismatch at t=" << t << ": actual=" << (int)actual << ", predicted=" << (int)predicted << "\n";
        }
    }

    if (mismatches == 0) {
        std::cout << ">> VERIFICATION SUCCESSFUL: Volterra-Green Theorem holds identically across all steps (0 mismatches)!\n\n";
    }

    // 3. Statistical & Algebraic analysis of Carry Sources q(s, i)
    std::cout << "[Step 2] Measuring Carry Source Field Density & Distribution (q(s, i) = c(s, i) * c(s, i+1))\n";
    std::cout << std::setw(8) << "Step t" 
              << std::setw(16) << "Width 2t+1"
              << std::setw(16) << "Black Cells" 
              << std::setw(18) << "Carry Pairs q=1" 
              << std::setw(16) << "Pair Density" << "\n";
    std::cout << std::string(74, '-') << "\n";

    for (int t : {10, 25, 50, 100, 250, 500, 1000}) {
        int black_cells = 0;
        int carry_pairs = 0;
        int offset = t + 1;
        for (int x = -t; x <= t; ++x) {
            if (grid[t][offset + x]) black_cells++;
            if (x < t && grid[t][offset + x] && grid[t][offset + x + 1]) carry_pairs++;
        }
        double density = (double)carry_pairs / (2 * t);
        std::cout << std::setw(8) << t
                  << std::setw(16) << (2 * t + 1)
                  << std::setw(16) << black_cells
                  << std::setw(18) << carry_pairs
                  << std::setw(15) << std::fixed << std::setprecision(4) << density << "\n";
    }

    std::cout << "\n>> Volterra Operator analysis complete.\n";
    return 0;
}
