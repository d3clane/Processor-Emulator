#include <stdio.h>
#include <string.h>

#include "Processor.h"
#include "Stack.h"

struct ProcessorType
{
    StackType stack;
    //TODO: докинуть файл с байт кодом
};

static const char* PUSH   = "push";
static const char* DIV    =  "div";
static const char* MUL    =  "mul";
static const char* SUB    =  "sub";
static const char* ADD    =  "add";
static const char* OUT    =  "out";
static const char* HLT    =  "hlt";

static void GetTwoLastValuesFromStack(StackType* stk, ElemType* firstVal, ElemType* secondVal)
{
    assert(stk);
    assert(firstVal);
    assert(secondVal);

    StackPop(stk, secondVal);
    StackPop(stk, firstVal);
    //TODO: ретерн в случае ошибки
}

static void Push(StackType* stk)
{
    assert(stk);

    ElemType valueToPush = 0;
    int scanfResult = scanf(ElemTypeFormat, &valueToPush);

    if (scanfResult != 1)
        return;
    
    StackPush(stk, valueToPush); //как проверить ваще чет не выкупаю
}

static void Divide(StackType* stk)
{
    assert(stk);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    GetTwoLastValuesFromStack(stk, &firstValue, &secondValue);
    
    //Мб пихнуть проверку isfinite
    if (Equal(&firstValue, &POISON) || Equal(&secondValue, &POISON))
    {
        return; //TODO: ретерн ошибки что какая-то фигня хранится 
    }

    const ElemType Zero = 0; //TODO: что-то точно надо сделать с этим
    if (!Equal(&secondValue, &Zero))
    {        
        StackPush(stk, firstValue / secondValue); //TODO: тоже проверку бы что все прошло нормально
        return;
    }

    StackPush(stk, Zero); //если делить на ноль нельзя - хотя стоит мб обработать адекватней
    return; //TODO: ретерн ошибки
}

static void Multiply(StackType* stk)
{
    assert(stk);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    GetTwoLastValuesFromStack(stk, &firstValue, &secondValue);

    StackPush(stk, firstValue * secondValue); //TODO: ретерн на ошибке, проверка адекватности умножения
}

static void Subtract(StackType* stk)
{
    assert(stk);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    GetTwoLastValuesFromStack(stk, &firstValue, &secondValue);

    StackPush(stk, firstValue - secondValue); //TODO: 
}

static void Add(StackType* stk)
{
    assert(stk);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    GetTwoLastValuesFromStack(stk, &firstValue, &secondValue);

    StackPush(stk, firstValue + secondValue); //TODO: 
}

static void PrintResult(StackType* stk)
{
    assert(stk);

    ElemType equationResult = POISON;
    StackPop(stk, &equationResult);

    printf("Equation result: " ElemTypeFormat "\n", equationResult);
}

void Processing()
{
    static const size_t maxCommandLength  =  5;
    static char command[maxCommandLength] = "";
    
    StackType stk = {};
    StackCtor(&stk);

    while (true)
    {
        int scanfResult = scanf("%s", command);
        if (scanfResult == 0)
            return;
        
        if (strcmp(command, PUSH) == 0)
            Push(&stk);
        else if (strcmp(command, DIV) == 0)
            Divide(&stk);
        else if (strcmp(command, MUL) == 0)
            Multiply(&stk);
        else if (strcmp(command, SUB) == 0)
            Subtract(&stk);
        else if (strcmp(command, ADD) == 0)
            Add(&stk);
        else if (strcmp(command, OUT) == 0)
            PrintResult(&stk);
        else if (strcmp(command, HLT) == 0)
            return;
        else
            return;
    }
}
