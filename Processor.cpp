#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "Processor.h"
#include "Stack.h"
#include "assembler/Commands.h"

struct ProcessorType
{
    StackType stack;
    FILE* byteCode;
};

static ProcessorErrors ProcessorCtor(ProcessorType* processor, FILE* inStream);

static ProcessorErrors CommandPush(ProcessorType* processor);
static ProcessorErrors CommandIn(ProcessorType* processor);

static ProcessorErrors CommandDiv(ProcessorType* processor);
static ProcessorErrors CommandMul(ProcessorType* processor);
static ProcessorErrors CommandSub(ProcessorType* processor);
static ProcessorErrors CommandAdd(ProcessorType* processor);

static ProcessorErrors CommandOut(ProcessorType* processor);

static ProcessorErrors GetTwoLastValuesFromStack(StackType* stack, ElemType* firstVal, ElemType* secondVal);

static inline bool IsValidValues(const ElemType* firstVal, const ElemType* secondVal);

ProcessorErrors Processing(FILE* inStream)
{
    assert(inStream);

    ProcessorType processor = {};
    ProcessorCtor(&processor, inStream);

    int command = -1;
    
    while (true)
    {
        fscanf(inStream, "%d", &command);

        ProcessorErrors processorError = ProcessorErrors::NO_ERR;

        switch((Commands) command)
        {
            case Commands::PUSH_ID:
                processorError = CommandPush(&processor);
                break;
            case Commands::IN_ID:
                processorError = CommandIn(&processor);
                break;
            case Commands::DIV_ID:
                processorError = CommandDiv(&processor);
                break;
            case Commands::ADD_ID:
                processorError = CommandAdd(&processor);
                break;
            case Commands::SUB_ID:
                processorError = CommandSub(&processor);
                break;
            case Commands::MUL_ID:
                processorError = CommandMul(&processor);
                break;
            case Commands::OUT_ID:
                processorError = CommandOut(&processor);
                break;
            case Commands::HLT_ID:
                return ProcessorErrors::NO_ERR;
            default:
                return ProcessorErrors::INVALID_COMMAND; //TODO: error handler
        }

        if (processorError != ProcessorErrors::NO_ERR)
            return processorError;
    }
}

#define RETURN_STACK_PUSH_ERR(PROCESSOR, VALUE_TO_PUSH)                           \
do                                                                                \
{                                                                                 \
    StackErrorsType stackErr = StackPush(&(PROCESSOR)->stack, VALUE_TO_PUSH);     \
    return stackErr ? ProcessorErrors::STACK_ERR : ProcessorErrors::NO_ERR;       \
} while (0)

static ProcessorErrors CommandPush(ProcessorType* processor)
{
    assert(processor);
    assert(processor->byteCode);

    ElemType valueToPush = 0;
    int scanfResult = fscanf(processor->byteCode, ElemTypeFormat, &valueToPush);

    if (scanfResult != 1)
        return ProcessorErrors::READING_FROM_FILE_ERROR;
    
    RETURN_STACK_PUSH_ERR(processor, valueToPush);
}

static ProcessorErrors CommandIn(ProcessorType* processor)
{
    assert(processor);
    assert(processor->byteCode);

    ElemType valueToPush = 0;
    int scanfResult = scanf(ElemTypeFormat, &valueToPush);

    if (scanfResult != 1)
        return ProcessorErrors::READING_FROM_FILE_ERROR;;
    
    RETURN_STACK_PUSH_ERR(processor, valueToPush);
}

#define VALUES_CHECK(FIRST_VALUE, SECOND_VALUE)                             \
do                                                                          \
{                                                                           \
    if (!IsValidValues(&(FIRST_VALUE), &(SECOND_VALUE)))                    \
    {                                                                       \
        return ProcessorErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC;    \
    }                                                                       \
} while (0)

static ProcessorErrors CommandDiv(ProcessorType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    ProcessorErrors errors = GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    if (errors != ProcessorErrors::NO_ERR)
        return errors;
    
    VALUES_CHECK(firstValue, secondValue);

    const ElemType Zero = 0; //хз как-то некрасиво выглядит
    if (!Equal(&secondValue, &Zero))
    {        
        RETURN_STACK_PUSH_ERR(processor, firstValue / secondValue);
    }

    RETURN_STACK_PUSH_ERR(processor, INFINITY);
}

static ProcessorErrors CommandMul(ProcessorType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    ProcessorErrors errors = GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    if (errors != ProcessorErrors::NO_ERR)
        return errors;

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(processor, firstValue * secondValue);
}

static ProcessorErrors CommandSub(ProcessorType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    ProcessorErrors errors = GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    if (errors != ProcessorErrors::NO_ERR)
        return errors;

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(processor, firstValue - secondValue);
}

static ProcessorErrors CommandAdd(ProcessorType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    ProcessorErrors errors = GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    if (errors != ProcessorErrors::NO_ERR)
        return errors;

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(processor, firstValue + secondValue);
}

#undef VALUES_CHECK

static ProcessorErrors CommandOut(ProcessorType* processor)
{
    assert(processor);

    ElemType equationResult = POISON;

    StackErrorsType stackError = StackPop(&processor->stack, &equationResult);

    if (stackError)
    {
        printf("Couldn't provide result. Getting result error occurred.\n");
        return ProcessorErrors::STACK_ERR;
    }

    printf("Equation result: " ElemTypeFormat "\n", equationResult);

    RETURN_STACK_PUSH_ERR(processor, equationResult);
}

#undef RETURN_STACK_PUSH_ERR

static ProcessorErrors GetTwoLastValuesFromStack(StackType* stack, ElemType* firstVal, ElemType* secondVal)
{
    assert(stack);
    assert(firstVal);
    assert(secondVal);


    StackErrorsType stackError = StackPop(stack, secondVal);
    stackError = StackPop(stack, firstVal);
    
    return stackError ? ProcessorErrors::STACK_ERR : ProcessorErrors::NO_ERR;
}

static ProcessorErrors ProcessorCtor(ProcessorType* processor, FILE* inStream)
{
    assert(processor);
    assert(inStream);

    processor->byteCode = inStream;

    StackErrorsType stackError = StackCtor(&processor->stack);

    return stackError ? ProcessorErrors::STACK_ERR : ProcessorErrors::NO_ERR;
}

static inline bool IsValidValues(const ElemType* firstVal, const ElemType* secondVal)
{
    return IsValidValue(firstVal) && IsValidValue(secondVal);
}
