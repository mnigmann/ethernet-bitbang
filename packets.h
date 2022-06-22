#include <stdint.h>

#define CHANGE "nop\n\tout 38, r26\n\t"
#define CHANGE7 CHANGE CHANGE CHANGE CHANGE CHANGE CHANGE CHANGE
#define CHANGE8 CHANGE7 CHANGE
#define NOCHANGE "nop\n\tnop\n\t"

#define XSTR(x) STR(x)
#define STR(x) #x

#define B(r, b) "sbrc " XSTR(r) ", " XSTR(b) "\n\tout 38, r26\n\t"
#define NB(r,b) "sbrs " XSTR(r) ", " XSTR(b) "\n\tout 38, r26\n\t"
#define FROMREG_B(rname, b) "sbrc " rname ", " b "\n\tout 38, r26\n\t"
#define FROMREG(rname) FROMREG_B(rname, "0") FROMREG_B(rname, "1") FROMREG_B(rname, "2") FROMREG_B(rname, "3") FROMREG_B(rname, "4") FROMREG_B(rname, "5") FROMREG_B(rname, "6") FROMREG_B(rname, "7")
#define FROMREG_INV0(rname) "sbrs " rname ", 0\n\tout 38, r26\n\t" FROMREG_B(rname, "1") FROMREG_B(rname, "2") FROMREG_B(rname, "3") FROMREG_B(rname, "4") FROMREG_B(rname, "5") FROMREG_B(rname, "6") FROMREG_B(rname, "7")
#define E2R(r, idx) "lds " XSTR(r) ", ethtxbuf+" XSTR(idx) "\n\t"
#define E2G(idx) "lds r20, ethtxbuf+" XSTR(idx) "\n\tout 0x1e, r20\n\tnop\n\t"
#define L(idx) "sbic 0x1e, " XSTR(idx) "\n\tout 38, r26\n\t"

#define ADDCHECK(cvar, val) {temp = cvar; cvar += val; if (cvar < temp) cvar++;}

void _print_hex(uint8_t num);

void packet_0();
void packet_1();
void packet_2();
void packet_3();