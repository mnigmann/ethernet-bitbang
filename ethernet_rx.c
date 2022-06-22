#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include <packets.h>


// Ethernet header
#define dest_MAC1        0xFF
#define dest_MAC2        0xFF
#define dest_MAC3        0xFF
#define dest_MAC4        0xFF
#define dest_MAC5        0xFF
#define dest_MAC6        0xFF
#define source_MAC1        0x12
#define source_MAC2        0x34
#define source_MAC3        0x56
#define source_MAC4        0x78
#define source_MAC5        0x9A
#define source_MAC6        0xBC
#define type               0x0800

// IP header
#define version            0b0100
#define header_length      0b0101
#define DSCP               0b000000
#define ECN                0b00
#define ip_length          0b0000000000011101
#define packet_id          0b0000000000000000
#define flags              0b010
#define fragment           0b0000000000000
#define time_to_live       0b10000000
#define protocol           0b00010001

#define source_IP1         192
#define source_IP2         168
#define source_IP3         6
#define source_IP4         100

#define dest_IP1           192
#define dest_IP2           168
#define dest_IP3           6
#define dest_IP4           255

#define header_sum         ( ((version<<12)|(header_length<<8)|(DSCP<<2)|ECN) + ip_length + packet_id + ((flags<<13)|fragment) + ((time_to_live<<8)|protocol) + ((source_IP1<<8)|source_IP2) + ((source_IP3<<8)|source_IP4) + ((dest_IP1<<8)|dest_IP2) + ((dest_IP3<<8)|dest_IP4) ) 
#define header_checksum    ~(header_sum+(header_sum>>16))&0xFFFF

// UDP header
#define src_port           0b0000000000000000
#define dest_port          0b0001001110001000
#define udp_length         0b0000000000001001
#define checksum           0b0000000000000000


volatile uint8_t txbuf[512];
uint8_t pos[256];
uint8_t neg[256];
volatile unsigned int txbuflen = 0;
volatile unsigned int txidx = 0;

volatile uint8_t transmit_val = 0;

#define WAITTX while(!(UCSR0A&0b00100000));

//#define DEBUG_RECV

void print_newline() {
    UDR0 = '\r';
    WAITTX;
    UDR0 = '\n';
    WAITTX;
}

void print_str(char *str) {
    uint8_t len = strlen(str);
    uint8_t idx = 0;
    for (; idx < len; idx++) {
        UDR0 = str[idx];
        WAITTX;
    }
}

void print_byte(uint8_t val) {
    uint8_t b = 0;
    for (; b<8; b++) {
        UDR0 = (val & 1) + '0';
        val = val >> 1;
        WAITTX;
    }
}

void print_hex(uint8_t num) {
    UDR0 = (num >> 4) + (num >= 160?('A'-10):'0');
    while (!(UCSR0A & 0b00100000));
    UDR0 = (num & 0x0f) + ((num & 0x0f) >= 10?('A'-10):'0');
    while (!(UCSR0A & 0b00100000));
}

ISR(TIMER3_COMPA_vect) {
    /*asm volatile (
        "ldi %A0, lo8(packet_0_databegin)\n\t"
        "ldi %B0, hi8(packet_0_databegin)\n\t"
        : "=r" (addr)
        :
        :
    );*/
    //packet_0(&transmit_val);
    transmit_val ++;
    PCIFR = 0b00000010;

}

// When receiving
ISR(PCINT1_vect, ISR_NAKED) {
    asm volatile(
        "push r24\n\t"
        "push r25\n\t"
        "push r26\n\t"
        "in r24, 0x3f\n\t"
        "push r24\n\t"
        
        "ldi r24, 0x20\n\t"//"nop\n\t"
        "out 0x03, r24\n\t"//"nop\n\t"
        "out 0x03, r24\n\t"//"nop\n\t"
        
        "in r24, 0x09\n\t"
        "nop\n\t"
        "in r25, 0x09\n\t"
        "mov r26, r25\n\t"
        "or r26, 24\n\t"
        "sbrs r26, 2\n\t"
        "rjmp no_rx\n"
        "ldi r24, 0\n\t"
        
        "nop\n\t"
        "ldi r25, 0b00100000\n\t"
        "ldi r26, 0b00000000\n\t"
        
        "ldi r24, 12\n\t"
    "L4:\n\t"
            "dec r24\n\t"
            "cpi r24, 0\n\t"
            "brne L4\n\t"
        
        "out 0x2e, r26\n\t"
        //"nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "in r24, 0x2e\n\t"
        
        "sbrc r24, 7\n\t"
        "rjmp nowait\n\t"
        "nop\n\t"
        "nop\n\t"
    "nowait:\n\t"
        
        // Aligned code starts here
        "lds r24, 0xce\n\t"     // clear RX buffers
        "lds r24, 0x136\n\t"
        "sts 0xce, r26\n\t"
        "sts 0x136, r26\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
    "L5:\n\t"
        "sts 0xce, r26\n\t"
        "sts 0x136, r26\n\t"
        //"nop\n\tnop\n\tnop\n\tnop\n\t"
        "lds r24, 0xce\n\t"
        "st %a0+, r24\n\t"
        "lds r24, 0x136\n\t"
        "st %a0+, r24\n\t"
        "inc r26\n\t"
        "cpi r26, 0\n\t"
        "brne L5\n\t"
        
        
        /*"nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "out 0x2e, r26\n\t"     // Start an SPI transaction
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
    "L5:\n\t"
        "in r25, 0x03\n\t"
        "out 0x2e, r26\n\t"
        "in r24, 0x2e\n\t"
        "st %a0+, r24\n\t"
        "st %a0+, r25\n\t"
        "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
        "inc r26\n\t"
        "cpi r26, 0\n\t"
        "brne L5\n\t"*/
        
        
    //"ldi r24, 0b00101000\n\t"   // Enable USART UDRE interrupt
    //"sts 0xc1, r24\n\t"
    
    //"subi %B0, 2\n\t"           // subtract 512 from pointer before starting
    //"ld r24, %a0\n\t"
    //"sts 0xC6, r24\n\t"         // Put the first byte into the transmitter
    "ldi r24, 0\n\t"
    "sts txidx, r24\n\t"
    
    "ldi r24, 2\n\t"            // Store the length of the message in txbuflen
    "sts txbuflen+1, r24\n\t"
    "ldi r24, 0\n\t"
    "sts txbuflen, r24\n\t"
    
    "ldi r24, 0\n\t"
    "L6:\n\t"
            "dec r24\n\t"
            "nop\n\tnop\n\tnop\n\tnop\n\t"
            "cpi r24, 0\n\t"
            "brne L6\n\t"
        
    "no_rx:\n\t"
        "ldi r24, 0b00000001\n\t"
        "out 0x1b, r24\n\t"
        "pop r24\n\t"
        "out 0x3f, r24\n\t"
        "pop r26\n\t"
        "pop r25\n\t"
        "pop r24\n\t"
        "reti\n\t"
        :: "e" (txbuf)
    );
    
}

// Transmit the link test pulse
ISR(TIMER1_COMPA_vect) {
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
        "ldi r24, 0b11000010\n\t"
        "out 36, r24\n\t"
        "nop\n\t"
        "ldi r26, 2\n\t"
        "out 36, r26\n\t"
        ::: "r24", "r25", "r26"
    );
    PCIFR = 0b00000010;
}

void shift_r(uint8_t* array, uint8_t num) {
    num = 1<<num;
    asm volatile (
        "push r0\n\t"
        "push r1\n\t"
        "push r20\n\t"
        "push r21\n\t"
        "push r22\n\t"
        

        "ldi r20, 1\n\t"                // Loop counter
        
        "ld r22, %a1+\n\t"               // Load first byte into r22
        "mul r22, %0\n\t"               // Shift the first byte
        "mov r21, r1\n\t"               // Store the upper byte of the shifted result for the next cycle. The lower byte is discarded
        
        "shift_loop:\n\t"
            "ld r22, %a1\n\t"           // Load current byte into r22
            "sbiw %a1, 1\n\t"
            "mul r22, %0\n\t"           // Shift the current byte
            "or r21, r0\n\t"            // Or the lower byte of the shifted result onto the overflow from the previous byte
            "st %a1, r21\n\t"
            "adiw %a1, 2\n\t"
            "mov r21, r1\n\t"           // Store the upper byte of the shifted result for the next cycle
            
            
            "inc r20\n\t"
            "brbc 1, shift_loop\n\t"
        
        "pop r22\n\t"
        "pop r21\n\t"
        "pop r20\n\t"
        "pop r1\n\t"
        "pop r0\n\t"
        :
        : "r" (num), "e" (array)
        : "r0", "r1", "r24", "r25"
    );
}

uint8_t crccalc(uint8_t *array) {
    // Return the length of the substring of array that has a valid crc appended
    uint8_t a = 0, b, c;
    uint32_t crc = 0xffffffff;
    do {
        c = array[a];
        for (b=0; b<8; b++) {
            if ((crc ^ c) & 1) crc = (crc>>1) ^ 0xEDB88320;
            else crc = crc >> 1;
            c = c >> 1;
        }
        a++;
    } while (a > 0 && (crc != 0xdebb20e3 || a < 16));
    return a;
}

uint8_t sync_a;
uint8_t sync_b;

int main () {
    DDRB   = 0b10100011;
    DDRA   = 0b00001011;
    DDRD   = 0b00101000;    // Set XCK1 as output even though it is unconnected
    DDRH   = 0b00000100;    // Set XCK2 as output
    DDRG   = 0b00100000;
    DDRJ   = 0b00000100;
    PORTB |= 0b00000000;
    
    // Manchester coding
    TCCR0A = 0b00000010;
    TCCR0B = 0b00000001;
    OCR0A  = 0;
    
    // Transmit packet
    TCCR3A = 0b00000000;
    TCCR3B = 0b00001101;
    TCCR3C = 0b00000000;
    OCR3A  = 15124;
    TIMSK3 = 0b00000010;    // set to 0b00000010 to enable transmission
    
    // Link test pulse
    TCCR1A = 0b00000000;
    TCCR1B = 0b00001101;
    TCCR1C = 0b00000000;
    OCR1A  = 249;
    TIMSK1 = 0b00000010;
    
    // Interrupt for the input pin
    PCICR  = 0b00000010;
    PCMSK1 = 0b00000010;
    
    SPCR   = 0b01110000;
    SPSR   = 0b00000001;
    SPDR   = 0;
    
    // Configure USART0 as a UART at 500k
    UCSR0A = 0b00000010;
	UCSR0C = 0b00000110;
	UCSR0B = 0b00001000;        // Interrupts are disabled
	UBRR0  = 4;
	
	// Configure USART1 and USART2 in master SPI mode
	UCSR1C = 0b11000100;
	UCSR1B = 0b00011000;
	UBRR1  = 0;
	
	UCSR3C = 0b11000100;
	UCSR3B = 0b00011000;
	UBRR3  = 0;
	
	asm (
	    "ldi r24, 10\n\t"       // Try to shift XCK2 by one half-cycle so that XCK1 and XCK2 are of the opposite phase
	    "ldi r26, 0\n\t"
        "sts 0x134, r24\n\t"
        "sts 0x134, r26\n\t"
        ::: "r24", "r26"
    );
    
    sei();
    
    while (1) {
        if (txbuflen > 0) {
            uint16_t temp_len = txbuflen;
            txbuflen = 0;
            sync_a = txbuf[0];
            sync_b = txbuf[1];
            print_str("txbuflen is ");
            print_hex(0);
            print_hex(temp_len>>8);
            print_hex(temp_len&0xFF);
            print_newline();
            
#ifdef DEBUG_RECV
            print_str("before    ");
            for (txidx=0; txidx < temp_len; txidx+=2) {
                print_byte(txbuf[txidx]);
            }
            print_str("\r\nbefore    ");
            for (txidx=1; txidx < temp_len; txidx+=2) {
                print_byte(txbuf[txidx]);
            }
            print_str("\r\nsync_a: ");
            print_byte(sync_a);
            print_str("sync_b: ");
            print_byte(sync_b);
            print_newline();
#endif
            uint8_t shift_a = 0;
            uint8_t shift_b = 0;
            if (((sync_a & 0b11111100) ^ ((sync_b<<2) & 0b11111100)) == 0b11111100) {
                shift_b = 2;
                sync_b = sync_b << 2;
            } else if (((sync_a & 0b11111110) ^ ((sync_b<<1) & 0b11111110)) == 0b11111110) { 
                shift_b = 1;
                sync_b = sync_b << 1;
            } else if ((sync_a ^ sync_b) == 0b11111111) {
            } else if ((((sync_a<<1) & 0b11111110) ^ (sync_b & 0b11111110)) == 0b11111110) {
                shift_a = 1;
                sync_a = sync_a << 1;
            } else if ((((sync_a<<2) & 0b11111100) ^ (sync_b & 0b11111100)) == 0b11111100) {
                shift_a = 2;
                sync_a = sync_a << 2;
            }
            
            
            if (((0b00110101 & sync_a) == 0b00110101) || ((0b00110101 & sync_b) == 0b00110101)) {
                shift_a += 2;
                shift_b += 2;
                sync_a = sync_a << 2;
                sync_b = sync_b << 2;
            } else if (((0b01101010 & sync_a) == 0b01101010) || ((0b01101010 & sync_b) == 0b01101010)) {
                shift_a += 1;
                shift_b += 1;
                sync_a = sync_a << 1;
                sync_b = sync_b << 1;
            }
#ifdef DEBUG_RECV
            print_str("shift a by "); UDR0 = '0'+shift_a; WAITTX;
            print_str(", shift b by "); UDR0 = '0'+shift_b; WAITTX;
#endif
            uint8_t b = 0;
            if (sync_a > sync_b) {
                for (txidx = 0; txidx < temp_len; txidx+=2) {
                    pos[b] = txbuf[txidx];
                    neg[b++] = ~txbuf[txidx+1];
                }
                shift_r(pos, shift_a);
                shift_r(neg, shift_b);
            } else {
                for (txidx = 0; txidx < temp_len; txidx+=2) {
                    neg[b] = ~txbuf[txidx];
                    pos[b++] = txbuf[txidx+1];
                }
                shift_r(pos, shift_b);
                shift_r(neg, shift_a);
            }
            uint8_t pos_len = crccalc(pos);
            uint8_t neg_len = crccalc(neg);
#ifdef DEBUG_RECV
            print_str("\r\npos_len "); print_hex(pos_len);
            print_str("\r\nneg_len "); print_hex(neg_len);
#endif
            
            /*print_str("before    ");
            for (txidx=0; txidx < 256; txidx+=1) {
                print_hex(pos[txidx]);
            }
            print_str("\r\nbefore    ");
            for (txidx=0; txidx < 256; txidx+=1) {
                print_hex(neg[txidx]);
            }
            print_newline();*/
            if (neg_len > pos_len) {
                // neg secceeds, but pos doesn't. Copy neg into pos
                memcpy(pos, neg, 256);
            }
            print_str("\r\nreceived  "); 
            for (txidx=0; txidx < pos_len; txidx++) {
                print_hex(pos[txidx]);
            }
            print_newline();
            if (
                    pos_len >= 64 &&                                                            // Packet detected
                    pos[12] == 0x08 && pos[13] == 0x06 &&                                       // correct packet type (ARP)
                    pos[21] == 0x01 &&                                                          // ARP requests only
                    pos[38] == 192  && pos[39] == 168  && pos[40] == 6    && pos[41] == 100     // This IP only
            ) {
                print_str("ARP request found, tell ");
                print_hex(pos[22]); print_hex(pos[23]); print_hex(pos[24]); print_hex(pos[25]); print_hex(pos[26]); print_hex(pos[27]);
                //for (txidx = 22; txidx < 28; txidx++) print_hex(0);
                print_str(", (IP ");
                print_hex(pos[28]); print_hex(pos[29]); print_hex(pos[30]); print_hex(pos[31]);
                print_str(") "); print_hex(0); print_newline();
                cli();
                packet_2(pos + 28, pos + 22);
                sei();
            }
            
            else if (
                    pos_len >= 64 &&                                                            // Packet detected
                    pos[12] == 0x08 && pos[13] == 0x00 &&                                       // correct packet type (IPV4)
                    pos[23] == 0x01 &&                                                          // ICMP packet
                    pos[30] == 192  && pos[31] == 168  && pos[32] == 6    && pos[33] == 100 &&  // This IP only
                    pos[34] == 0x08                                                             // Ping requests only
            ) {
                print_str("Ping request found\r\n");
                // ICMP header is at position 38 and has length 12 (or 36 with length 14 when including checksum)
                // Compute the checksum by adding 0x0800 to pos[36]/pos[37]
                asm (
                    "lds r25, pos+36\n\t"
                    "lds r24, pos+37\n\t"
                    "adiw r24, 8\n\t"
                    "sbci r24, 0\n\t"
                    "sbci r25, 0\n\t"
                    "sts pos+36, r25\n\t"
                    "sts pos+37, r24\n\t"
                    ::: "r24", "r25"
                );
                
                cli();
                packet_3(pos + 6, pos + 26, pos+38);
                sei();
                
            }
            
            //print_newline();
            //print_newline();
            //print_newline();
            //print_newline();
            PCIFR = 0b00000010;
        }
    }
}