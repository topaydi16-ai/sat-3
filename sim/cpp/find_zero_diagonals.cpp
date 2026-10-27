#include <iostream>
#include <vector>
#include <cstdint>

int main() {
    const int T_MAX = 8000;
    std::vector<uint8_t> curr(2 * T_MAX + 5, 0);
    std::vector<uint8_t> next(2 * T_MAX + 5, 0);
    int center = T_MAX + 2;
    curr[center] = 1;

    const int J_MAX = 500;
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

    std::cout << "Searching for all Constant-0 and Constant-1 Left Diagonals up to j=" << J_MAX-1 << "...\n";
    for (int j = 0; j < J_MAX; ++j) {
        int n = (int)D[j].size();
        for (int trans = 0; trans <= 300 && trans + 50 <= n; ++trans) {
            bool all_zero = true;
            bool all_one = true;
            for (int k = trans; k < n; ++k) {
                if (D[j][k] != 0) all_zero = false;
                if (D[j][k] != 1) all_one = false;
                if (!all_zero && !all_one) break;
            }
            if (all_zero) {
                std::cout << "  ZERO DIAGONAL: D_" << j << " = 0 for t >= " << j + trans << "\n";
                break;
            }
            if (all_one) {
                std::cout << "  ONE DIAGONAL:  D_" << j << " = 1 for t >= " << j + trans << "\n";
                break;
            }
        }
    }
    std::cout << "Search complete.\n";
    return 0;
}
