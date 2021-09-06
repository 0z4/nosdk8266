#include "c_types.h"
#include "esp8266_auxrom.h"
//#include "esp8266_rom.h"
#include "eagle_soc.h"
//#include "ets_sys.h"
#include "nosdk8266.h"
#include "nosdki2s.h"
//#include <math.h>

// TODO: Use float number (related to 8) to fix the drift
#define call_delay_us(time) { asm volatile("mov.n a2, %0\n_call0 delay4clk" : : "r"(time * (MAIN_MHZ / 8)) : "a2" ); }

extern volatile int isrs;
extern struct sdio_queue i2sBufDesc[2];

int is_prime(unsigned int n) {
   	if (n <= 1) {
   		return 0; // zero and one are not prime
   	}
   	unsigned int i = 0;
   	for (i = 2; i * i <= n; i++) {
       	if (n % i == 0) {
       		return 0;
       	}
    }
    return 1;
}  

int main() {
	int i = 0;
	nosdk8266_init();

	//int j = 0, primes = 0, p_to_search = 100000;
	//call_delay_us(5000000);
	//printf("START\n");
	//for (j = 0; j <= p_to_search; j++) {
	//	if (is_prime(j) == 1) {
	//		primes++;
	//	}
	//}
	//printf("END (%d / %d are prime numbers)", primes, p_to_search);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);
	PIN_DIR_OUTPUT = _BV(2); //Enable GPIO2 light off.

	printf("Trying a Flash test write.\n");
	Cache_Read_Disable();
	SPIUnlock();
	SPIEraseBlock(0x40000 >> 16);
	uint32_t stuff = 0xAABBCCDD;
	SPIWrite(0x40004, &stuff, 4);
	stuff = 0;
	SPIRead(0x40004, &stuff, 4);
	Cache_Read_Enable(0, 0, 1);

	printf("Checking to see if we can read from cache: %p / %p (both should be 0xaabbccdd)\n", *(void **)(0x40200000 + 0x40000 + 4), stuff);

	printf("Starting I2S Engine (%p)\n", *(void**)0x400040bc);
	InitI2S();

	//call_delay_us( 3000000 );
	//int j = 0;
	//for (j = 0; j < 200; j++) {
	//	uart_div_modify(0, (j * 1000000) / 115200);
	//	printf("pllworkingfreq: %d ", j);
	//}

	while(1) {
		SLC_INT_CLRL = -1;
		SendI2S();
		PIN_OUT_SET = _BV(2); //Turn GPIO2 light off.
		call_delay_us(1000000);
		//float aa = 12.34;
		//printf("float test: %f\n", aa);
		printf("Hello World %d / %d  %p\n", i, isrs, SLC_INT_RAWL);
		//printf("PLL divider register values: (1)0x%x | (2)0x%x\n", rom_i2c_readReg(103, 4, 1), rom_i2c_readReg(103, 4, 2));
		PIN_OUT_CLEAR = _BV(2); //Turn GPIO2 light off.
		call_delay_us(1000000);
		i++;
	}
}

