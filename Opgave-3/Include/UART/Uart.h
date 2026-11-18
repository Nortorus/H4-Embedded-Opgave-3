#ifndef UART_H_
#define UART_H_

#include <stdbool.h>

void UART0_Init(void);
void UART0_EnableReceiveInterrupt(void);
bool UART0_IsDataAvailable(void);
char UART0_GetChar(void);
void UART0_SendChar(char ch);
void UART0_SendString(const char *str);
void UART0_SendNewLine(void);

#endif