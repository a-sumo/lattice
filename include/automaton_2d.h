#ifndef AUTOMATON_2D_H
#define AUTOMATON_2D_H

#include <stddef.h>
#include <stdint.h>

void compute_next_state(uint8_t** current_state, uint8_t** next_state, size_t width, size_t height);
void apply_boundary_conditions(uint8_t** state, size_t width, size_t height);
void add_sustained_excitation(uint8_t** state, size_t width, size_t height, size_t timestep);
#endif
