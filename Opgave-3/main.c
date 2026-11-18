#include <avr/interrupt.h>
#include "Include/UART/Uart.h"
#include "Include/StateMachine/StateMachine.h"

int main(void)
{
    // Start UART og state machine
    UART0_Init();
    UART0_EnableReceiveInterrupt();

    StateMachine_Init();

    // Aktiver globale interrupts
    sei();

    while (1)
    {
        // Hent alle tegn, der ligger i UART-bufferen
        while (UART0_IsDataAvailable())
        {
            char receivedByte = UART0_GetChar();
            StateMachine_ProcessChar(receivedByte);
        }
    }
}