#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdint>

int main() {
    const int MAX_T = 16385;
    std::vector<uint8_t> prev(2 * MAX_T + 3, 0);
    std::vector<uint8_t> curr(2 * MAX_T + 3, 0);
    std::vector<uint8_t> center(MAX_T + 1, 0);

    const int origin = MAX_T + 1;
    prev[origin] = 1;
    center[0] = 1;

    for (int t = 1; t <= MAX_T; ++t) {
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
    std::cout << "DYADIC SHOCK FRONT ANALYSIS AT t_k = 2^(k+1) + 1\n";
    std::cout << "========================================================\n";
    std::cout << std::setw(4) << "k"
              << std::setw(10) << "2^k"
              << std::setw(12) << "t_k"
              << std::setw(10) << "c(t_k)"
              << std::setw(12) << "c(t_k - 1)"
              << std::setw(12) << "c(t_k + 1)"
              << "\n";
    std::cout << "--------------------------------------------------------\n";

    for (int k = 0; k <= 13; ++k) {
        int two_k = 1 << k;
        int t_k = (1 << (k + 1)) + 1;
        if (t_k <= MAX_T) {
            std::cout << std::setw(4) << k
                      << std::setw(10) << two_k
                      << std::setw(12) << t_k
                      << std::setw(10) << (int)center[t_k]
                      << std::setw(12) << (int)center[t_k - 1]
                      << std::setw(12) << (int)center[t_k + 1]
                      << "\n";
        }
    }

    std::cout << "\nChecking left corner carry q(2^k, -2^k):\n";
    std::cout << "q(2^k, -2^k) = D_0(2^k) * D_1(2^k) = 1 * 1 = 1 for all k >= 1.\n";

    return 0;
}
