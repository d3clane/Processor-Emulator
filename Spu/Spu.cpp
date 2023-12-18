#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "Common/Common.h"
#include "SPU.h"
#include "Stack/Stack.h"
#include "InputOutput/InputOutput.h"
#include "Common/Log.h"

const int CalculatingPrecision = 1e2;

//--------------SpuType functions--------------

static SpuErrors SpuCtor(SpuType* spu, FILE* inStream);
static SpuErrors SpuDtor(SpuType* spu);

#define SPU_DUMP(SPU) SpuDump((SPU), __FILE__, __func__, __LINE__)
static SpuErrors SpuDump(SpuType* spu, const char* fileName,
                                       const char* funcName,
                                       const int line);

static SpuErrors SpuVerify(SpuType* spu);

//-----------Other functions-------------

static int* ReadByteCode(FILE* inStream);

typedef SpuErrors BinaryCommandFunc(int, int, int*);
typedef SpuErrors  UnaryCommandFunc(int,      int*);

static SpuErrors CallBinaryCommand(BinaryCommandFunc* Command, SpuType* spu);
static SpuErrors CallUnaryCommand (UnaryCommandFunc*  Command, SpuType* spu);

static SpuErrors GetStackLastValue(StackType* stack, int* value);
static SpuErrors GetTwoLastValuesFromStack(StackType* stack, int* firstVal, int* secondVal);

static inline bool IsValidValues(const int* firstVal, const int* secondVal);

static int* GetArgument(SpuType* spu, int command);

//-----------defines------------------

#define IF_ERR_RETURN(ERROR)                                                      \
do                                                                                \
{                                                                                 \
    if ((ERROR) != SpuErrors::NO_ERR)                                             \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR((ERROR));                                            \
                      return (ERROR);                                             \
    }                                                                             \
} while (0)

//----------constants-------------

static const VersionType SpuVersion = 1; 

//--------functions realization------

/// @brief push and return error
#define STACK_PUSH(SPU, VALUE_TO_PUSH)                           \
do                                                                                \
{                                                                                 \
    StackErrorsType stackErr = StackPush(&(SPU)->stack, (VALUE_TO_PUSH));     \
                                                                                  \
    if (stackErr)                                                                 \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);                               \
                      return SpuErrors::STACK_ERR;                                \
    }                                                                             \
                                                                                  \
} while (0)

#define VALUES_CHECK(FIRST_VALUE, SECOND_VALUE)                                   \
do                                                                                \
{                                                                                 \
    if (!IsValidValues(&(FIRST_VALUE), &(SECOND_VALUE)))                          \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR(SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC); \
                      return SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC;  \
    }                                                                             \
} while (0)

#define VALUE_CHECK(inValue)                                                      \
do                                                                                \
{                                                                                 \
    if (!IsValidValue(&inValue))                                                  \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR(SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC); \
                      return SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC;  \
    }                                                                             \
} while (0)

/// @brief verifies spu and returns
#ifndef NDEBUG

    #define SPU_CHECK(SPU)                                  \
    do                                                      \
    {                                                       \
        SpuErrors spuErr = SpuVerify((SPU));                \
                                                            \
        if (spuErr != SpuErrors::NO_ERR)                    \
        {                                                   \
            SPU_DUMP((SPU));                                \
            SPU_ERRORS_LOG_ERROR(spuErr);                   \
            return spuErr;                                  \
        }                                                   \
    } while (0)
    
#else

    #define SPU_CHECK()

#endif

//-------_Creating functions--------------

#define DEF_CMD(NAME, v2, v3, functionCode, ...) functionCode

#include "Common/Commands.h"

#undef DEF_CMD

/// @brief Spu check with dtor
#define SPU_CHECK_WITH_DTOR(SPU)          \
do                                                              \
{                                                               \
    SpuErrors verifyingError = SpuVerify(&(SPU));   \
                                                                \
    if (verifyingError != SpuErrors::NO_ERR)                    \
    {                                                           \
        SPU_DUMP(&(SPU));                           \
                                                                \
        SpuDtor(&(SPU));                            \
                                                                \
        SPU_ERRORS_LOG_ERROR(verifyingError);                   \
                      return verifyingError;                    \
    }                                                           \
} while (0)

SpuErrors ExecuteByteCode(FILE* inStream)
{
    assert(inStream);

    SpuType spu = {};
    SpuErrors spuError = SpuCtor(&spu, inStream);

    if (spuError != SpuErrors::NO_ERR)
    {
        SPU_ERRORS_LOG_ERROR(spuError);

        SpuDtor(&spu);

        return spuError;
    }

    spuError = ExecuteByteCode(&spu);

    SpuDtor(&spu);

    IF_ERR_RETURN(spuError);

    return SpuErrors::NO_ERR;
}

#undef SPU_CHECK_WITH_DTOR

SpuErrors ExecuteByteCode(SpuType* spu)
{
    assert(spu);

    int command = -1;
    SpuErrors spuError = SpuErrors::NO_ERR;
    while (true)
    {
        command = spu->byteCode[spu->ip];
        spu->ip++;

        if (command == (int)Commands::HLT)
            break;

        //printf("Command id - %d\n", command);
        #define DEF_CMD(NAME, v1, v2, v3, code, ...)\
            case Commands::NAME:                    \
            {                                       \
                code;                               \
                break;                              \
            }

        switch((Commands)(GetCommand(command)))
        {
            #include "Common/Commands.h"

            default:
                spuError = SpuErrors::INVALID_COMMAND;
                SPU_ERRORS_LOG_ERROR(spuError);

                break;
        }

        //printf("(\n");

        #undef DEF_CMD

        if (spuError != SpuErrors::NO_ERR)
            break;
        
        SPU_CHECK(spu);
        //printf("NO_ERR2\n");
    }

    return SpuErrors::NO_ERR;
}

static int* GetArgument(SpuType* spu, int command)
{
    static int value = 0;
    value = 0;

    int* res = nullptr;
    bool hasArgs = false;

    if (command & ARG_FORMAT_IMM)
    {
        //sprintf("HERE\n");
        hasArgs = true;
        res     = nullptr;
        value   = spu->byteCode[spu->ip];

        //TODO: добавить в commands.h инфу о том, нужно ли домножение тут.
        if (GetCommand(command) == (int)Commands::PUSH || GetCommand(command) == (int)Commands::POP)
            value *= CalculatingPrecision;

        spu->ip++;
    }

    if (command & ARG_FORMAT_REG)
    {
        hasArgs = true;
        assert(spu->byteCode[spu->ip] < NumberOfRegisters);

        res    = &spu->registers[spu->byteCode[spu->ip]];
        value +=  spu->registers[spu->byteCode[spu->ip]];
        spu->ip++;
    }

    if (command & ARG_FORMAT_RAM)
    {
        hasArgs = true;

        assert(0 <= value && value / CalculatingPrecision < RamSize);

        res   = &spu->ram[value / CalculatingPrecision];
        value =  spu->ram[value / CalculatingPrecision];
    }

    if (!hasArgs)
        return nullptr;
    //TODO: мб надо что-то добавить в commands.h, чтобы оно это обрабатывало
    // здесь как будто условие, что что-то достает откуда-то, а это буквально 1 операция у меня
    if (GetCommand(command) == (int)Commands::POP)
        return res;
    else
        return &value;
    
    return nullptr;
}

static SpuErrors CallUnaryCommand (UnaryCommandFunc* Command, SpuType* spu)
{
    assert(Command);
    assert(spu);

    SPU_CHECK(spu);

    int value = POISON;

    SpuErrors error = GetStackLastValue(&spu->stack, &value);

    IF_ERR_RETURN(error);

    if (!IsValidValue(&value))
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC);
                      return SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC;
    }

    int commandResult = POISON;
    error = Command(value, &commandResult);

    STACK_PUSH(spu, commandResult);

    SPU_CHECK(spu);

    return error;
}

static SpuErrors CallBinaryCommand(BinaryCommandFunc* Command, SpuType* spu)
{
    assert(Command);
    assert(spu);

    SPU_CHECK(spu);

    int firstValue  = POISON;
    int secondValue = POISON;

    SpuErrors error = GetTwoLastValuesFromStack(&spu->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(error);

    if (!IsValidValues(&firstValue, &secondValue))
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC);
                      return SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC;        
    }

    int commandResult = POISON;
    error = Command(firstValue, secondValue, &commandResult);    

    STACK_PUSH(spu, commandResult);

    SPU_CHECK(spu);
    return error;
}

#undef STACK_PUSH
#undef IF_ERR_RETURN

static SpuErrors GetStackLastValue(StackType* stack, int* value)
{
    assert(stack);
    assert(value);

    StackErrorsType stackError = StackPop(stack, value);

    if (stackError)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    return SpuErrors::NO_ERR;
}

static SpuErrors GetTwoLastValuesFromStack(StackType* stack, int* firstVal, int* secondVal)
{
    assert(stack);
    assert(firstVal);
    assert(secondVal);

    StackErrorsType stackError  = StackPop(stack, secondVal);
                    stackError |= StackPop(stack, firstVal);

    if (stackError)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    return SpuErrors::NO_ERR;
}

static inline bool IsValidValues(const int* firstVal, const int* secondVal)
{
    return IsValidValue(firstVal) && IsValidValue(secondVal);
}

//----Spu structure functions--------

static SpuErrors SpuCtor(SpuType* spu, FILE* inStream)
{
    assert(spu);
    assert(inStream);

    spu->byteCode = ReadByteCode(inStream);
    spu->ip       = 0;

    StackErrorsType stackError =  StackCtor(&spu->stack);
                    stackError |= StackCtor(&spu->stackRet);

    SpuErrors error = (stackError ? SpuErrors::STACK_ERR : SpuErrors::NO_ERR);

    if (error != SpuErrors::NO_ERR)
    {
        SPU_ERRORS_LOG_ERROR(error);
                      return error;
    }

    if (stackError)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;
}

static SpuErrors SpuDtor(SpuType* spu)
{
    assert(spu);

    free(spu->byteCode);
    spu->byteCode = nullptr;
    spu->ip = 0;

    StackErrorsType stackError  = StackDtor(&spu->stack);
                    stackError |= StackDtor(&spu->stackRet);
    if (stackError)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    return SpuErrors::NO_ERR;
}

static SpuErrors SpuDump(SpuType* spu, const char* fileName,
                                       const char* funcName,
                                       const int line)
{
    assert(spu);

    LOG_BEGIN();

    Log("Spu dump called from file %s, function %s, line %d\n", fileName, funcName, line);

    Log("Bytecode array beginning adress      : %p\n", spu->byteCode);
    Log("Bytecode array adress pointer to read: %p\n", spu->byteCode + spu->ip);
    
    if (spu->byteCode) 
        Log("Bytecode array info: %s\n\n", spu->byteCode);

    if (spu->byteCode + spu->ip) 
        Log("Bytecode array info from last pointer: \n%s\n\n", spu->byteCode + spu->ip);

    StackDump(&spu->stack, fileName, funcName, line);

    LOG_END();

    return SpuErrors::NO_ERR;
}

static SpuErrors SpuVerify(SpuType* spu)
{
    assert(spu);

    if (!spu->byteCode)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::BYTE_CODE_ARR_IS_NULLPTR);
                      return SpuErrors::BYTE_CODE_ARR_IS_NULLPTR;
    }

    if (StackVerify(&spu->stack) != 0)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    if (StackVerify(&spu->stackRet) != 0)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    return SpuErrors::NO_ERR;
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

        case SpuErrors::BYTE_CODE_ARR_IS_NULLPTR:
            LOG_ERR("Byte code array is nullptr.\n");
            break;
        case SpuErrors::BYTE_CODE_ARR_PTR_OUT_OF_RANGE:
            LOG_ERR("Byte code reading pointer is out of range.\n");
            break;
        
        case SpuErrors::INVALID_SIGNATURE:
            LOG_ERR("Invalid signature.\n");
            break;
        case SpuErrors::INVALID_VERSION:
            LOG_ERR("Invalid version.\n");
            break;

        case SpuErrors::INVALID_REGISTER:
            LOG_ERR("Invalid register id.\n");
            break;
        
        default:
            LOG_ERR("Unknown error.\n");
            break;
    }

    LOG_END();
}

static int* ReadByteCode(FILE* inStream)
{
    assert(inStream);

    size_t byteCodeFileSize = GetFileSize(inStream);

    size_t byteCodeArrSize = byteCodeFileSize / sizeof(int) + 1;
    int* byteCode = (int*) calloc(byteCodeArrSize, sizeof(*byteCode));

    fread(byteCode, sizeof(*byteCode), byteCodeArrSize, inStream);

    return byteCode;    
}

#undef SPU_CHECK