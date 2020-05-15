#include <mach/uart.h>
#include <mach/gpio.h>
#include <asm/io.h>

void uart_send (char c)
{
	// wait for transmit FIFO to have an available slot
	while(readl(U_FR_REG) & (1<<5)) { }

	writel(c, U_DATA_REG);
}

char uart_recv ()
{
	// wait for receive FIFO to have data to read
	while(readl(U_FR_REG) & (1<<4)) { }

	return(readl(U_DATA_REG) & 0xFF);
}

void uart_send_string(char* str) { for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char) str[i]);
	}
}

void uart_init (void)
{
	unsigned int selector;

	selector = readl(GPFSEL1); selector &= ~(7<<12);                   // clean gpio14
	selector |= 4<<12;                      // set alt0 for gpio14
	selector &= ~(7<<15);                   // clean gpio15
	selector |= 4<<15;                      // set alt0 for gpio 15
	writel(selector, GPFSEL1);

	writel(0, GPPUD);
	delay(150);
	writel((1<<14) | (1<<15), GPPUDCLK0);
	delay(150);
	writel(0, GPPUDCLK0);

    
	writel(0, U_CR_REG);                    // disable UART until configuration is done

	/* baud divisor = UARTCLK / (16 * baud_rate) 
	                = 48 * 10^6 / (16 * 115200) = 26.0416666667
       integer part = 26
	   fractional part = (int) ((0.0416666667 * 64) + 0.5) 
	                   = 3
	   generated baud rate divisor = 26 + (3 / 64) = 26.046875
       generated baud rate = (48 * 10^6) / (16 * 26.046875) = 115177
	   error = |(115177 - 115200) / 115200 * 100| = 0.02%
	*/
	writel(26, U_IBRD_REG);                 // baud rate divisor, integer part
	writel(3, U_FBRD_REG);                  // baud rate divisor, fractional part
	
	writel((1<<4) | (3<<5), U_LCRH_REG);    // enable FIFOs and 8 bits frames
	writel(0, U_IMSC_REG);                  // mask interupts
	writel(1 | (1<<8) | (1<<9), U_CR_REG);  // enable UART, receive and transmit
}
