#include <console/loglevel.h>

static void __console_tx_byte(unsigned char byte)
{
	uart_tx_byte(byte);
}

static void __console_tx_nibble(unsigned nibble)
{
	unsigned char digit;
	digit = nibble + '0';
	if (digit > '9') {
		digit += 39;
	}
	__console_tx_byte(digit);
}

static void __console_tx_char(int loglevel, unsigned char byte)
{
	if (ASM_CONSOLE_LOGLEVEL > loglevel) {
		uart_tx_byte(byte);
	}
}

static void __console_tx_hex8(int loglevel, unsigned char value)
{
	if (ASM_CONSOLE_LOGLEVEL > loglevel) {
		__console_tx_nibble((value >>  4U) & 0x0fU);
		__console_tx_nibble(value & 0x0fU);
	}
}

static void __console_tx_hex16(int loglevel, unsigned short value)
{
	if (ASM_CONSOLE_LOGLEVEL > loglevel) {
		__console_tx_nibble((value >> 12U) & 0x0fU);
		__console_tx_nibble((value >>  8U) & 0x0fU);
		__console_tx_nibble((value >>  4U) & 0x0fU);
		__console_tx_nibble(value & 0x0fU);
	}
}

static void __console_tx_hex32(int loglevel, unsigned int value)
{
	if (ASM_CONSOLE_LOGLEVEL > loglevel) {
		__console_tx_nibble((value >> 28U) & 0x0fU);
		__console_tx_nibble((value >> 24U) & 0x0fU);
		__console_tx_nibble((value >> 20U) & 0x0fU);
		__console_tx_nibble((value >> 16U) & 0x0fU);
		__console_tx_nibble((value >> 12U) & 0x0fU);
		__console_tx_nibble((value >>  8U) & 0x0fU);
		__console_tx_nibble((value >>  4U) & 0x0fU);
		__console_tx_nibble(value & 0x0fU);
	}
}

static void __console_tx_string(int loglevel, const char *str)
{
	if (ASM_CONSOLE_LOGLEVEL > loglevel) {
		unsigned char ch;
		while((ch = *str++) != '\0') {
			__console_tx_byte(ch);
		}
	}
}

#define NOINLINE __attribute__((noinline))
static void print_emerg_char(unsigned char byte) { __console_tx_char(BIOS_EMERG, byte); }
static void print_emerg_hex8(unsigned char value){ __console_tx_hex8(BIOS_EMERG, value); }
static void print_emerg_hex16(unsigned short value){ __console_tx_hex16(BIOS_EMERG, value); }
static void print_emerg_hex32(unsigned int value) { __console_tx_hex32(BIOS_EMERG, value); }
static void print_emerg(const char *str) { __console_tx_string(BIOS_EMERG, str); }

static void print_alert_char(unsigned char byte) { __console_tx_char(BIOS_ALERT, byte); }
static void print_alert_hex8(unsigned char value) { __console_tx_hex8(BIOS_ALERT, value); }
static void print_alert_hex16(unsigned short value){ __console_tx_hex16(BIOS_ALERT, value); }
static void print_alert_hex32(unsigned int value) { __console_tx_hex32(BIOS_ALERT, value); }
static void print_alert(const char *str) { __console_tx_string(BIOS_ALERT, str); }

static void print_crit_char(unsigned char byte) { __console_tx_char(BIOS_CRIT, byte); }
static void print_crit_hex8(unsigned char value) { __console_tx_hex8(BIOS_CRIT, value); }
static void print_crit_hex16(unsigned short value){ __console_tx_hex16(BIOS_CRIT, value); }
static void print_crit_hex32(unsigned int value) { __console_tx_hex32(BIOS_CRIT, value); }
static void print_crit(const char *str) { __console_tx_string(BIOS_CRIT, str); }

static void print_err_char(unsigned char byte) { __console_tx_char(BIOS_ERR, byte); }
static void print_err_hex8(unsigned char value) { __console_tx_hex8(BIOS_ERR, value); }
static void print_err_hex16(unsigned short value){ __console_tx_hex16(BIOS_ERR, value); }
static void print_err_hex32(unsigned int value) { __console_tx_hex32(BIOS_ERR, value); }
static void print_err(const char *str) { __console_tx_string(BIOS_ERR, str); }

static void print_warning_char(unsigned char byte) { __console_tx_char(BIOS_WARNING, byte); }
static void print_warning_hex8(unsigned char value) { __console_tx_hex8(BIOS_WARNING, value); }
static void print_warning_hex16(unsigned short value){ __console_tx_hex16(BIOS_WARNING, value); }
static void print_warning_hex32(unsigned int value) { __console_tx_hex32(BIOS_WARNING, value); }
static void print_warning(const char *str) { __console_tx_string(BIOS_WARNING, str); }

static void print_notice_char(unsigned char byte) { __console_tx_char(BIOS_NOTICE, byte); }
static void print_notice_hex8(unsigned char value) { __console_tx_hex8(BIOS_NOTICE, value); }
static void print_notice_hex16(unsigned short value){ __console_tx_hex16(BIOS_NOTICE, value); }
static void print_notice_hex32(unsigned int value) { __console_tx_hex32(BIOS_NOTICE, value); }
static void print_notice(const char *str) { __console_tx_string(BIOS_NOTICE, str); }

static void print_info_char(unsigned char byte) { __console_tx_char(BIOS_INFO, byte); }
static void print_info_hex8(unsigned char value) { __console_tx_hex8(BIOS_INFO, value); }
static void print_info_hex16(unsigned short value){ __console_tx_hex16(BIOS_INFO, value); }
static void print_info_hex32(unsigned int value) { __console_tx_hex32(BIOS_INFO, value); }
static void print_info(const char *str) { __console_tx_string(BIOS_INFO, str); }

static void print_debug_char(unsigned char byte) { __console_tx_char(BIOS_DEBUG, byte); }
static void print_debug_hex8(unsigned char value) { __console_tx_hex8(BIOS_DEBUG, value); }
static void print_debug_hex16(unsigned short value){ __console_tx_hex16(BIOS_DEBUG, value); }
static void print_debug_hex32(unsigned int value) { __console_tx_hex32(BIOS_DEBUG, value); }
static void print_debug(const char *str) { __console_tx_string(BIOS_DEBUG, str); }

static void print_spew_char(unsigned char byte) { __console_tx_char(BIOS_SPEW, byte); }
static void print_spew_hex8(unsigned char value) { __console_tx_hex8(BIOS_SPEW, value); }
static void print_spew_hex16(unsigned short value){ __console_tx_hex16(BIOS_SPEW, value); }
static void print_spew_hex32(unsigned int value) { __console_tx_hex32(BIOS_SPEW, value); }
static void print_spew(const char *str) { __console_tx_string(BIOS_SPEW, str); }

/* Non inline versions.... */

static void print_alert_char_(unsigned char value) NOINLINE   { print_alert_char(value); }
static void print_alert_hex8_(unsigned char value) NOINLINE   { print_alert_hex8(value); }
static void print_alert_hex16_(unsigned short value) NOINLINE { print_alert_hex16(value); }
static void print_alert_hex32_(unsigned int value) NOINLINE   { print_alert_hex32(value); }
static void print_alert_(const char *str) NOINLINE            { print_alert(str); }

static void print_crit_char_(unsigned char value) NOINLINE   { print_crit_char(value); }
static void print_crit_hex8_(unsigned char value) NOINLINE   { print_crit_hex8(value); }
static void print_crit_hex16_(unsigned short value) NOINLINE { print_crit_hex16(value); }
static void print_crit_hex32_(unsigned int value) NOINLINE   { print_crit_hex32(value); }
static void print_crit_(const char *str) NOINLINE            { print_crit(str); }

static void print_err_char_(unsigned char value) NOINLINE   { print_err_char(value); }
static void print_err_hex8_(unsigned char value) NOINLINE   { print_err_hex8(value); }
static void print_err_hex16_(unsigned short value) NOINLINE { print_err_hex16(value); }
static void print_err_hex32_(unsigned int value) NOINLINE   { print_err_hex32(value); }
static void print_err_(const char *str) NOINLINE            { print_err(str); }

static void print_warning_char_(unsigned char value) NOINLINE   { print_warning_char(value); }
static void print_warning_hex8_(unsigned char value) NOINLINE   { print_warning_hex8(value); }
static void print_warning_hex16_(unsigned short value) NOINLINE { print_warning_hex16(value); }
static void print_warning_hex32_(unsigned int value) NOINLINE   { print_warning_hex32(value); }
static void print_warning_(const char *str) NOINLINE            { print_warning(str); }

static void print_notice_char_(unsigned char value) NOINLINE   { print_notice_char(value); }
static void print_notice_hex8_(unsigned char value) NOINLINE   { print_notice_hex8(value); }
static void print_notice_hex16_(unsigned short value) NOINLINE { print_notice_hex16(value); }
static void print_notice_hex32_(unsigned int value) NOINLINE   { print_notice_hex32(value); }
static void print_notice_(const char *str) NOINLINE            { print_notice(str); }

static void print_info_char_(unsigned char value) NOINLINE   { print_info_char(value); }
static void print_info_hex8_(unsigned char value) NOINLINE   { print_info_hex8(value); }
static void print_info_hex16_(unsigned short value) NOINLINE { print_info_hex16(value); }
static void print_info_hex32_(unsigned int value) NOINLINE   { print_info_hex32(value); }
static void print_info_(const char *str) NOINLINE            { print_info(str); }

static void print_debug_char_(unsigned char value) NOINLINE   { print_debug_char(value); }
static void print_debug_hex8_(unsigned char value) NOINLINE   { print_debug_hex8(value); }
static void print_debug_hex16_(unsigned short value) NOINLINE { print_debug_hex16(value); }
static void print_debug_hex32_(unsigned int value) NOINLINE   { print_debug_hex32(value); }
static void print_debug_(const char *str) NOINLINE            { print_debug(str); }

static void print_spew_char_(unsigned char value) NOINLINE   { print_spew_char(value); }
static void print_spew_hex8_(unsigned char value) NOINLINE   { print_spew_hex8(value); }
static void print_spew_hex16_(unsigned short value) NOINLINE { print_spew_hex16(value); }
static void print_spew_hex32_(unsigned int value) NOINLINE   { print_spew_hex32(value); }
static void print_spew_(const char *str) NOINLINE            { print_spew(str); }

#ifndef LINUXBIOS_EXTRA_VERSION
#define LINUXBIOS_EXTRA_VERSION ""
#endif

static void console_init(void)
{
	static const char console_test[] = 
		"\r\n\r\nLinuxBIOS-"
		LINUXBIOS_VERSION
		LINUXBIOS_EXTRA_VERSION
		" "
		LINUXBIOS_BUILD
		" starting...\r\n";
	print_info(console_test);
}


static void die(const char *str)
{
	print_emerg(str);
	do {
		hlt();
	} while(1);
}
