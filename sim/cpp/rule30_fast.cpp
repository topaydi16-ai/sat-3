#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
#include <cmath>
#include <chrono>
#include <iomanip>

// Official validation vector: first 40 bits of the center column c_t for t = 0..39
static const int VALIDATION_FIRST_40[40] = {
    1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1,
    0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1
};

struct SimulationResult {
    std::vector<uint8_t> center;
    std::vector<int64_t> discrepancy;
    double elapsed_ms;
};

class Rule30Simulator {
public:
    // Run forward simulation from t=0 to t=max_t
    static SimulationResult run(size_t max_t, bool track_discrepancy = true) {
        auto t_start = std::chrono::high_resolution_clock::now();

        // Total spatial extent needed at step max_t is [-max_t, +max_t]
        // Number of bits needed: 2 * max_t + 1
        // Origin x = 0 is placed at bit index max_t.
        size_t total_bits = 2 * max_t + 65;
        size_t num_words = (total_bits + 63) / 64;
        size_t origin_bit = max_t + 32; // add a buffer word on left
        size_t origin_word = origin_bit / 64;
        size_t origin_offset = origin_bit % 64;

        std::vector<uint64_t> current(num_words, 0);
        std::vector<uint64_t> next_row(num_words, 0);

        // Initial condition: c(0, 0) = 1
        current[origin_word] |= (1ULL << origin_offset);

        SimulationResult res;
        res.center.reserve(max_t + 1);
        if (track_discrepancy) {
            res.discrepancy.reserve(max_t + 1);
        }

        uint8_t c0 = (current[origin_word] >> origin_offset) & 1ULL;
        res.center.push_back(c0);
        int64_t current_disc = (c0 == 1) ? 1 : -1;
        if (track_discrepancy) {
            res.discrepancy.push_back(current_disc);
        }

        // Active word bounds shrink/expand with causal light cone
        size_t active_min = origin_word;
        size_t active_max = origin_word;

        for (size_t t = 1; t <= max_t; ++t) {
            // Light cone expands by 1 bit left and right per step
            size_t min_bit = (origin_bit > t) ? (origin_bit - t) : 0;
            size_t max_b = origin_bit + t;
            active_min = min_bit / 64;
            active_max = max_b / 64;

            for (size_t w = active_min; w <= active_max; ++w) {
                uint64_t curr = current[w];
                uint64_t prev = (w > 0) ? current[w - 1] : 0ULL;
                uint64_t next_w = (w + 1 < num_words) ? current[w + 1] : 0ULL;

                // Cell at bit k has left neighbor at k-1, right neighbor at k+1
                // left = (curr << 1) | (prev >> 63)
                // right = (curr >> 1) | (next_w << 63)
                uint64_t left = (curr << 1) | (prev >> 63);
                uint64_t right = (curr >> 1) | (next_w << 63);

                // Rule 30: new = left ^ (curr | right)
                next_row[w] = left ^ (curr | right);
            }

            // Extract center column bit c(t, 0)
            uint8_t ct = (next_row[origin_word] >> origin_offset) & 1ULL;
            res.center.push_back(ct);

            if (track_discrepancy) {
                current_disc += (ct == 1) ? 1 : -1;
                res.discrepancy.push_back(current_disc);
            }

            // Swap buffers for active region
            for (size_t w = active_min; w <= active_max; ++w) {
                current[w] = next_row[w];
                next_row[w] = 0;
            }
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        res.elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        return res;
    }

    // Extract j-th left diagonal D_j(t) = c(t, j - t) for t >= j
    static std::vector<uint8_t> get_left_diagonal(size_t j, size_t max_t) {
        if (max_t < j) return {};
        // Simulate and record c(t, j - t)
        size_t total_bits = 2 * max_t + 65;
        size_t num_words = (total_bits + 63) / 64;
        size_t origin_bit = max_t + 32;
        size_t origin_word = origin_bit / 64;
        size_t origin_offset = origin_bit % 64;

        std::vector<uint64_t> current(num_words, 0);
        std::vector<uint64_t> next_row(num_words, 0);
        current[origin_word] |= (1ULL << origin_offset);

        std::vector<uint8_t> diag;
        diag.reserve(max_t - j + 1);

        if (j == 0) {
            diag.push_back(1); // c(0, 0) = 1
        }

        for (size_t t = 1; t <= max_t; ++t) {
            size_t min_bit = (origin_bit > t) ? (origin_bit - t) : 0;
            size_t max_b = origin_bit + t;
            size_t active_min = min_bit / 64;
            size_t active_max = max_b / 64;

            for (size_t w = active_min; w <= active_max; ++w) {
                uint64_t curr = current[w];
                uint64_t prev = (w > 0) ? current[w - 1] : 0ULL;
                uint64_t next_w = (w + 1 < num_words) ? current[w + 1] : 0ULL;
                uint64_t left = (curr << 1) | (prev >> 63);
                uint64_t right = (curr >> 1) | (next_w << 63);
                next_row[w] = left ^ (curr | right);
            }

            if (t >= j) {
                int64_t target_x = (int64_t)j - (int64_t)t;
                size_t target_bit = origin_bit + target_x;
                size_t tw = target_bit / 64;
                size_t toff = target_bit % 64;
                uint8_t val = (next_row[tw] >> toff) & 1ULL;
                diag.push_back(val);
            }

            for (size_t w = active_min; w <= active_max; ++w) {
                current[w] = next_row[w];
                next_row[w] = 0;
            }
        }
        return diag;
    }
};

int main(int argc, char** argv) {
    size_t steps = 10000;
    bool verify_only = false;
    std::string out_center_file = "";
    bool print_stats = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--steps" && i + 1 < argc) {
            steps = std::stoull(argv[++i]);
        } else if (arg == "--verify") {
            verify_only = true;
        } else if (arg == "--output" && i + 1 < argc) {
            out_center_file = argv[++i];
        } else if (arg == "--quiet") {
            print_stats = false;
        }
    }

    // Always run verification first against the validation gate
    auto res = Rule30Simulator::run(steps < 40 ? 40 : steps);

    bool match_40 = true;
    for (size_t i = 0; i < 40; ++i) {
        if (res.center[i] != VALIDATION_FIRST_40[i]) {
            match_40 = false;
            std::cerr << "VALIDATION GATE FAILURE at t=" << i
                      << ": expected " << VALIDATION_FIRST_40[i]
                      << ", got " << (int)res.center[i] << std::endl;
            return 1;
        }
    }

    if (print_stats) {
        std::cout << "[Rule30 C++ Fast Simulator] PASSED 40-bit validation gate." << std::endl;
        std::cout << "Simulated T=" << steps << " steps in "
                  << std::fixed << std::setprecision(3) << res.elapsed_ms << " ms ("
                  << (double)steps / (res.elapsed_ms / 1000.0) << " steps/sec)" << std::endl;

        // Black count & empirical density
        size_t black_count = 0;
        for (size_t t = 0; t <= steps; ++t) {
            if (res.center[t] == 1) black_count++;
        }
        double density = (double)black_count / (steps + 1);
        std::cout << "Black count: " << black_count << " / " << (steps + 1)
                  << " (density: " << std::setprecision(6) << density << ")" << std::endl;
        std::cout << "Final Discrepancy S(" << steps << ") = " << res.discrepancy.back() << std::endl;
    }

    if (!out_center_file.empty()) {
        std::ofstream ofs(out_center_file);
        for (size_t i = 0; i <= steps; ++i) {
            ofs << (int)res.center[i];
        }
        ofs << "\n";
        ofs.close();
        if (print_stats) {
            std::cout << "Wrote center sequence to " << out_center_file << std::endl;
        }
    }

    return 0;
}
