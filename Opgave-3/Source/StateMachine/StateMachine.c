#include "../../Include/StateMachine/StateMachine.h"
#include "../../Include/UART/Uart.h"
#include <stdint.h>
#include <stddef.h>

#define ADDRESS_BUFFER_SIZE 9

static StateMachineState currentState;

static char addressBuffer[ADDRESS_BUFFER_SIZE];
static uint8_t addressIndex = 0;

static uint16_t receivedAddress = 0;
static uint8_t receivedBitPosition = 0;
static uint8_t receivedBitValue = 0;

static volatile uint8_t *registerPointer = 0;

static void ChangeState(StateMachineState newState)
{
    currentState = newState;

    switch (currentState)
    {
        case Modtag_Adresse:
            UART0_SendString("Tilstand: Modtag_Adresse");
            break;

        case Modtag_Bit_Position:
            UART0_SendString("Tilstand: Modtag_Bit_Position");
            break;

        case Modtag_Bit_Vaerdi:
            UART0_SendString("Tilstand: Modtag_Bit_Vaerdi");
            break;
    }

    UART0_SendNewLine();
}

static void ResetAddressInput(void)
{
    addressIndex = 0;
    addressBuffer[0] = '\0';
}

static bool IsHexCharacter(char ch)
{
    return ((ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'F') ||
            (ch >= 'a' && ch <= 'f') ||
            ch == 'x' || ch == 'X'); // Tillad også 'x' til 0x-præfiks
}

static bool IsAllowedRegisterAddress(uint16_t address)
{
    // Begræns adgang til de to registre, der bruges i opgaveeksemplet.
    // DDRB = 0x0024 og PORTB = 0x0025.
    return (address == 0x0024 || address == 0x0025);
}

static bool ParseHexStringToUint16(const char *str, uint16_t *outValue)
{
    unsigned long val = 0;
    
    // Håndter valgfrit "0x" eller "0X" præfiks
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
    }
    
    if (*str == '\0')
    {
        return false;
    }

    while (*str != '\0')
    {
        char c = *str;
        uint8_t digit = 0;

        if (c >= '0' && c <= '9')
        {
            digit = c - '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            digit = c - 'A' + 10;
        }
        else if (c >= 'a' && c <= 'f')
        {
            digit = c - 'a' + 10;
        }
        else
        {
            return false; // Ugyldigt tegn
        }

        val = (val << 4) | digit;
        if (val > 0xFFFF)
        {
            return false; // For stort til 16-bit
        }
        str++;
    }

    *outValue = (uint16_t)val;
    return true;
}

static void PrintBitPattern(uint8_t value)
{
    for (int8_t bit = 7; bit >= 0; bit--)
    {
        if (value & (1 << bit))
        {
            UART0_SendChar('1');
        }
        else
        {
            UART0_SendChar('0');
        }

        if (bit == 4)
        {
            UART0_SendChar(' ');
        }
    }
}

static void PrintResult(void)
{
    UART0_SendString("Adresse: 0x");

    // Udskriv adressen som fire hexadecimale cifre
    const char hexDigits[] = "0123456789ABCDEF";
    UART0_SendChar(hexDigits[(receivedAddress >> 12) & 0x0F]);
    UART0_SendChar(hexDigits[(receivedAddress >> 8) & 0x0F]);
    UART0_SendChar(hexDigits[(receivedAddress >> 4) & 0x0F]);
    UART0_SendChar(hexDigits[receivedAddress & 0x0F]);
    UART0_SendNewLine();

    UART0_SendString("Bit position: ");
    UART0_SendChar('0' + receivedBitPosition);
    UART0_SendNewLine();

    UART0_SendString("Bit vaerdi: ");
    UART0_SendChar('0' + receivedBitValue);
    UART0_SendNewLine();

    UART0_SendString("Bitmoenster: ");
    PrintBitPattern(*registerPointer);
    UART0_SendNewLine();
    UART0_SendNewLine();
}

static void SetOrClearBit(void)
{
    uint8_t bitMask = (1 << receivedBitPosition);

    if (receivedBitValue == 1)
    {
        // Sæt kun den valgte bit høj
        *registerPointer |= bitMask;
    }
    else
    {
        // Sæt kun den valgte bit lav
        *registerPointer &= (uint8_t)(~bitMask);
    }

    PrintResult();
    ResetAddressInput();
    ChangeState(Modtag_Adresse);
}

static void ProcessAddressCharacter(char receivedChar)
{
    if (receivedChar == ':')
    {
        if (addressIndex == 0)
        {
            UART0_SendString("Fejl: Ingen adresse indtastet.");
            UART0_SendNewLine();
            ChangeState(Modtag_Adresse);
            return;
        }

        addressBuffer[addressIndex] = '\0';
        uint16_t parsedAddress = 0;

        if (!ParseHexStringToUint16(addressBuffer, &parsedAddress) ||
            !IsAllowedRegisterAddress(parsedAddress))
        {
            UART0_SendString("Ugyldig adresse. Brug 0024 eller 0025.");
            UART0_SendNewLine();
            ResetAddressInput();
            ChangeState(Modtag_Adresse);
            return;
        }

        receivedAddress = parsedAddress;
        registerPointer = (volatile uint8_t *)(uintptr_t)receivedAddress;

        ResetAddressInput();
        ChangeState(Modtag_Bit_Position);
    }
    else if (IsHexCharacter(receivedChar) && addressIndex < ADDRESS_BUFFER_SIZE - 1)
    {
        addressBuffer[addressIndex] = receivedChar;
        addressIndex++;
    }
    else
    {
        UART0_SendString("Ugyldigt adressetegn. Proev igen.");
        UART0_SendNewLine();
        ResetAddressInput();
        ChangeState(Modtag_Adresse);
    }
}

void StateMachine_Init(void)
{
    ResetAddressInput();
    ChangeState(Modtag_Adresse);
}

void StateMachine_ProcessChar(char receivedChar)
{
    switch (currentState)
    {
        case Modtag_Adresse:
            ProcessAddressCharacter(receivedChar);
            break;

        case Modtag_Bit_Position:
            if (receivedChar >= '0' && receivedChar <= '7')
            {
                receivedBitPosition = (uint8_t)(receivedChar - '0');
                ChangeState(Modtag_Bit_Vaerdi);
            }
            else
            {
                UART0_SendString("Ugyldig bit position. Brug 0-7.");
                UART0_SendNewLine();
                ResetAddressInput();
                ChangeState(Modtag_Adresse);
            }
            break;

        case Modtag_Bit_Vaerdi:
            if (receivedChar == '0' || receivedChar == '1')
            {
                receivedBitValue = (uint8_t)(receivedChar - '0');
                SetOrClearBit();
            }
            else
            {
                UART0_SendString("Ugyldig bit vaerdi. Brug 0 eller 1.");
                UART0_SendNewLine();
                ResetAddressInput();
                ChangeState(Modtag_Adresse);
            }
            break;
    }
}