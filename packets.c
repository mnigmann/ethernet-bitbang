#include <packets.h>
#include <ethtools.h>
#include <avr/io.h>

// NOTE: the CRC algorithms for packet_0 and packet_2 are DIFFERENT!
// Only PACKET2 can account for the "sbrs" instruction


void _print_hex(uint8_t num) {
    UDR0 = (num >> 4) + (num >= 160?('A'-10):'0');
    while (!(UCSR0A & 0b00100000));
    UDR0 = (num & 0x0f) + ((num & 0x0f) >= 10?('A'-10):'0');
    while (!(UCSR0A & 0b00100000));
}

void packet_0(uint8_t *num) {
    prepare_value(0, num, 1, 0);
    register uint32_t crc asm("r24");
    asm volatile (
        "push r0\n\tpush r1\n\tpush r2\n\tpush r3\n\tpush r4\n\tpush r5\n\tpush r6\n\tpush r7\n\tpush r8\n\tpush r9\n\tpush r10\n\tpush r11\n\tpush r12\n\tpush r13\n\tpush r14\n\tpush r15\n\t"
        "ser r24\n\t"
        "ser r25\n\t"
        "ser r26\n\t"
        "ser r27\n\t"
        "ldi r30, lo8(packet_0_databegin)\n\t"
        "ldi r31, hi8(packet_0_databegin)\n\t"
        "ser r29\n\t"
        "packet_0_crcloop:\n\t"
            "lpm r22, Z+\n\t"
            "lpm r23, Z+\n\t"
            "push r30\n\t"
            "push r31\n\t"
            // Identify the first instruction:
            // NOP:  r23 == 0x00
            "cpi r23, 0\n\t"
            "breq packet_0_checknext\n\t"
            // SBIC: r23 == 0x99
            "cpi r23, 0x99\n\t"
            "brne packet_0_nsbic\n\t"
                "in r28, 0x1e\n\t"
                "rjmp packet_0_shift\n\t"
            "packet_0_nsbic:\n\t"
            // SBRC: r23 == 0xFC or r23 = 0xFD
            "mov r28, r23\n\t"
            "andi r28, 0xFE\n\t"
            "cpi r28, 0xFC\n\t"
            "brne packet_0_nsbrc\n\t"
                // Load r28 with the address of the target register
                "mov r28, r22\n\t"
                "andi r28, 0xF0\n\t"
                "andi r23, 1\n\t"
                "or r28, r23\n\t"
                "swap r28\n\t"
                // Load r28 with the value in that register
                "clr r31\n\t"
                "mov r30, r28\n\t"
                "ld r28, Z\n\t"
                // Load r22 with the requested bit and shift
                "packet_0_shift:\n\t"
                "andi r22, 0x07\n\t"
                "cpi r22, 7\n\t"
                "breq packet_0_noshift\n\t"
                    "lsl r28\n\t"
                    "inc r22\n\t"
                    "rjmp packet_0_shift\n\t"
                "packet_0_noshift:\n\t"
                
                
                "pop r31\n\t"
                "pop r30\n\t"
                "adiw r30, 2\n\t"
                
                "sbrc r28, 7\n\t"
                "rjmp packet_0_change\n\t"
                "rjmp packet_0_nochange\n\t"
                
            "packet_0_nsbrc:\n\t"
            // LDS:  r23 == 0x90 or r23 == 0x91
            "cpi r28, 0x90\n\t"
            "brne packet_0_nlds\n\t"
                // Set r28 to the target register
                "mov r28, r23\n\t"
                "andi r28, 1\n\t"
                "or r28, r22\n\t"
                "swap r28\n\t"
                
                "lpm r22, Z+\n\t"
                "lpm r23, Z+\n\t"
                "movw r30, r22\n\t"
                "ld r22, Z\n\t"
                "clr r31\n\t"
                "mov r30, r28\n\t"
                "st Z, r22\n\t"
                "pop r31\n\t"
                "pop r30\n\t"
                "adiw r30, 2\n\t"
                "rjmp packet_0_nochange\n\t"
                
            "packet_0_nlds:\n\t"
            "nop\n\t"
            // OUT:  r23 == 0xBB
            
            "packet_0_checknext:\n\t"
            "pop r31\n\t"
            "pop r30\n\t"
            "adiw r30, 1\n\t"
            "lpm r28, Z+\n\t"
            "cpi r28, 0xbd\n\t"
            "brne packet_0_nochange\n\t"
                "packet_0_change:\n\t"
                "com r29\n\t"
            "packet_0_nochange:\n\t"
            // r29 now contains the current bit value
            
            
                /*"push r24\n\t"
                "push r25\n\t"
                "mov r24, r29\n\t"
                "call _print_hex\n\t"
                "pop r25\n\t"
                "pop r24\n\t"*/
            
            "mov r28, r24\n\t"
            "sbrc r29, 0\n\t"
                "com r28\n\t"
            // r28 now contains crc^bit
            "lsr r27\n\t"
            "ror r26\n\t"
            "ror r25\n\t"
            "ror r24\n\t"
            "andi r28, 1\n\t"
            "breq packet_0_nopoly\n\t"
                // The following code runs if ((crc^bit)&1)
                "ldi r28, 0x20\n\t"
                "eor r24, r28\n\t"
                "ldi r28, 0x83\n\t"
                "eor r25, r28\n\t"
                "ldi r28, 0xB8\n\t"
                "eor r26, r28\n\t"
                "ldi r28, 0xED\n\t"
                "eor r27, r28\n\t"
            "packet_0_nopoly:\n\t"
            "ldi r28, hi8(packet_0_dataend)\n\t"
            "cpi r30, lo8(packet_0_dataend)\n\t"
            "cpc r31, r28\n\t"
            "in r28, 0x3f\n\t"
            "sbrc r28, 0\n\t"
            "rjmp packet_0_crcloop\n\t"
        "com r24\n\t"
        "com r25\n\t"
        "com r26\n\t"
        "com r27\n\t"
        "mov r28, r24\n\t"
        "lsl r28\n\t"
        "eor r24, r28\n\t"
        "mov r28, r25\n\t"
        "rol r28\n\t"
        "eor r25, r28\n\t"
        "mov r28, r26\n\t"
        "rol r28\n\t"
        "eor r26, r28\n\t"
        "mov r28, r27\n\t"
        "rol r28\n\t"
        "eor r27, r28\n\t"
        
        "packet_0_continue:\n\t"
        "pop r15\n\tpop r14\n\tpop r13\n\tpop r12\n\tpop r11\n\tpop r10\n\tpop r9\n\tpop r8\n\tpop r7\n\tpop r6\n\tpop r5\n\tpop r4\n\tpop r3\n\tpop r2\n\tpop r1\n\tpop r0\n\t"
        
        "push r24\n\t"
        "push r25\n\t"
        "push r26\n\t"
        "push r27\n\t"
        
        :
        :
        : "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
    );
    /*_print_hex(crc >> 24);
    _print_hex((crc >> 16)&255);
    _print_hex((crc >> 8)&255);
    _print_hex(crc & 0xff);
    UDR0 = '\r';
    while (!(UCSR0A & 0b00100000));
    UDR0 = '\n';
    while (!(UCSR0A & 0b00100000));*/

    asm (
        "pop r19\n\t"
        "pop r18\n\t"
        "pop r17\n\t"
        "pop r16\n\t"
        "ldi r26, 255\n\t"
        "ldi r24, 0b00000011\n\t"
        "out 0x13, r24\n\t"
        "out 0x04, r24\n\t"
        "ldi r24, 0b10100010\n\t"
        "out 36, r24\n\t"
        "ldi r24, 0b10100011\n\t"
        "out 0x13, r24\n\t"
        "out 0x04, r24\n\t"
        "ldi r24, 0b10110010\n\t"
        "ldi r25, 0b01010010\n\t"
        "out 36, r24\n\t"
        "out 36, r25\n\t"
                
        // Transmit the preamble
        "out 38, r26\n\t" CHANGE7
        "ldi r24, 0b11100010\n\t" "out 38, r26\n\t" CHANGE7
        CHANGE8
        CHANGE8
        CHANGE8
        CHANGE8
        CHANGE7
        CHANGE7 NOCHANGE
        "packet_0_databegin:\n\t"
        E2R(0,0) E2G(1)            NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
          CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 12, difference 37
        NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 34, difference 5c
        NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 56, difference fa
        NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 78, difference 88
        NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 9a, difference ae
          CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte bc, difference c5
          
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 08, difference 19
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
          CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte 45, difference cf
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
          CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 1d, difference 27
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte 40, difference c0
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 80, difference 80
        NOCHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 11, difference 32
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte c0, difference 40
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte a8, difference f9
          CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0b
        NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 64, difference ac
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte c0, difference 40
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte a8, difference f9
          CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0b
          CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 01
        
          CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 01
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
          CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 13, difference 35
        NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 88, difference 98
          CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 01
          CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 09, difference 1b
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        FROMREG("r0")                                                            // Current byte 55, difference ff
        //  CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 55, difference ff
        L(0)     NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        "packet_0_dataend:\n\t"
        FROMREG("r16")
        FROMREG("r17")
        FROMREG("r18")
        FROMREG("r19")
        /*NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 98, difference a8
          CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte 42, difference c7
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte d0, difference 70
          CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE  // Current byte b4, difference dd*/

        //NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE  // Current byte a0, difference e0 (LSB)
        //  CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 0e, difference 13
        //NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte d0, difference 70
        //  CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE  // Current byte 5e, difference e3 (MSB)

        /*NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE  // Current byte a6, difference ea
        NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte d9, difference 6a
          CHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 78, difference 89
          CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 55, difference ff*/
        "nop\n\t"
        "out 36, r24\n\t"
        "nop\n\tnop\n\tnop\n\t"
        "ldi r26, 0b00000010\n\t"
        "out 36, r26\n\t"
        :
        :
        : "r0", "r1", "r24", "r25", "r26"
    );
}

void packet_1() {
    asm (
        "ldi r26, 255\n\t"
        "ldi r24, 0b00000011\n\t"
        "out 0x13, r24\n\t"
        "out 0x04, r24\n\t"
        "ldi r24, 0b10100010\n\t"
        "out 36, r24\n\t"
        "ldi r24, 0b10100011\n\t"
        "out 0x13, r24\n\t"
        "out 0x04, r24\n\t"
        "ldi r24, 0b10110010\n\t"
        "ldi r25, 0b01010010\n\t"
        "out 36, r24\n\t"
        "out 36, r25\n\t"
                
        // Transmit the preamble
        "out 38, r26\n\t" CHANGE7
        "ldi r24, 0b11100010\n\t" "out 38, r26\n\t" CHANGE7
        CHANGE8
        CHANGE8
        CHANGE8
        CHANGE8
        CHANGE7
        CHANGE7 NOCHANGE
        
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 00
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 68, difference b9
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte f7, difference 19
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 28, difference 79
        NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte da, difference 6e
        NOCHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 11, difference 32
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 10, difference 30
        NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 08, difference 18
        NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0a
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
          CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 01, difference 03
        NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 08, difference 18
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0a
        NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 04, difference 0c
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 02, difference 06
        NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 12, difference 36
        NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 34, difference 5c
        NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 56, difference fa
        NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 78, difference 88
        NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 9a, difference ae
          CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte bc, difference c5
          CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte c0, difference 41
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte a8, difference f9
          CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0b
        NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 64, difference ac
        NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 68, difference b8
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte f7, difference 19
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 28, difference 79
        NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte da, difference 6e
        NOCHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 11, difference 32
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 10, difference 30
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte c0, difference 40
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte a8, difference f9
          CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0b
        NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 0a, difference 1e
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
          CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 51, difference f3
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE  // Current byte a0, difference e0
          CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 2a, difference 7f
          CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 71, difference 93
        "nop\n\t"
        "out 36, r24\n\t"
        "nop\n\tnop\n\tnop\n\t"
        "ldi r26, 0b00000010\n\t"
        "out 36, r26\n\t"
        ::: "r24", "r25", "r26"
    );
}



void packet_2(uint8_t *sender_ip, uint8_t *sender_mac) {
    prepare_value(0, sender_mac, 6, 0x80);
    prepare_value(7, sender_ip, 4, sender_mac[5] & 0x80);
    register uint32_t crc asm("r24");
    asm volatile (
        "push r0\n\tpush r1\n\tpush r2\n\tpush r3\n\tpush r4\n\tpush r5\n\tpush r6\n\tpush r7\n\tpush r8\n\tpush r9\n\tpush r10\n\tpush r11\n\tpush r12\n\tpush r13\n\tpush r14\n\tpush r15\n\t"
        "push r16\n\tpush r17\n\tpush r18\n\tpush r19\n\tpush r22\n\tpush r23\n\tpush r24\n\tpush r25\n\tpush r26\n\tpush r27\n\tpush r28\n\tpush r29\n\tpush r30\n\tpush r31\n\t"
        // Without this line the CRC algorithm will not find the values of r0 to r5 and will not compute the CRC correctly
        E2R(0,0) E2R(1,1) E2R(2,2) E2R(3,3) E2R(4,4) E2R(5,5) E2R(6,6)
        "ser r24\n\t"
        "ser r25\n\t"
        "ser r26\n\t"
        "ser r27\n\t"
        "ldi r30, lo8(packet_2_databegin)\n\t"
        "ldi r31, hi8(packet_2_databegin)\n\t"
        "ser r29\n\t"
        "ldi r16, 0\n\t"
        "packet_2_crcloop:\n\t"
            "lpm r22, Z+\n\t"
            "lpm r23, Z+\n\t"
            "push r30\n\t"
            "push r31\n\t"
            // Identify the first instruction:
            // NOP:  r23 == 0x00
            "cpi r23, 0\n\t"
            "breq packet_2_checknext\n\t"
            // SBIC: r23 == 0x99
            "cpi r23, 0x99\n\t"
            "brne packet_2_nsbic\n\t"
                "in r28, 0x1e\n\t"
                "rjmp packet_2_shift\n\t"
            "packet_2_nsbic:\n\t"
            // SBRC: r23 == 0xFC or r23 = 0xFD
            "mov r28, r23\n\t"
            "mov r16, r23\n\t"
            "andi r28, 0xFC\n\t"
            "cpi r28, 0xFC\n\t"
            "brne packet_2_nsbrc\n\t"
                // Load r28 with the address of the target register
                "mov r28, r22\n\t"
                "andi r28, 0xF0\n\t"
                "andi r23, 1\n\t"
                "or r28, r23\n\t"
                "swap r28\n\t"
                // Load r28 with the value in that register
                "clr r31\n\t"
                "mov r30, r28\n\t"
                "ld r28, Z\n\t"
                // Load r22 with the requested bit and shift
                "packet_2_shift:\n\t"
                "andi r22, 0x07\n\t"
                "cpi r22, 7\n\t"
                "breq packet_2_noshift\n\t"
                    "lsl r28\n\t"
                    "inc r22\n\t"
                    "rjmp packet_2_shift\n\t"
                "packet_2_noshift:\n\t"
                
                
                "pop r31\n\t"
                "pop r30\n\t"
                "adiw r30, 2\n\t"
                
                
                "sbrc r16, 1\n\t"
                "com r28\n\t"
                "sbrc r28, 7\n\t"
                "rjmp packet_2_change\n\t"
                "rjmp packet_2_nochange\n\t"
                
            "packet_2_nsbrc:\n\t"
            "ldi r16, 0\n\t"
            // LDS:  r23 == 0x90 or r23 == 0x91
            "cpi r28, 0x90\n\t"
            "brne packet_2_nlds\n\t"
                // Set r28 to the target register
                "mov r28, r23\n\t"
                "andi r28, 1\n\t"
                "or r28, r22\n\t"
                "swap r28\n\t"
                
                "lpm r22, Z+\n\t"
                "lpm r23, Z+\n\t"
                "movw r30, r22\n\t"
                "ld r22, Z\n\t"
                "clr r31\n\t"
                "mov r30, r28\n\t"
                "st Z, r22\n\t"
                "pop r31\n\t"
                "pop r30\n\t"
                "adiw r30, 2\n\t"
                "rjmp packet_2_nochange\n\t"
                
            "packet_2_nlds:\n\t"
            "nop\n\t"
            // OUT:  r23 == 0xBB
            
            "packet_2_checknext:\n\t"
            "pop r31\n\t"
            "pop r30\n\t"
            "adiw r30, 1\n\t"
            "lpm r28, Z+\n\t"
            "cpi r28, 0xbd\n\t"
            "brne packet_2_nochange\n\t"
                "packet_2_change:\n\t"
                "com r29\n\t"
            "packet_2_nochange:\n\t"
            // r29 now contains the current bit value
            
            
                /*"push r24\n\t"
                "push r25\n\t"
                "mov r24, r29\n\t"
                "call _print_hex\n\t"
                "pop r25\n\t"
                "pop r24\n\t"*/
            
            "mov r28, r24\n\t"
            "sbrc r29, 0\n\t"
                "com r28\n\t"
            // r28 now contains crc^bit
            "lsr r27\n\t"
            "ror r26\n\t"
            "ror r25\n\t"
            "ror r24\n\t"
            "andi r28, 1\n\t"
            "breq packet_2_nopoly\n\t"
                // The following code runs if ((crc^bit)&1)
                "ldi r28, 0x20\n\t"
                "eor r24, r28\n\t"
                "ldi r28, 0x83\n\t"
                "eor r25, r28\n\t"
                "ldi r28, 0xB8\n\t"
                "eor r26, r28\n\t"
                "ldi r28, 0xED\n\t"
                "eor r27, r28\n\t"
            "packet_2_nopoly:\n\t"
            "ldi r28, hi8(packet_2_dataend)\n\t"
            "cpi r30, lo8(packet_2_dataend)\n\t"
            "cpc r31, r28\n\t"
            "in r28, 0x3f\n\t"
            "sbrc r28, 0\n\t"
            "rjmp packet_2_crcloop\n\t"
        "com r24\n\t"
        "com r25\n\t"
        "com r26\n\t"
        "com r27\n\t"
        "mov r28, r24\n\t"
        "lsl r28\n\t"
        "eor r24, r28\n\t"
        "mov r28, r25\n\t"
        "rol r28\n\t"
        "eor r25, r28\n\t"
        "mov r28, r26\n\t"
        "rol r28\n\t"
        "eor r26, r28\n\t"
        "mov r28, r27\n\t"
        "rol r28\n\t"
        "eor r27, r28\n\t"
        
        "packet_2_continue:\n\t"
        
        /*"push r24\n\t"
        "push r25\n\t"
        "push r26\n\t"
        "mov r24, r27\n\t"
        "call _print_hex\n\t"
        "pop r24\n\t"
        "call _print_hex\n\t"
        "pop r24\n\t"
        "call _print_hex\n\t"
        "pop r24\n\t"
        "call _print_hex\n\t"*/
        
        
        
        
        E2R(0,0) E2R(1,1) E2R(2,2) E2R(3,3) E2R(4,4) E2R(5,5) E2R(6,6)
        "mov r16, r24\n\t"
        "mov r17, r25\n\t"
        "mov r18, r26\n\t"
        "mov r19, r27\n\t"
        "ldi r26, 255\n\t"
        "ldi r24, 0b00000011\n\t"
        "out 0x13, r24\n\t"
        "out 0x04, r24\n\t"
        "ldi r24, 0b10100010\n\t"
        "out 36, r24\n\t"
        "ldi r24, 0b10100011\n\t"
        "out 0x13, r24\n\t"
        "out 0x04, r24\n\t"
        "ldi r24, 0b10110010\n\t"
        "ldi r25, 0b01010010\n\t"
        "out 36, r24\n\t"
        "out 36, r25\n\t"
                
        // Transmit the preamble
        "out 38, r26\n\t" CHANGE7
        "ldi r24, 0b11100010\n\t" "out 38, r26\n\t" CHANGE7
        CHANGE8
        CHANGE8
        CHANGE8
        CHANGE8
        CHANGE7
        CHANGE7 NOCHANGE
        "packet_2_databegin:\n\t"
        FROMREG("r0")
        FROMREG("r1")
        FROMREG("r2")
        FROMREG("r3")
        FROMREG("r4")
        FROMREG("r5")
        B(6,0)     CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 12, difference 37
        NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 34, difference 5c
        NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 56, difference fa
        NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 78, difference 88
        NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 9a, difference ae
          CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte bc, difference c5
          
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE E2R(7,7)  // Current byte 08, difference 18
        NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE E2R(8,8)  // Current byte 06, difference 0a
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE E2R(9,9)  // Current byte 00, difference 00
          CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE E2R(10,10)  // Current byte 01, difference 03
        NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE E2R(11,11)  // Current byte 08, difference 18
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0a
        NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 04, difference 0c
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 02, difference 06
        
        NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 12, difference 36
        NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 34, difference 5c
        NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 56, difference fa
        NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 78, difference 88
        NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 9a, difference ae
          CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte bc, difference c5
          
          CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte c0, difference 41
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte a8, difference f9
          CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0b
        NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 64, difference ac
        
        FROMREG_INV0("r0")
        FROMREG("r1")
        FROMREG("r2")
        FROMREG("r3")
        FROMREG("r4")
        FROMREG("r5")
        
        FROMREG("r7")
        FROMREG("r8")
        FROMREG("r9")
        FROMREG("r10")
        
        B(11,0)  NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        "packet_2_dataend:\n\t"
        FROMREG("r16")
        FROMREG("r17")
        FROMREG("r18")
        FROMREG("r19")
        "nop\n\t"
        "out 36, r24\n\t"
        "nop\n\tnop\n\tnop\n\t"
        "ldi r26, 0b00000010\n\t"
        "out 36, r26\n\t"
        "pop r31\n\tpop r30\n\tpop r29\n\tpop r28\n\tpop r27\n\tpop r26\n\tpop r25\n\tpop r24\n\tpop r23\n\tpop r22\n\tpop r19\n\tpop r18\n\tpop r17\n\tpop r16\n\t"
        "pop r15\n\tpop r14\n\tpop r13\n\tpop r12\n\tpop r11\n\tpop r10\n\tpop r9\n\tpop r8\n\tpop r7\n\tpop r6\n\tpop r5\n\tpop r4\n\tpop r3\n\tpop r2\n\tpop r1\n\tpop r0\n\t"
        :
        :
        :
    );
}


void packet_3(uint8_t* dest_mac, uint8_t *dest_ip, uint8_t* icmp_head) {
    icmp_head[11] = 0;      // Set the last byte of the timestamp to 0
    uint16_t temp;
    volatile uint16_t ip_check = 0xCC62;
    ADDCHECK(ip_check, (dest_ip[0] << 8) | dest_ip[1]);
    ADDCHECK(ip_check, (dest_ip[2] << 8) | dest_ip[3]);
    ip_check = ~ip_check;
    _print_hex(ip_check >> 8);
    _print_hex(ip_check & 0xff);
    volatile uint16_t icmp_check = 0xBED2;
    for (uint8_t i=0; i < 20; i+=2) ADDCHECK(icmp_check, (icmp_head[i]<<8) | icmp_head[i+1]);
    icmp_check = ~icmp_check;
    _print_hex(icmp_check >> 8);
    _print_hex(icmp_check & 0xff);
    icmp_check = (icmp_check << 8) | (icmp_check >> 8);
    ip_check = (ip_check << 8) | (ip_check >> 8);
    prepare_value(0, (uint8_t*)(&ip_check), 2, 0);
    prepare_value(3, dest_ip, 4, 0);
    prepare_value(8, (uint8_t*)(&icmp_check), 2, icmp_head[0] & 0x01);
    prepare_value(11, icmp_head, 20, (uint8_t)(icmp_check & 0x80));
    prepare_value(32, dest_mac, 6, 0x80);
    
    
    asm volatile (
        "push r0\n\tpush r1\n\tpush r2\n\tpush r3\n\tpush r4\n\tpush r5\n\tpush r6\n\tpush r7\n\tpush r8\n\tpush r9\n\tpush r10\n\tpush r11\n\tpush r12\n\tpush r13\n\tpush r14\n\tpush r15\n\t"
        "push r16\n\tpush r17\n\tpush r18\n\tpush r19\n\tpush r22\n\tpush r23\n\tpush r24\n\tpush r25\n\tpush r26\n\tpush r27\n\tpush r28\n\tpush r29\n\tpush r30\n\tpush r31\n\t"
        // Without this line the CRC algorithm will not find the values of r0 to r5 and will not compute the CRC correctly
        E2R(0,32) E2R(1,33) E2R(2,34) E2R(3,35) E2R(4,36) E2R(5,37) E2R(6,38)
        "ser r24\n\t"
        "ser r25\n\t"
        "ser r26\n\t"
        "ser r27\n\t"
        "ldi r30, lo8(packet_3_databegin)\n\t"
        "ldi r31, hi8(packet_3_databegin)\n\t"
        "ser r29\n\t"
        "ldi r16, 0\n\t"
        "packet_3_crcloop:\n\t"
            "lpm r22, Z+\n\t"
            "lpm r23, Z+\n\t"
            "push r30\n\t"
            "push r31\n\t"
            // Identify the first instruction:
            // NOP:  r23 == 0x00
            "cpi r23, 0\n\t"
            "breq packet_3_checknext\n\t"
            // SBIC: r23 == 0x99
            "cpi r23, 0x99\n\t"
            "brne packet_3_nsbic\n\t"
                "in r28, 0x1e\n\t"
                "rjmp packet_3_shift\n\t"
            "packet_3_nsbic:\n\t"
            // SBRC: r23 == 0xFC or r23 = 0xFD
            "mov r28, r23\n\t"
            "mov r16, r23\n\t"
            "andi r28, 0xFC\n\t"
            "cpi r28, 0xFC\n\t"
            "brne packet_3_nsbrc\n\t"
                // Load r28 with the address of the target register
                "mov r28, r22\n\t"
                "andi r28, 0xF0\n\t"
                "andi r23, 1\n\t"
                "or r28, r23\n\t"
                "swap r28\n\t"
                // Load r28 with the value in that register
                "clr r31\n\t"
                "mov r30, r28\n\t"
                "ld r28, Z\n\t"
                // Load r22 with the requested bit and shift
                "packet_3_shift:\n\t"
                "andi r22, 0x07\n\t"
                "cpi r22, 7\n\t"
                "breq packet_3_noshift\n\t"
                    "lsl r28\n\t"
                    "inc r22\n\t"
                    "rjmp packet_3_shift\n\t"
                "packet_3_noshift:\n\t"
                
                
                "pop r31\n\t"
                "pop r30\n\t"
                "adiw r30, 2\n\t"
                
                
                "sbrc r16, 1\n\t"
                "com r28\n\t"
                "sbrc r28, 7\n\t"
                "rjmp packet_3_change\n\t"
                "rjmp packet_3_nochange\n\t"
                
            "packet_3_nsbrc:\n\t"
            "ldi r16, 0\n\t"
            // LDS:  r23 == 0x90 or r23 == 0x91
            "cpi r28, 0x90\n\t"
            "brne packet_3_nlds\n\t"
                // Set r28 to the target register
                "mov r28, r23\n\t"
                "andi r28, 1\n\t"
                "or r28, r22\n\t"
                "swap r28\n\t"
                
                "lpm r22, Z+\n\t"
                "lpm r23, Z+\n\t"
                "movw r30, r22\n\t"
                "ld r22, Z\n\t"
                "clr r31\n\t"
                "mov r30, r28\n\t"
                "st Z, r22\n\t"
                "pop r31\n\t"
                "pop r30\n\t"
                "adiw r30, 2\n\t"
                "rjmp packet_3_nochange\n\t"
                
            "packet_3_nlds:\n\t"
            "nop\n\t"
            // OUT:  r23 == 0xBB
            
            "packet_3_checknext:\n\t"
            "pop r31\n\t"
            "pop r30\n\t"
            "adiw r30, 1\n\t"
            "lpm r28, Z+\n\t"
            "cpi r28, 0xbd\n\t"
            "brne packet_3_nochange\n\t"
                "packet_3_change:\n\t"
                "com r29\n\t"
            "packet_3_nochange:\n\t"
            // r29 now contains the current bit value
            
            
                /*"push r24\n\t"
                "push r25\n\t"
                "mov r24, r29\n\t"
                "call _print_hex\n\t"
                "pop r25\n\t"
                "pop r24\n\t"*/
            
            "mov r28, r24\n\t"
            "sbrc r29, 0\n\t"
                "com r28\n\t"
            // r28 now contains crc^bit
            "lsr r27\n\t"
            "ror r26\n\t"
            "ror r25\n\t"
            "ror r24\n\t"
            "andi r28, 1\n\t"
            "breq packet_3_nopoly\n\t"
                // The following code runs if ((crc^bit)&1)
                "ldi r28, 0x20\n\t"
                "eor r24, r28\n\t"
                "ldi r28, 0x83\n\t"
                "eor r25, r28\n\t"
                "ldi r28, 0xB8\n\t"
                "eor r26, r28\n\t"
                "ldi r28, 0xED\n\t"
                "eor r27, r28\n\t"
            "packet_3_nopoly:\n\t"
            "ldi r28, hi8(packet_3_dataend)\n\t"
            "cpi r30, lo8(packet_3_dataend)\n\t"
            "cpc r31, r28\n\t"
            "in r28, 0x3f\n\t"
            "sbrc r28, 0\n\t"
            "rjmp packet_3_crcloop\n\t"
        "com r24\n\t"
        "com r25\n\t"
        "com r26\n\t"
        "com r27\n\t"
        "mov r28, r24\n\t"
        "lsl r28\n\t"
        "eor r24, r28\n\t"
        "mov r28, r25\n\t"
        "rol r28\n\t"
        "eor r25, r28\n\t"
        "mov r28, r26\n\t"
        "rol r28\n\t"
        "eor r26, r28\n\t"
        "mov r28, r27\n\t"
        "rol r28\n\t"
        "eor r27, r28\n\t"
        
        "packet_3_continue:\n\t"
        
        /*"push r24\n\t"
        "push r25\n\t"
        "push r26\n\t"
        "mov r24, r27\n\t"
        "call _print_hex\n\t"
        "pop r24\n\t"
        "call _print_hex\n\t"
        "pop r24\n\t"
        "call _print_hex\n\t"
        "pop r24\n\t"
        "call _print_hex\n\t"*/
        
        
        
        
        E2R(0,32) E2R(1,33) E2R(2,34) E2R(3,35) E2R(4,36) E2R(5,37) E2R(6,38)
        "mov r16, r24\n\t"
        "mov r17, r25\n\t"
        "mov r18, r26\n\t"
        "mov r19, r27\n\t"
        "ldi r26, 255\n\t"
        "ldi r24, 0b00000011\n\t"
        "out 0x13, r24\n\t"
        "out 0x04, r24\n\t"
        "ldi r24, 0b10100010\n\t"
        "out 36, r24\n\t"
        "ldi r24, 0b10100011\n\t"
        "out 0x13, r24\n\t"
        "out 0x04, r24\n\t"
        "ldi r24, 0b10110010\n\t"
        "ldi r25, 0b01010010\n\t"
        "out 36, r24\n\t"
        "out 36, r25\n\t"
                
        // Transmit the preamble
        "out 38, r26\n\t" CHANGE7
        "ldi r24, 0b11100010\n\t" "out 38, r26\n\t" CHANGE7
        CHANGE8
        CHANGE8
        CHANGE8
        CHANGE8
        CHANGE7
        CHANGE7 NOCHANGE
        "packet_3_databegin:\n\t"
        FROMREG("r0")
        FROMREG("r1")
        FROMREG("r2")
        FROMREG("r3")
        FROMREG("r4")
        FROMREG("r5")
        B(6,0)     CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 12, difference 37
        NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 34, difference 5c
        NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 56, difference fa
        NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE  // Current byte 78, difference 88
        NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE  // Current byte 9a, difference ae
          CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte bc, difference c5
        
        // IP header
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 08, difference 19
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
          CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte 45, difference cf
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte 54, difference fc
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE  // Current byte 40, difference c0
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        E2R(0,0) E2R(1,1) E2R(2,2) E2R(3,3) E2R(4,4) E2R(5,5) E2R(6,6)   CHANGE  // Current byte 80, difference 80
        E2R(7,7)   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 01, difference 02
        
        //NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        //NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 00, difference 00
        FROMREG("r0")                                                            // Transmit the IP checksum. Most significant byte first
        FROMREG("r1")
        
        B(2,0)   NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte c0, difference 40
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte a8, difference f9
          CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0b
        NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE E2R(15,31) CHANGE  // Current byte 64, difference ac
        
        //NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte c0, difference 40
        //  CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE  // Current byte a8, difference f9
        //  CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte 06, difference 0b
        //  CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE  // Current byte ff, difference 01
        FROMREG("r3")
        FROMREG("r4")
        FROMREG("r5")
        FROMREG("r6")
        
        // ICMP header
        B(7,0)   E2R(0,8) E2R(1,9)E2R(2,11)E2R(3,12)E2R(4,13)E2R(5,14)E2R(6,15)  // Current byte 00, difference 01
       E2R(7,16)E2R(8,17)E2R(9,18)E2R(10,19)E2R(11,20)E2R(12,21)E2R(13,22)E2R(14,23)  // Current byte 00, difference 00
        FROMREG("r0")       // Checksum
        FROMREG("r1")
        FROMREG("r2")       // Identifier
        FROMREG("r3")
        FROMREG("r4")       // Sequence number
        FROMREG("r5")
        FROMREG("r6")       // Timestamp (last byte is assumed to be zero)
        FROMREG("r7")
        FROMREG("r8")
        FROMREG("r9")
        FROMREG("r10")
        FROMREG("r11")
        FROMREG("r12")
        B(13, 0)E2R(1,24)E2R(2,25)E2R(3,26)E2R(4,27)E2R(5,28)E2R(6,29)E2R(7,30)
        // Repeat the first 8 bytes of the data
        FROMREG("r14")
        FROMREG("r1")
        FROMREG("r2")
        FROMREG("r3")
        FROMREG("r4")
        FROMREG("r5")
        FROMREG("r6")
        FROMREG("r7")
        
        // Transmit the rest of the data (fixed)
        B(15, 0) NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 10, difference 31
          CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 11, difference 33
        NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 12, difference 36
          CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 13, difference 35
        NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 14, difference 3c
          CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 15, difference 3f
        NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 16, difference 3a
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 17, difference 39
        NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 18, difference 28
          CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 19, difference 2b
        NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 1a, difference 2e
          CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 1b, difference 2d
        NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 1c, difference 24
          CHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 1d, difference 27
        NOCHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 1e, difference 22
          CHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE  // Current byte 1f, difference 21
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 20, difference 60
          CHANGE   CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 21, difference 63
        NOCHANGE   CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 22, difference 66
          CHANGE NOCHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 23, difference 65
        NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 24, difference 6c
          CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 25, difference 6f
        NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 26, difference 6a
          CHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 27, difference 69
        NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 28, difference 78
          CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 29, difference 7b
        NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 2a, difference 7e
          CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 2b, difference 7d
        NOCHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 2c, difference 74
          CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 2d, difference 77
        NOCHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 2e, difference 72
          CHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE  // Current byte 2f, difference 71
        NOCHANGE NOCHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 30, difference 50
          CHANGE   CHANGE NOCHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 31, difference 53
        NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 32, difference 56
          CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 33, difference 55
        NOCHANGE NOCHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 34, difference 5c
          CHANGE   CHANGE   CHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 35, difference 5f
        NOCHANGE   CHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 36, difference 5a
          CHANGE NOCHANGE NOCHANGE   CHANGE   CHANGE NOCHANGE   CHANGE NOCHANGE  // Current byte 37, difference 59

        
        "packet_3_dataend:\n\t"
        FROMREG("r16")
        FROMREG("r17")
        FROMREG("r18")
        FROMREG("r19")
        "nop\n\t"
        "out 36, r24\n\t"
        "nop\n\tnop\n\tnop\n\t"
        "ldi r26, 0b00000010\n\t"
        "out 36, r26\n\t"
        "pop r31\n\tpop r30\n\tpop r29\n\tpop r28\n\tpop r27\n\tpop r26\n\tpop r25\n\tpop r24\n\tpop r23\n\tpop r22\n\tpop r19\n\tpop r18\n\tpop r17\n\tpop r16\n\t"
        "pop r15\n\tpop r14\n\tpop r13\n\tpop r12\n\tpop r11\n\tpop r10\n\tpop r9\n\tpop r8\n\tpop r7\n\tpop r6\n\tpop r5\n\tpop r4\n\tpop r3\n\tpop r2\n\tpop r1\n\tpop r0\n\t"
        :
        :
        :
    );
}
