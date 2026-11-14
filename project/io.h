#pragma once

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize IO layer
void init_io();

// Get input from IO layer
ssize_t input_io(uint8_t* buf, size_t max_length);

// Output to IO layer
void output_io(uint8_t* buf, size_t length);

#ifdef __cplusplus
}
#endif
