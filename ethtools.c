#include <ethtools.h>


void prepare_value(uint8_t dest, uint8_t *src, uint8_t len, uint8_t surround) {
    uint8_t x = 0, val;
    for (; x<len; x++) {
        val = src[x] ^ (src[x] << 1);
        if (x == 0) val ^= (surround >> 7);
        else val ^= (src[x-1] >> 7);
        ethtxbuf[dest+x] = val;
    }
    ethtxbuf[dest+len] = (src[len-1] >> 7) ^ surround;
}

/*uint32_t compute_crc(uint32_t addr) {
    
}*/