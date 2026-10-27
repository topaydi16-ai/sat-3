#include <iostream>
#include <vector>
#include <cstdint>

int main() {
    uint64_t total_words = 16;
    uint64_t center_word = 8;
    int center_bit = 32;

    std::vector<uint64_t> row(total_words, 0ULL);
    std::vector<uint64_t> next_row(total_words, 0ULL);

    row[center_word] = (1ULL << center_bit);

    std::cout << "Testing Rule 30 center column for 10 steps:\n";
    for (int t = 0; t < 10; ++t) {
        int center_cell = (row[center_word] >> center_bit) & 1;
        std::cout << "t=" << t << ": " << center_cell << "\n";

        for (uint64_t w = 1; w < total_words - 1; ++w) {
            uint64_t curr = row[w];
            uint64_t prev_w = row[w - 1];
            uint64_t next_w_val = row[w + 1];

            // If bit k is position x:
            // x-1 is bit k-1
            // In a uint64_t, bit k-1 is (curr >> 1). From prev_w, bit 63 shifts to bit -1?
            // Wait! If bit k is position x, then bit k+1 is (curr << 1), which is x+1!
            // Let's check:
            // left cell (x-1): its bit is k-1.
            // When we compute cell x at bit k, we need the value of cell x-1 (which was at bit k-1).
            // To align bit k-1 with bit k, we must SHIFT LEFT by 1! Because bit k-1 << 1 becomes bit k!
            // And for cell x+1 (which was at bit k+1), to align bit k+1 with bit k, we must SHIFT RIGHT by 1!
            // Let's check this!
            uint64_t left = (curr << 1) | (prev_w >> 63);
            uint64_t right = (curr >> 1) | (next_w_val << 63);

            next_row[w] = left ^ (curr | right);
        }
        row.swap(next_row);
    }
    return 0;
}
