#include "../../Include/ProjectDefines.h"
#include "../../Include/UART/Uart.h"
#include <avr/interrupt.h>

#define BAUD 9600UL
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1UL)

// Ringbuffer, så tegn ikke overskriver hinanden, hvis de modtages hurtigt.
#define UART_BUFFER_SIZE 64

static volatile char uartBuffer[UART_BUFFER_SIZE];
static volatile uint8_t uartHead = 0;
static volatile uint8_t uartTail = 0;

void UART0_Init(void)
{
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;

    UCSR0A = 0;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART0_EnableReceiveInterrupt(void)
{
    UCSR0B |= (1 << RXCIE0);
}

bool UART0_IsDataAvailable(void)
{
    return uartHead != uartTail;
}

char UART0_GetChar(void)
{
    char receivedChar = 0;

    if (uartHead != uartTail)
    {
        receivedChar = uartBuffer[uartTail];
        uartTail = (uartTail + 1) % UART_BUFFER_SIZE;
    }

    return receivedChar;
}

void UART0_SendChar(char ch)
{
    while (!(UCSR0A & (1 << UDRE0)))
    {
        // Vent indtil UART0 er klar til at sende
    }

    UDR0 = ch;
}

void UART0_SendString(const char *str)
{
    while (*str != '\0')
    {
        UART0_SendChar(*str);
        str++;
    }
}

void UART0_SendNewLine(void)
{
    UART0_SendString("\r\n");
}

// Interrupt når UART0 modtager et tegn
ISR(USART0_RX_vect)
{
    char receivedChar = UDR0;
    uint8_t nextHead = (uartHead + 1) % UART_BUFFER_SIZE;

    // Gem tegnet, hvis der stadig er plads i buffer
    if (nextHead != uartTail)
    {
        uartBuffer[uartHead] = receivedChar;
        uartHead = nextHead;
    }
}