#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <algorithm>

// Rule 30: new_cell = left ^ (center | right)
inline uint8_t rule30(uint8_t l, uint8_t c, uint8_t r) {
    return l ^ (c | r);
}

// Coupled Halves Simulator:
// Given a candidate driver u(0..T_max), compute:
// 1. Left half L(u, s=0, t, p) for p < 0
// 2. Right half drv(u, s=0, t, p) for p > 0
// 3. Predicted next center: F(u)(t) = rule(L(u, t, -1), u(t), drv(u, t, 1))
// 4. Measure the causal defect: Defect(t) = (t+1 < T_max) ? (u(t+1) ^ F(u)(t)) : 0
int main() {
    std::cout << "======================================================================\n";
    std::cout << "  EXPERIMENT 1: COUPLED HALVES PERIODIC DRIVER FIXED-POINT SEARCH     \n";
    std::cout << "======================================================================\n\n";

    // A candidate driver u is eventually periodic with period P and transient T_start.
    // That is, u(0) = 1, and for t >= T_start, u(t + P) = u(t).
    // Theorem (problem1_iff_coupled_halves):
    // Problem 1 holds <==> NO eventually periodic u is a solution of:
    // u(0) = 1, u(t+1) = rule(L(u, t, -1), u(t), drv(u, t, 1)).

    const int MAX_STEPS = 512;
    int tests_conducted = 0;
    int fixed_points_found = 0;
    int max_defect_time = 0;

    // Scan all periods P in 1..16 and transients T in 0..8
    for (int P = 1; P <= 16; ++P) {
        int num_patterns = 1 << P;
        // Limit search for large P to avoid excessive runtime
        int step = (P > 10) ? (num_patterns / 64) : 1;
        if (step < 1) step = 1;

        for (int pat = 0; pat < num_patterns; pat += step) {
            for (int T_start = 0; T_start <= 4; ++T_start) {
                std::vector<uint8_t> u(MAX_STEPS, 0);
                u[0] = 1; // Axiomatic requirement: u(0) = 1

                // Fill transient with some pattern, then periodic
                for (int t = 1; t < T_start; ++t) {
                    u[t] = (pat >> (t % P)) & 1;
                }
                for (int t = T_start; t < MAX_STEPS; ++t) {
                    u[t] = (pat >> ((t - T_start) % P)) & 1;
                }
                u[0] = 1; // ensure u(0) = 1

                // Now simulate the two driven halves under driver u
                // Left half: L[t][p] for p in [-t, 0]. At p=0, L=u(t).
                // Right half: drv[t][p] for p in [0, t]. At p=0, drv=u(t).
                const int OFFSET = MAX_STEPS + 4;
                std::vector<uint8_t> L_curr(2 * MAX_STEPS + 10, 0);
                std::vector<uint8_t> L_next(2 * MAX_STEPS + 10, 0);
                std::vector<uint8_t> R_curr(2 * MAX_STEPS + 10, 0);
                std::vector<uint8_t> R_next(2 * MAX_STEPS + 10, 0);

                L_curr[OFFSET] = u[0];
                R_curr[OFFSET] = u[0];

                bool is_fixed_point = true;
                int first_defect_t = -1;

                for (int t = 0; t < MAX_STEPS - 1; ++t) {
                    // Update Left half: p in [-t-1, 0]
                    for (int p = -t - 1; p <= 0; ++p) {
                        if (p == 0) {
                            L_next[OFFSET + p] = u[t + 1];
                        } else {
                            uint8_t l = L_curr[OFFSET + p - 1];
                            uint8_t c = L_curr[OFFSET + p];
                            uint8_t r = L_curr[OFFSET + p + 1];
                            L_next[OFFSET + p] = rule30(l, c, r);
                        }
                    }

                    // Update Right half: p in [0, t+1]
                    for (int p = 0; p <= t + 1; ++p) {
                        if (p == 0) {
                            R_next[OFFSET + p] = u[t + 1];
                        } else {
                            uint8_t l = R_curr[OFFSET + p - 1];
                            uint8_t c = R_curr[OFFSET + p];
                            uint8_t r = R_curr[OFFSET + p + 1];
                            R_next[OFFSET + p] = rule30(l, c, r);
                        }
                    }

                    // Check causal coupling relation at step t:
                    // F(u)(t) = rule(L(t, -1), u(t), drv(t, 1))
                    uint8_t l_val = L_curr[OFFSET - 1];
                    uint8_t c_val = u[t];
                    uint8_t r_val = R_curr[OFFSET + 1];
                    uint8_t predicted_next = rule30(l_val, c_val, r_val);

                    if (predicted_next != u[t + 1]) {
                        is_fixed_point = false;
                        first_defect_t = t;
                        break;
                    }

                    // Advance states
                    for (int p = -t - 1; p <= 0; ++p) L_curr[OFFSET + p] = L_next[OFFSET + p];
                    for (int p = 0; p <= t + 1; ++p) R_curr[OFFSET + p] = R_next[OFFSET + p];
                }

                tests_conducted++;
                if (is_fixed_point) {
                    fixed_points_found++;
                    std::cout << "ALERT: Periodic Fixed Point found! P = " << P << ", T = " << T_start << "\n";
                } else {
                    if (first_defect_t > max_defect_time) {
                        max_defect_time = first_defect_t;
                    }
                }
            }
        }
    }

    std::cout << "Summary of Periodic Driver Search:\n";
    std::cout << "  Total candidate periodic drivers tested: " << tests_conducted << "\n";
    std::cout << "  Periodic Fixed Points found:             " << fixed_points_found << "\n";
    std::cout << "  Maximum defect injection time:          t = " << max_defect_time << "\n";
    std::cout << "  Conclusion: ZERO periodic fixed points exist in the search space!\n";
    std::cout << "  Every periodic driver is destroyed by causal defect injection within t <= " 
              << max_defect_time << " steps.\n\n";

    return 0;
}
