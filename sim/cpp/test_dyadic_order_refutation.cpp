#include <iostream>
#include <vector>
#include <iomanip>
#include <numeric>
#include <cstdint>

// Compute order of 2 mod P for odd P
int order2(int P) {
    if (P % 2 == 0) return -1;
    int cur = 2 % P;
    for (int d = 1; d <= 2 * P; ++d) {
        if (cur == 1) return d;
        cur = (cur * 2) % P;
    }
    return -1;
}

int main() {
    const int MAX_T = 65536; // 64K steps!
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
    std::cout << "TESTING PERIOD EXCLUSION VIA DYADIC ORDER SUBGROUPS\n";
    std::cout << "========================================================\n";

    int tested_odd_P = 0;
    int refuted_odd_P = 0;

    for (int P = 3; P <= 500; P += 2) {
        int d = order2(P);
        if (d <= 0) continue;
        tested_odd_P++;

        // For this P, indices t = 2^(m*d) + 1 all have t = 2 (mod P)
        // If P were an eventual period, c(2^(m*d) + 1) would have to become constant for large m!
        std::vector<int> values;
        std::vector<int> t_vals;
        for (int m = 1; ; ++m) {
            int64_t power = 1;
            int exponent = m * d;
            if (exponent > 15) break; // 2^16 = 65536
            for (int e = 0; e < exponent; ++e) power *= 2;
            int64_t t = power + 1;
            if (t > MAX_T) break;
            values.push_back(center[t]);
            t_vals.push_back((int)t);
        }

        bool has0 = false, has1 = false;
        for (int v : values) {
            if (v == 0) has0 = true;
            if (v == 1) has1 = true;
        }

        if (has0 && has1) {
            refuted_odd_P++;
        }
    }

    std::cout << "Tested odd candidate periods P <= 500: " << tested_odd_P << "\n";
    std::cout << "Refuted via dyadic oscillation: " << refuted_odd_P << "\n";

    // Let's inspect small P: 3, 5, 7, 9, 11, 13, 15
    std::cout << "\nDetailed inspection for small odd P:\n";
    for (int P : {3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31}) {
        int d = order2(P);
        std::cout << "P = " << std::setw(3) << P << " (d = " << std::setw(2) << d << "): ";
        for (int m = 1; ; ++m) {
            int64_t power = 1;
            int exponent = m * d;
            if (exponent > 15) break;
            for (int e = 0; e < exponent; ++e) power *= 2;
            int64_t t = power + 1;
            if (t > MAX_T) break;
            std::cout << "c(" << t << ")=" << (int)center[t] << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
