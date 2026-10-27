#include <iostream>
#include <vector>
#include <cstdint>
#include <map>

int main() {
    const int T_MAX = 8000;
    std::vector<uint8_t> curr(2 * T_MAX + 5, 0);
    std::vector<uint8_t> next(2 * T_MAX + 5, 0);
    int center = T_MAX + 2;
    curr[center] = 1;

    const int J_MAX = 600;
    std::vector<std::vector<uint8_t>> D(J_MAX);

    for (int t = 0; t < T_MAX; ++t) {
        for (int j = 0; j < J_MAX; ++j) {
            if (t >= j) {
                int x = j - t;
                D[j].push_back(curr[center + x]);
            }
        }
        if (t + 1 < T_MAX) {
            for (int x = -(t + 1); x <= (t + 1); ++x) {
                uint8_t l = (x - 1 < -t) ? 0 : curr[center + x - 1];
                uint8_t m = (x < -t || x > t) ? 0 : curr[center + x];
                uint8_t r = (x + 1 > t) ? 0 : curr[center + x + 1];
                next[center + x] = l ^ (m | r);
            }
            for (int x = -(t + 1); x <= (t + 1); ++x) {
                curr[center + x] = next[center + x];
            }
        }
    }

    std::cout << "Deep scan of D_j periods (J_MAX=" << J_MAX << ", T_MAX=" << T_MAX << ")...\n";
    std::map<int, int> period_counts;
    int max_p = 0;

    for (int j = 0; j < J_MAX; ++j) {
        int n = (int)D[j].size();
        int found_p = 0;
        int found_trans = -1;
        for (int p : {1, 2, 4, 8, 16, 32, 64, 128}) {
            for (int trans = 0; trans <= 500 && trans + 2 * p <= n; ++trans) {
                bool ok = true;
                for (int k = trans; k < n - p; ++k) {
                    if (D[j][k] != D[j][k + p]) { ok = false; break; }
                }
                if (ok) {
                    found_p = p;
                    found_trans = trans;
                    break;
                }
            }
            if (found_p > 0) break;
        }

        period_counts[found_p]++;

        if (found_p > max_p) {
            max_p = found_p;
            std::cout << "--> BREAKTHROUGH: Period " << found_p 
                      << " first appears at j = " << j 
                      << " (transient " << found_trans << ", Rowland bound = 2^" << j << ")\n";
        } else if (found_p == 0) {
            std::cout << "j = " << j << ": Period not found within <= 128 (transient <= 500)\n";
        }
    }

    std::cout << "\nPeriod distribution across j in [0, " << J_MAX - 1 << "]:\n";
    for (auto const& [p, count] : period_counts) {
        std::cout << "  Period " << p << ": " << count << " diagonals\n";
    }

    return 0;
}
