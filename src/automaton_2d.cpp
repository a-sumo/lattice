#include "automaton_2d.h"
#include <arm_neon.h>
#include <algorithm>
#include <iostream>

void compute_next_state(uint8_t **current_state, uint8_t **next_state, size_t width, size_t height) {
    // Change loop ordering for better memory access pattern
#pragma omp parallel for
    for (size_t i = 1; i < height - 1; i++) {
        for (size_t j = 1; j < width - 9; j += 8) {
            uint8x8_t center = vld1_u8(&current_state[i][j]);
            uint8x8_t left = vld1_u8(&current_state[i][j - 1]);
            uint8x8_t right = vld1_u8(&current_state[i][j + 1]);
            uint8x8_t up = vld1_u8(&current_state[i - 1][j]);
            uint8x8_t down = vld1_u8(&current_state[i + 1][j]);

            // Sum up the neighbors
            uint16x8_t sum = vaddl_u8(center, left);
            sum = vaddw_u8(sum, right);
            sum = vaddw_u8(sum, up);
            sum = vaddw_u8(sum, down);

            uint8x8_t avg = vshrn_n_u16(sum, 2);  // Divide by 4 using right shift by 2
            uint8x8_t diff = vsub_u8(center, avg);
            vst1_u8(&next_state[i][j], diff);
        }

        // Non-vectorized processing for the last columns
        for (size_t j = width - 9; j < width - 1; j++) {
            uint8_t center = current_state[i][j];
            uint8_t left = current_state[i][j - 1];
            uint8_t right = current_state[i][j + 1];
            uint8_t up = current_state[i - 1][j];
            uint8_t down = current_state[i + 1][j];

            uint8_t avg = (center + left + right + up + down) / 5;
            next_state[i][j] = center - avg;
        }
    }

    std::cout << "Computed next state." << std::endl;
}

void apply_boundary_conditions(uint8_t **state, size_t width, size_t height) {
    // Reflective boundaries
    for (size_t i = 0; i < height; i++) {
        state[i][0] = state[i][1];
        state[i][width - 1] = state[i][width - 2];
    }
    for (size_t j = 0; j < width; j++) {
        state[0][j] = state[1][j];
        state[height - 1][j] = state[height - 2][j];
    }

    std::cout << "Applied boundary conditions." << std::endl;
}

void add_sustained_excitation(uint8_t **state, size_t width, size_t height, size_t timestep) {
    if (timestep % 367 == 0) {
        for (size_t i = height / 2 - 5; i < height / 2 + 5; i++) {
            for (size_t j = width / 2 - 5; j < width / 2 + 5; j++) {
                state[i][j] = 255;
            }
        }
        std::cout << "Added sustained excitation at timestep " << timestep << "." << std::endl;
        std::flush(std::cout);
    }
}
