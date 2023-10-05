#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "assembler/Commands.h"
#include "SPU.h"
#include "Stack.h"
#include "InputOutput.h"

//TODO: пусть на возврате ошибок все принтится в лог файл(такую функцию запилить надо бы и все) 
//Просто функция принтов ошибок как в ассемблере и дизассемблере сделана 
//+ доделать, чтобы выводилась именно строчка, откуда call был функции на вывод ошибки(ну макрос лепануть хз)

struct SpuType
{
    StackType stack;

    FILE* byteCodeFile;
    char* byteCodeArray;
    char* byteCodeArrayReadPtr;

    ElemType registers[NumberOfRegisters];
};

static SpuErrors ProcessorCtor(SpuType* processor, FILE* inStream);
static SpuErrors ProcessorDtor(SpuType* processor);
static SpuErrors ProcessorDump(SpuType* processor);
static SpuErrors ProcessorVerify(SpuType* processor);

static SpuErrors CommandPush(SpuType* processor);
static SpuErrors CommandIn(SpuType* processor);

static SpuErrors CommandDiv(SpuType* processor);
static SpuErrors CommandMul(SpuType* processor);
static SpuErrors CommandSub(SpuType* processor);
static SpuErrors CommandAdd(SpuType* processor);

static SpuErrors CommandOut(SpuType* processor);

static SpuErrors GetTwoLastValuesFromStack(StackType* stack, ElemType* firstVal, ElemType* secondVal);

static inline bool IsValidValues(const ElemType* firstVal, const ElemType* secondVal);

#define IF_ERR_RETURN(ERROR)                                                      \
do                                                                                \
{                                                                                 \
    if ((ERROR) != SpuErrors::NO_ERR)                                             \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR((ERROR));                                            \
                      return (ERROR);                                             \
    }                                                                             \
} while (0)

SpuErrors Processing(FILE* inStream)
{
    assert(inStream);

    SpuType processor = {};
    ProcessorCtor(&processor, inStream);

    int command = -1;
    
    SpuErrors processorError = SpuErrors::NO_ERR;

    while (true)
    {
        command = (int) strtol(processor.byteCodeArrayReadPtr, 
                              &processor.byteCodeArrayReadPtr, 
                               10);

        bool quitCycle = false;

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
                quitCycle = true;
            default:
                processorError = SpuErrors::INVALID_COMMAND;
                SPU_ERRORS_LOG_ERROR(processorError);

                quitCycle = true;
                break;
        }

        if (processorError != SpuErrors::NO_ERR)
            break;

        if (quitCycle)
            break;
    }

    ProcessorDtor(&processor);
    
    IF_ERR_RETURN(processorError);

    return SpuErrors::NO_ERR;
}

#define RETURN_STACK_PUSH_ERR(PROCESSOR, VALUE_TO_PUSH)                           \
do                                                                                \
{                                                                                 \
    StackErrorsType stackErr = StackPush(&(PROCESSOR)->stack, VALUE_TO_PUSH);     \
                                                                                  \
    if (stackErr)                                                                 \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);                               \
                      return SpuErrors::STACK_ERR;                                \
    }                                                                             \
                                                                                  \
    return SpuErrors::NO_ERR;                                                     \
} while (0)

static SpuErrors CommandPush(SpuType* processor)
{
    assert(processor);
    assert(processor->byteCodeFile);

    ElemType valueToPush = 0;

    char* tmpByteCodePtr = nullptr;
    valueToPush = strtol(processor->byteCodeArrayReadPtr,
                        &tmpByteCodePtr,
                        10);
                    
    if (tmpByteCodePtr == processor->byteCodeArrayReadPtr)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::READING_FROM_BYTE_CODE_ERR);
                      return SpuErrors::READING_FROM_BYTE_CODE_ERR;
    }
    
    processor->byteCodeArrayReadPtr = tmpByteCodePtr;

    RETURN_STACK_PUSH_ERR(processor, valueToPush);
}

static SpuErrors CommandIn(SpuType* processor)
{
    assert(processor);
    assert(processor->byteCodeFile);

    ElemType valueToPush = 0;
    int scanfResult = scanf(ElemTypeFormat, &valueToPush);

    if (scanfResult != 1)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::READING_FROM_BYTE_CODE_ERR);
                      return SpuErrors::READING_FROM_BYTE_CODE_ERR;
    }
    
    RETURN_STACK_PUSH_ERR(processor, valueToPush);
}

#define VALUES_CHECK(FIRST_VALUE, SECOND_VALUE)                                   \
do                                                                                \
{                                                                                 \
    if (!IsValidValues(&(FIRST_VALUE), &(SECOND_VALUE)))                          \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR(SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC); \
                      return SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC;  \
    }                                                                             \
} while (0)

static SpuErrors CommandDiv(SpuType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    SpuErrors errors = GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(errors);
    
    VALUES_CHECK(firstValue, secondValue);

    const ElemType Zero = 0; //хз как-то некрасиво выглядит
    if (!Equal(&secondValue, &Zero))
    {        
        RETURN_STACK_PUSH_ERR(processor, firstValue / secondValue);
    }

    RETURN_STACK_PUSH_ERR(processor, INFINITY);
}

static SpuErrors CommandMul(SpuType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    SpuErrors errors = GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(errors);

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(processor, firstValue * secondValue);
}

static SpuErrors CommandSub(SpuType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    SpuErrors errors = GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(errors);

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(processor, firstValue - secondValue);
}

static SpuErrors CommandAdd(SpuType* processor)
{
    assert(processor);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    SpuErrors errors = GetTwoLastValuesFromStack(&processor->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(errors);

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(processor, firstValue + secondValue);
}

#undef VALUES_CHECK

static SpuErrors CommandOut(SpuType* processor)
{
    assert(processor);

    ElemType equationResult = POISON;

    StackErrorsType stackError = StackPop(&processor->stack, &equationResult);

    if (stackError)
    {
        printf("Couldn't provide result. Getting result error occurred.\n");
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    printf("Equation result: " ElemTypeFormat "\n", equationResult);

    //PUSH_ON_STACK_BACK
    RETURN_STACK_PUSH_ERR(processor, equationResult);
}

#undef RETURN_STACK_PUSH_ERR
#undef IF_ERR_RETURN

static SpuErrors GetTwoLastValuesFromStack(StackType* stack, ElemType* firstVal, ElemType* secondVal)
{
    assert(stack);
    assert(firstVal);
    assert(secondVal);


    StackErrorsType stackError = StackPop(stack, secondVal);
    stackError = StackPop(stack, firstVal);
    
    if (stackError)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    return SpuErrors::NO_ERR;
}

static SpuErrors ProcessorCtor(SpuType* processor, FILE* inStream)
{
    assert(processor);
    assert(inStream);

    processor->byteCodeFile         =          inStream;
    processor->byteCodeArray        = ReadText(inStream);
    processor->byteCodeArrayReadPtr = processor->byteCodeArray;

    StackErrorsType stackError = StackCtor(&processor->stack);

    if (stackError)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    return SpuErrors::NO_ERR;
}

static inline bool IsValidValues(const ElemType* firstVal, const ElemType* secondVal)
{
    return IsValidValue(firstVal) && IsValidValue(secondVal);
}

void SpuErrorsLogError(SpuErrors error, const char* fileName,
                                        const char* funcName,
                                        const int line)
{
    assert(fileName);
    assert(funcName);
    
    LOG_BEGIN();

    Log("Error occurred in file %s, function %s, line %d\n", fileName, funcName, line);

    switch(error)
    {
        case SpuErrors::NO_ERR:
            break;
        
        case SpuErrors::STACK_ERR:
            LOG_ERR("Error occurred in stack used for arithmetic.\n");
            break;
        
        case SpuErrors::READING_FROM_BYTE_CODE_ERR:
            LOG_ERR("Couldn't read from byte code file.\n");
            break;

        case SpuErrors::INVALID_COMMAND:
            LOG_ERR("Invalid command: can't execute it.\n");
            break;

        case SpuErrors::TRYING_TO_DIVIDE_ON_ZERO:
            LOG_ERR("User is trying to divide by zero.\n");
            break;
        case SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC:
            LOG_ERR("Values are not suitable for arithmetic operations.\n");
            break;
        
        default:
            LOG_ERR("Unknown error.\n");
            break;
    }

    LOG_END();
}
