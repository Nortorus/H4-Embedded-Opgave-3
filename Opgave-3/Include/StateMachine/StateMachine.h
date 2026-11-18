#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

typedef enum
{
    Modtag_Adresse,
    Modtag_Bit_Position,
    Modtag_Bit_Vaerdi
} StateMachineState;

void StateMachine_Init(void);
void StateMachine_ProcessChar(char receivedChar);

#endif