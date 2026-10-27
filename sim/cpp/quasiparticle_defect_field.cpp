#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <algorithm>

// Quasiparticle / Defect Field Mapper for Left Flank
// Computes delta_j(t) = D_j(t) ^ W_j(t mod P_j)
// and determines the exact freezing curve t_freeze(j) for j in [0, 410]

int main() {
    std::cout << "=======================================================================\n";
    std::cout << "EXPEDITION 4: Left-Flank Quasiparticle Defect Field & Freezing Dynamics\n";
    std::cout << "Author: Umidjon Rasulov | Inha University in Tashkent (SOCIE)\n";
    std::cout << "=======================================================================\n\n";

    int max_j = 410;
    int max_t = 1200; // Sufficient to capture all transients up to j = 410 (z_4 settles at t=501)

    // Compute 2D grid c(t, x) for x in [-t, t]
    // D_j(t) = c(t, j - t)
    // Nonzero cells require j - t >= -t => j >= 0
    // and j - t <= t => t >= j / 2.
    std::vector<std::vector<uint8_t>> grid(max_t, std::vector<uint8_t>(2 * max_t + 1, 0));
    int origin = max_t;
    grid[0][origin] = 1;

    for (int t = 0; t < max_t - 1; ++t) {
        for (int x = -t - 1; x <= t + 1; ++x) {
            uint8_t l = grid[t][origin + x - 1];
            uint8_t m = grid[t][origin + x];
            uint8_t r = grid[t][origin + x + 1];
            grid[t + 1][origin + x] = l ^ (m | r);
        }
    }

    // Extract D_j(t) = c(t, j - t)
    std::vector<std::vector<uint8_t>> D(max_j + 1, std::vector<uint8_t>(max_t, 0));
    for (int j = 0; j <= max_j; ++j) {
        for (int t = 0; t < max_t; ++t) {
            int x = j - t;
            if (x >= -t && x <= t) {
                D[j][t] = grid[t][origin + x];
            } else {
                D[j][t] = 0;
            }
        }
    }

    // For each diagonal j, determine minimal period P and attractor word W_j
    std::vector<int> period(max_j + 1, 1);
    std::vector<int> t_freeze(max_j + 1, 0);
    std::vector<int> defect_count(max_j + 1, 0);

    // Let's test periods P in {1, 2, 4, 8, 16}
    int candidate_periods[] = {1, 2, 4, 8, 16};

    for (int j = 0; j <= max_j; ++j) {
        int best_P = 16;
        for (int P : candidate_periods) {
            // Check if tail is periodic with period P from some T
            bool periodic = true;
            for (int t = max_t - 2 * P - 1; t >= max_t - 100; --t) {
                if (D[j][t + P] != D[j][t]) {
                    periodic = false;
                    break;
                }
            }
            if (periodic) {
                best_P = P;
                break;
            }
        }
        period[j] = best_P;

        // Extract asymptotic attractor word W_j of length P
        std::vector<uint8_t> W(best_P);
        int tail_start = max_t - best_P;
        for (int r = 0; r < best_P; ++r) {
            W[r] = D[j][tail_start + r];
        }

        // Find t_freeze: largest t such that D[j][t] != W[t % P]
        // Starting after the diagonal enters the light cone (t >= ceil(j/2))
        int start_t = (j + 1) / 2;
        int last_defect = start_t;
        int d_count = 0;

        for (int t = start_t; t < max_t; ++t) {
            uint8_t expected = W[t % best_P];
            if (D[j][t] != expected) {
                last_defect = t + 1; // frozen for all t >= last_defect
                d_count++;
            }
        }
        t_freeze[j] = last_defect;
        defect_count[j] = d_count;
    }

    std::cout << "Special Zero Diagonal Transient Horizons:\n";
    std::cout << "  z_1 =   2: t_freeze = " << t_freeze[2] << " (Period = " << period[2] << ")\n";
    std::cout << "  z_2 =   7: t_freeze = " << t_freeze[7] << " (Period = " << period[7] << ")\n";
    std::cout << "  z_3 =  28: t_freeze = " << t_freeze[28] << " (Period = " << period[28] << ")\n";
    std::cout << "  z_4 = 399: t_freeze = " << t_freeze[399] << " (Period = " << period[399] << ")\n\n";

    // Fit linear scaling law: t_freeze(j) ~ alpha * j + beta
    double sum_j = 0, sum_t = 0, sum_jj = 0, sum_jt = 0;
    int n_fit = 0;
    for (int j = 10; j <= max_j; ++j) {
        if (t_freeze[j] > 0) {
            sum_j += j;
            sum_t += t_freeze[j];
            sum_jj += (double)j * j;
            sum_jt += (double)j * t_freeze[j];
            n_fit++;
        }
    }
    double slope = (n_fit * sum_jt - sum_j * sum_t) / (n_fit * sum_jj - sum_j * sum_j);
    double intercept = (sum_t - slope * sum_j) / n_fit;

    std::cout << "Fitted Freezing Boundary Curve (j in [10, 410]):\n";
    std::cout << "  t_freeze(j) = " << std::fixed << std::setprecision(4) << slope << " * j + " 
              << intercept << " (Linear Transient Envelope!)\n\n";

    // Write JSON output
    std::ofstream ofs("data/quasiparticle_defect_field.json");
    ofs << "{\n";
    ofs << "  \"author\": \"Umidjon Rasulov\",\n";
    ofs << "  \"affiliation\": \"Inha University in Tashkent (SOCIE)\",\n";
    ofs << "  \"slope\": " << slope << ",\n";
    ofs << "  \"intercept\": " << intercept << ",\n";
    ofs << "  \"z1_t_freeze\": " << t_freeze[2] << ",\n";
    ofs << "  \"z2_t_freeze\": " << t_freeze[7] << ",\n";
    ofs << "  \"z3_t_freeze\": " << t_freeze[28] << ",\n";
    ofs << "  \"z4_t_freeze\": " << t_freeze[399] << ",\n";
    ofs << "  \"diagonals\": [\n";
    for (int j = 0; j <= max_j; ++j) {
        ofs << "    {\"j\": " << j << ", \"P\": " << period[j] 
            << ", \"t_freeze\": " << t_freeze[j] << ", \"defects\": " << defect_count[j] << "}"
            << (j < max_j ? "," : "") << "\n";
    }
    ofs << "  ]\n";
    ofs << "}\n";
    ofs.close();

    std::cout << "Saved results to data/quasiparticle_defect_field.json\n";
    return 0;
}
