#include <stdint.h>

uint8_t ethtxbuf[256];
uint8_t endtxbuf[32];

/**
 * Prepare an array of up to 16 bytes for transmission
 * 
 * @param dest the index of he starting position in ethtxbuf
 * @param src A pointer to the first value in the array
 * @param len The length of the array
 * @param surround The MSB of this value represents the bit transmitted immediately before the given array. The LSB of this value represents the bit transmitted immediately after the given array.
 */
void prepare_value(uint8_t dest, uint8_t *src, uint8_t len, uint8_t surround);
uint32_t compute_crc(uint32_t addr);