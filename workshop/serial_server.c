#include <stdint.h>
#include <stdio.h>
#include <sel4cp.h>

// This file is based off of the UART driver at:
// https://github.com/seL4/util_libs/blob/master/libplatsupport/src/plat/qemu-arm-virt/serial.c

#define RHR_MASK 0b111111111
#define UARTDR 0x000
#define UARTFR 0x018
#define UARTIMSC 0x038
#define UARTICR 0x044
#define PL011_UARTFR_TXFF (1 << 5)
#define PL011_UARTFR_RXFE (1 << 4)

#define REG_PTR(base, offset) ((volatile uint32_t *)((base) + (offset)))

uintptr_t uart_base_vaddr;

/* Start of my changes. */
#define SERIAL_SERVER_CHANNEL_ID (1)
#define UART_IRQ_CHANNEL_ID (0)
uintptr_t serial_server_buf;
uintptr_t client_buf;
/* End of my changes. */

void uart_init() {
    *REG_PTR(uart_base_vaddr, UARTIMSC) = 0x50;
}

int uart_get_char() {
    int ch = 0;

    if ((*REG_PTR(uart_base_vaddr, UARTFR) & PL011_UARTFR_RXFE) == 0) {
        ch = *REG_PTR(uart_base_vaddr, UARTDR) & RHR_MASK;
    }

    return ch;
}

void uart_put_char(int ch) {
    while ((*REG_PTR(uart_base_vaddr, UARTFR) & PL011_UARTFR_TXFF) != 0);

    *REG_PTR(uart_base_vaddr, UARTDR) = ch;
    if (ch == '\r') {
        uart_put_char('\n');
    }
}

void uart_put_str(char *str) {
    while (*str) {
        uart_put_char(*str);
        str++;
    }
}

void uart_handle_irq() {
    *REG_PTR(uart_base_vaddr, UARTICR) = 0x7f0;
}

void
init(void) {
    uart_init();
    // Use sel4cp_dbg_puts in case we mess up the implementation of talking to
    // UART, we should be able to rely on seL4's debug output.
    sel4cp_dbg_puts("SERIAL SERVER: starting\n");
}

void
notified(sel4cp_channel channel) {
    switch (channel) {
        case UART_IRQ_CHANNEL_ID: {
            /* Handle UART. */
            uart_handle_irq();
            /* Get character. */
            char ch = uart_get_char();
            /* Print character to serial device. */
            uart_put_char(ch);
            /* Acknowledge receipt of the interrupt. */
            sel4cp_irq_ack(channel);
            break;
        }
        case SERIAL_SERVER_CHANNEL_ID: {
            /* This will execute when client sends a message. */
            sel4cp_dbg_puts("Received message.");
            break;
        }
        default: {
            break;
        }
    }
}




