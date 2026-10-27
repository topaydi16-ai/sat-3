#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <algorithm>

int main() {
    const int N = 262144; // 2^18 = 262,144 steps!
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
    std::cout << "RANDOM WALK & DISCREPANCY TRAJECTORY (N = " << N << ")\n";
    std::cout << "Testing Khinchin LIL and Oscillating Discrepancy\n";
    std::cout << "========================================================\n";

    int ones = 0;
    int max_W = 0;
    int min_W = 0;
    int zero_crossings = 0;
    int prev_sign = 0;

    std::vector<std::pair<int, int>> positive_records;
    std::vector<std::pair<int, int>> negative_records;

    for (int n = 1; n <= N; ++n) {
        if (center[n - 1] == 1) ones++;
        int W = 2 * ones - n; // Black (+1) minus White (-1)

        if (W > max_W) {
            max_W = W;
            positive_records.push_back({n, W});
        }
        if (W < min_W) {
            min_W = W;
            negative_records.push_back({n, W});
        }

        int sign = (W > 0) ? 1 : (W < 0) ? -1 : 0;
        if (sign != 0 && prev_sign != 0 && sign != prev_sign) {
            zero_crossings++;
        }
        if (sign != 0) prev_sign = sign;

        // Print at power-of-two milestones
        if ((n & (n - 1)) == 0 && n >= 256) {
            double log_log = std::log(std::log((double)n));
            double lil_denom = (log_log > 0) ? std::sqrt(2.0 * n * log_log) : 1.0;
            double lil_ratio = (double)W / lil_denom;
            double density = (double)ones / n;

            std::cout << "n = " << std::setw(7) << n
                      << " | ones = " << std::setw(7) << ones
                      << " | W(n) = " << std::setw(5) << std::showpos << W << std::noshowpos
                      << " | D(n) = " << std::setw(6) << std::fixed << std::setprecision(1) << (W / 2.0)
                      << " | density = " << std::setprecision(5) << density
                      << " | W/LIL = " << std::setprecision(3) << std::showpos << lil_ratio << std::noshowpos
                      << " | Max W = " << std::setw(4) << max_W
                      << " | Min W = " << std::setw(4) << min_W
                      << "\n";
        }
    }

    std::cout << "--------------------------------------------------------\n";
    std::cout << "SUMMARY ACROSS " << N << " STEPS:\n";
    std::cout << "Final Walk W(N): " << (2 * ones - N) << "\n";
    std::cout << "Final Density: " << std::fixed << std::setprecision(6) << ((double)ones / N) << "\n";
    std::cout << "Max Positive Walk Record: +" << max_W << " (at n = " << positive_records.back().first << ")\n";
    std::cout << "Min Negative Walk Record: " << min_W << " (at n = " << negative_records.back().first << ")\n";
    std::cout << "Total Zero Crossings: " << zero_crossings << "\n";
    std::cout << "Positive Records Count: " << positive_records.size() << "\n";
    std::cout << "Negative Records Count: " << negative_records.size() << "\n";

    std::cout << "\nPositive Records progression (first 10 and last 5):\n";
    for (size_t i = 0; i < std::min((size_t)10, positive_records.size()); ++i) {
        std::cout << "  n = " << std::setw(7) << positive_records[i].first << " -> W = +" << positive_records[i].second << "\n";
    }
    std::cout << "  ...\n";
    for (size_t i = positive_records.size() > 5 ? positive_records.size() - 5 : 0; i < positive_records.size(); ++i) {
        std::cout << "  n = " << std::setw(7) << positive_records[i].first << " -> W = +" << positive_records[i].second << "\n";
    }

    std::cout << "\nNegative Records progression (first 10 and last 5):\n";
    for (size_t i = 0; i < std::min((size_t)10, negative_records.size()); ++i) {
        std::cout << "  n = " << std::setw(7) << negative_records[i].first << " -> W = " << negative_records[i].second << "\n";
    }
    std::cout << "  ...\n";
    for (size_t i = negative_records.size() > 5 ? negative_records.size() - 5 : 0; i < negative_records.size(); ++i) {
        std::cout << "  n = " << std::setw(7) << negative_records[i].first << " -> W = " << negative_records[i].second << "\n";
    }

    return 0;
}
