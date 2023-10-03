#include <assert.h>
#include <string.h>

#include "Processor.h"
#include "Stack.h"
#include "assembler/Commands.h"

struct ProcessorType
{
    StackType stack;
    //TODO: докинуть файл с байт кодом
};

static void Push(StackType* stk, FILE* inStream);
static void In(StackType* stk);

static void Divide(StackType* stk);
static void Multiply(StackType* stk);
static void Subtract(StackType* stk);
static void Add(StackType* stk);

static void PrintResult(StackType* stk);

static void GetTwoLastValuesFromStack(StackType* stk, ElemType* firstVal, ElemType* secondVal);

void Processing(FILE* inStream)
{
    assert(inStream);

    StackType stk = {};
    StackCtor(&stk);

    int command = -1;
    
    while (true)
    {
        fscanf(inStream, "%d", &command);

        switch((Commands) command)
        {
            case Commands::PUSH_ID:
                Push(&stk, inStream);
                break;
            case Commands::IN_ID:
                In(&stk);
                break;
            case Commands::DIV_ID:
                Divide(&stk);
                break;
            case Commands::ADD_ID:
                Add(&stk);
                break;
            case Commands::SUB_ID:
                Subtract(&stk);
                break;
            case Commands::MUL_ID:
                Multiply(&stk);
                break;
            case Commands::OUT_ID:
                PrintResult(&stk);
                break;
            case Commands::HLT_ID:
                return;
            default:
                return; //TODO: error handler
            
        }
    }
}

static void Push(StackType* stk, FILE* inStream)
{
    assert(stk);
    assert(inStream);

    ElemType valueToPush = 0;
    int scanfResult = fscanf(inStream, ElemTypeFormat, &valueToPush);

    if (scanfResult != 1)
        return;
    
    StackPush(stk, valueToPush); //как проверить ваще чет не выкупаю
}

static void In(StackType* stk)
{
    Push(stk, stdin);
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

static void GetTwoLastValuesFromStack(StackType* stk, ElemType* firstVal, ElemType* secondVal)
{
    assert(stk);
    assert(firstVal);
    assert(secondVal);

    StackPop(stk, secondVal);
    StackPop(stk, firstVal);
    //TODO: ретерн в случае ошибки
}
