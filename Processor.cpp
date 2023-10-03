#include <assert.h>
#include <string.h>

#include "Processor.h"
#include "Stack.h"
#include "assembler/Commands.h"

struct ProcessorType
{
    StackType stack;
    FILE* byteCode;
    //TODO: докинуть файл с байт кодом
};

static void ProcessorCtor(ProcessorType* processor, FILE* inStream)
{
    assert(processor);
    assert(inStream);

    StackCtor(&processor->stack);
    processor->byteCode = inStream;
}

static void Push(ProcessorType* processor);
static void In(ProcessorType* processor);

static void Divide(ProcessorType* processor);
static void Multiply(ProcessorType* processor);
static void Subtract(ProcessorType* processor);
static void Add(ProcessorType* processor);

static void PrintResult(ProcessorType* processor);

static void GetTwoLastValuesFromStack(StackType* stack, ElemType* firstVal, ElemType* secondVal);

void Processing(FILE* inStream)
{
    assert(inStream);


    ProcessorType processor = {};
    ProcessorCtor(&processor, inStream);

    int command = -1;
    
    while (true)
    {
        fscanf(inStream, "%d", &command);

        switch((Commands) command)
        {
            case Commands::PUSH_ID:
                Push(&processor);
                break;
            case Commands::IN_ID:
                In(&processor);
                break;
            case Commands::DIV_ID:
                Divide(&processor);
                break;
            case Commands::ADD_ID:
                Add(&processor);
                break;
            case Commands::SUB_ID:
                Subtract(&processor);
                break;
            case Commands::MUL_ID:
                Multiply(&processor);
                break;
            case Commands::OUT_ID:
                PrintResult(&processor);
                break;
            case Commands::HLT_ID:
                return;
            default:
                return; //TODO: error handler
        }
    }
}

static void Push(ProcessorType* processor)
{
    assert(processor);
    assert(processor->byteCode);

    ElemType valueToPush = 0;
    int scanfResult = fscanf(processor->byteCode, ElemTypeFormat, &valueToPush);

    if (scanfResult != 1)
        return;
    
    StackPush(&processor->stack, valueToPush); //проверить
}

static void In(ProcessorType* processor)
{
    
    assert(processor);
    assert(processor->byteCode);

    ElemType valueToPush = 0;
    int scanfResult = scanf(ElemTypeFormat, &valueToPush);

    if (scanfResult != 1)
        return;
    
    StackPush(&processor->stack, valueToPush); //проверить
}

static void Divide(ProcessorType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    //Мб пихнуть проверку isfinite
    if (Equal(&firstValue, &POISON) || Equal(&secondValue, &POISON))
    {
        return; //TODO: ретерн ошибки что какая-то фигня хранится 
    }

    const ElemType Zero = 0; //TODO: что-то точно надо сделать с этим
    if (!Equal(&secondValue, &Zero))
    {        
        StackPush(&processor->stack, firstValue / secondValue); //TODO: тоже проверку бы что все прошло нормально
        return;
    }

    StackPush(&processor->stack, Zero); //если делить на ноль нельзя - хотя стоит мб обработать адекватней
    return; //TODO: ретерн ошибки
}

static void Multiply(ProcessorType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    StackPush(&processor->stack, firstValue * secondValue); //TODO: ретерн на ошибке, проверка адекватности умножения
}

static void Subtract(ProcessorType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    StackPush(&processor->stack, firstValue - secondValue); //TODO: 
}

static void Add(ProcessorType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    StackPush(&processor->stack, firstValue + secondValue); //TODO: 
}

static void PrintResult(ProcessorType* processor)
{
    assert(processor);

    ElemType equationResult = POISON;
    StackPop(&processor->stack, &equationResult);

    printf("Equation result: " ElemTypeFormat "\n", equationResult);

    StackPush(&processor->stack, equationResult); 
}

static void GetTwoLastValuesFromStack(StackType* stack, ElemType* firstVal, ElemType* secondVal)
{
    assert(stack);
    assert(firstVal);
    assert(secondVal);

    StackPop(stack, secondVal);
    StackPop(stack, firstVal);
    //TODO: ретерн в случае ошибки
}
