#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cstdint>
#include <map>
#include <numeric>

// Volterra-Green Carry Source Entropy, Linear Complexity & Lucas 2-adic Valuation
// Carry source: q(t, x) = c(t, x) & c(t, x+1)

// Berlekamp-Massey algorithm over GF(2)
int berlekamp_massey(const std::vector<uint8_t>& s) {
    int n = s.size();
    std::vector<uint8_t> c(n, 0);
    std::vector<uint8_t> b(n, 0);
    c[0] = 1;
    b[0] = 1;
    int L = 0;
    int m = -1;

    for (int i = 0; i < n; ++i) {
        uint8_t d = 0;
        for (int j = 0; j <= L; ++j) {
            d ^= (c[j] & s[i - j]);
        }
        if (d == 1) {
            std::vector<uint8_t> t = c;
            int shift = i - m;
            for (int j = 0; j + shift < n; ++j) {
                c[j + shift] ^= b[j];
            }
            if (L <= i / 2) {
                L = i + 1 - L;
                m = i;
                b = t;
            }
        }
    }
    return L;
}

// Block entropy calculation
double compute_block_entropy(const std::vector<uint8_t>& seq, int k) {
    if ((int)seq.size() < k) return 0.0;
    std::map<uint32_t, int> counts;
    int total = (int)seq.size() - k + 1;
    for (int i = 0; i < total; ++i) {
        uint32_t w = 0;
        for (int b = 0; b < k; ++b) {
            w = (w << 1) | seq[i + b];
        }
        counts[w]++;
    }
    double h = 0.0;
    for (const auto& [w, cnt] : counts) {
        double p = (double)cnt / (double)total;
        h -= p * std::log2(p);
    }
    return h;
}

int main() {
    std::cout << "=======================================================================\n";
    std::cout << "EXPEDITION 4: Volterra-Green Carry Entropy, Complexity & Lucas 2-Adic\n";
    std::cout << "Author: Umidjon Rasulov | Inha University in Tashkent (SOCIE)\n";
    std::cout << "=======================================================================\n\n";

    int N = 50000; // 50,000 steps of center column and carry source
    std::vector<uint8_t> center_seq(N, 0);
    std::vector<uint8_t> carry_seq(N, 0);

    // Simulate CA
    int width = 2 * N + 1;
    int origin = N;
    std::vector<uint8_t> row(width, 0);
    std::vector<uint8_t> next_row(width, 0);
    row[origin] = 1;

    for (int t = 0; t < N; ++t) {
        center_seq[t] = row[origin];
        carry_seq[t] = row[origin] & row[origin + 1];

        int min_x = -t - 1;
        int max_x = t + 1;
        for (int x = min_x; x <= max_x; ++x) {
            uint8_t l = row[origin + x - 1];
            uint8_t m = row[origin + x];
            uint8_t r = row[origin + x + 1];
            next_row[origin + x] = l ^ (m | r);
        }
        row = next_row;
    }

    // 1. Block Entropy H_k for k in {1, 2, 3, 4, 8}
    std::cout << "--- Part 1: Block Entropy H_k (Bits) ---\n";
    std::cout << "  k | Center H_k | Center H_k/k | Carry H_k  | Carry H_k/k | I.I.D. Max H_k\n";
    std::cout << "----+------------+--------------+------------+-------------+---------------\n";
    int ks[] = {1, 2, 3, 4, 8};
    for (int k : ks) {
        double h_center = compute_block_entropy(center_seq, k);
        double h_carry = compute_block_entropy(carry_seq, k);
        std::cout << "  " << k << " | " << std::fixed << std::setprecision(5) << std::setw(10) << h_center
                  << " | " << std::setw(12) << h_center / k
                  << " | " << std::setw(10) << h_carry
                  << " | " << std::setw(11) << h_carry / k
                  << " | " << std::setw(14) << (double)k << "\n";
    }

    // 2. Linear Complexity via Berlekamp-Massey for first 10,000 bits
    std::cout << "\n--- Part 2: Linear Complexity (Berlekamp-Massey) ---\n";
    int bm_lens[] = {1000, 2000, 5000, 10000};
    for (int L_test : bm_lens) {
        std::vector<uint8_t> sub_center(center_seq.begin(), center_seq.begin() + L_test);
        std::vector<uint8_t> sub_carry(carry_seq.begin(), carry_seq.begin() + L_test);
        int lc_center = berlekamp_massey(sub_center);
        int lc_carry = berlekamp_massey(sub_carry);
        std::cout << "  N = " << std::setw(5) << L_test 
                  << " | Center L_N = " << std::setw(5) << lc_center << " (L_N/N = " << std::setprecision(4) << (double)lc_center / L_test << ")"
                  << " | Carry L_N = " << std::setw(5) << lc_carry << " (L_N/N = " << std::setprecision(4) << (double)lc_carry / L_test << ")\n";
    }

    // 3. 2-adic Valuation nu_2 correlation
    std::cout << "\n--- Part 3: 2-Adic Valuation & Lucas Support Correlation ---\n";
    std::vector<int> nu2_counts(8, 0);
    std::vector<int> nu2_center_ones(8, 0);
    std::vector<int> nu2_carry_ones(8, 0);

    for (int t = 1; t < N; ++t) {
        int v = std::countr_zero((unsigned int)t);
        if (v >= 8) v = 7;
        nu2_counts[v]++;
        if (center_seq[t]) nu2_center_ones[v]++;
        if (carry_seq[t]) nu2_carry_ones[v]++;
    }

    std::cout << "  nu_2(t) | Count | Center Density | Carry Density\n";
    std::cout << "----------+-------+----------------+--------------\n";
    for (int v = 0; v < 8; ++v) {
        double d_center = (double)nu2_center_ones[v] / nu2_counts[v];
        double d_carry = (double)nu2_carry_ones[v] / nu2_counts[v];
        std::cout << "    " << v << "     | " << std::setw(5) << nu2_counts[v] 
                  << " | " << std::fixed << std::setprecision(6) << std::setw(14) << d_center
                  << " | " << std::setw(13) << d_carry << "\n";
    }

    // Save JSON
    std::ofstream ofs("data/volterra_carry_entropy.json");
    ofs << "{\n";
    ofs << "  \"author\": \"Umidjon Rasulov\",\n";
    ofs << "  \"affiliation\": \"Inha University in Tashkent (SOCIE)\",\n";
    ofs << "  \"N\": " << N << ",\n";
    ofs << "  \"center_entropy_8\": " << compute_block_entropy(center_seq, 8) << ",\n";
    ofs << "  \"carry_entropy_8\": " << compute_block_entropy(carry_seq, 8) << ",\n";
    ofs << "  \"center_linear_complexity_10000\": " << berlekamp_massey(std::vector<uint8_t>(center_seq.begin(), center_seq.begin() + 10000)) << ",\n";
    ofs << "  \"carry_linear_complexity_10000\": " << berlekamp_massey(std::vector<uint8_t>(carry_seq.begin(), carry_seq.begin() + 10000)) << "\n";
    ofs << "}\n";
    ofs.close();

    std::cout << "\nSaved results to data/volterra_carry_entropy.json\n";
    return 0;
}
