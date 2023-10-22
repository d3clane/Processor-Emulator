#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "../Common/Common.h"
#include "SPU.h"
#include "../Stack/Stack.h"
#include "../InputOutput/InputOutput.h"
#include "../Common/Log.h"

const int CalculatingPrecision = 1e2;

//--------------SpuType functions--------------

static SpuErrors SpuCtor(SpuType* spu, FILE* inStream);
static SpuErrors SpuDtor(SpuType* spu);

#define SPU_DUMP(SPU) SpuDump((SPU), __FILE__, __func__, __LINE__)
static SpuErrors SpuDump(SpuType* spu, const char* fileName,
                                       const char* funcName,
                                       const int line);

static SpuErrors SpuVerify(SpuType* spu);

//--------------Spu commands--------------

static SpuErrors CommandPushRegister(SpuType* spu);
static SpuErrors CommandPush        (SpuType* spu);
static SpuErrors CommandIn          (SpuType* spu);
static SpuErrors CommandPop         (SpuType* spu);

static inline SpuErrors CommandDiv(int inFirstValue, int inSecondValue, int* outValue);
static inline SpuErrors CommandMul(int inFirstValue, int inSecondValue, int* outValue);
static inline SpuErrors CommandSub(int inFirstValue, int inSecondValue, int* outValue);
static inline SpuErrors CommandAdd(int inFirstValue, int inSecondValue, int* outValue);

static inline SpuErrors CommandSin (int inValue, int* outValue);
static inline SpuErrors CommandCos (int inValue, int* outValue);
static inline SpuErrors CommandTan (int inValue, int* outValue);
static inline SpuErrors CommandCot (int inValue, int* outValue);
static inline SpuErrors CommandPow2 (int inValue, int* outValue);
static inline SpuErrors CommandSqrt(int inValue, int* outValue);

static SpuErrors CommandMeow ();
static SpuErrors CommandBark ();
static SpuErrors CommandSleep();
static SpuErrors CommandBotay();

static inline SpuErrors CommandOut(int inValue, int* outValue);

//----------Added info functions-------

static inline SpuErrors FileVerify(SpuType* spu);
static inline SpuErrors SkipAddedInfo(SpuType* spu);

//-----------Other functions-------------

typedef SpuErrors BinaryCommandFunc(int, int, int*);
typedef SpuErrors  UnaryCommandFunc(int,      int*);

static SpuErrors CallBinaryCommand(BinaryCommandFunc* Command, SpuType* spu);
static SpuErrors CallUnaryCommand (UnaryCommandFunc*  Command, SpuType* spu);

static SpuErrors GetStackLastValue(StackType* stack, int* value);
static SpuErrors GetTwoLastValuesFromStack(StackType* stack, int* firstVal, int* secondVal);

static inline bool IsValidValues(const int* firstVal, const int* secondVal);

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

    #define SPU_CHECK(SPU)                      \
    do                                                      \
    {                                                       \
        SpuErrors spuErr = SpuVerify((SPU));  \
                                                            \
        if (spuErr != SpuErrors::NO_ERR)                  \
        {                                                   \
            SPU_DUMP((SPU));                    \
            SPU_ERRORS_LOG_ERROR(spuErr);                 \
            return spuErr;                                \
        }                                                   \
    } while (0)
    
#else

    #define SPU_CHECK()

#endif

//-------_Creating functions--------------

#define DEF_CMD(name, num, asmCode, argId, switchCode, functionCode, ...)  functionCode

#include "../Common/Commands.h"

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
              spuError = SkipAddedInfo(&spu);

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

#define DEF_CMD(name, num, asmCode, argId, code, ...)  \
    case Commands::name ##_ID:                         \
        code;                                          \
        break;

SpuErrors ExecuteByteCode(SpuType* spu)
{
    assert(spu);

    int command = -1;
    SpuErrors spuError = SpuErrors::NO_ERR;
    while (true)
    {
        bool quitCycle = false;

        command = *spu->byteCodeArrayReadPtr++;

        switch((Commands) command)
        {

        #include "../Common/Commands.h"

        default:
            spuError = SpuErrors::INVALID_COMMAND;
            SPU_ERRORS_LOG_ERROR(spuError);

            quitCycle = true;
            break;
        }

        if (spuError != SpuErrors::NO_ERR)
            break;

        SPU_CHECK(spu);

        if (quitCycle)
            break;
    }

    return SpuErrors::NO_ERR;
}

#undef DEF_CMD

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

    StackErrorsType stackError = StackPop(stack, secondVal);
    stackError = StackPop(stack, firstVal);
    
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

    spu->byteCodeArray        = ReadByteCode(inStream, &spu->byteCodeArraySize);
    spu->byteCodeArrayReadPtr = spu->byteCodeArray;

    StackErrorsType stackError = StackCtor(&spu->stack);
    SpuErrors error = FileVerify(spu);

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

    free(spu->byteCodeArray);
    spu->byteCodeArray = spu->byteCodeArrayReadPtr = nullptr;

    StackErrorsType stackError = StackDtor(&spu->stack);

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

    Log("Bytecode array beginning adress      : %p\n", spu->byteCodeArray);
    Log("Bytecode array adress pointer to read: %p\n", spu->byteCodeArrayReadPtr);
    
    if (spu->byteCodeArray) 
        Log("Bytecode array info: %s\n\n", spu->byteCodeArray);

    if (spu->byteCodeArrayReadPtr) 
        Log("Bytecode array info from last pointer: \n%s\n\n", spu->byteCodeArrayReadPtr);

    StackDump(&spu->stack, fileName, funcName, line);

    LOG_END();

    return SpuErrors::NO_ERR;
}

static SpuErrors SpuVerify(SpuType* spu)
{
    assert(spu);

    if (!spu->byteCodeArray)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::BYTE_CODE_ARR_IS_NULLPTR);
                      return SpuErrors::BYTE_CODE_ARR_IS_NULLPTR;
    }

    if (!spu->byteCodeArrayReadPtr)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::BYTE_CODE_ARR_PTR_OUT_OF_RANGE);
                      return SpuErrors::BYTE_CODE_ARR_PTR_OUT_OF_RANGE;           
    }

    if (StackVerify(&spu->stack) != 0)
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

static inline SpuErrors SkipAddedInfo(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);


    spu->byteCodeArrayReadPtr += AddedInfoSizeByteCode;

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;
}

static inline SpuErrors FileVerify(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int* byteCodeArrayReadPtr = spu->byteCodeArrayReadPtr;

    SignatureType fileSignature = (SignatureType) byteCodeArrayReadPtr[SIGNATURE_INFO_POSITION];

    if (fileSignature != Signature)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_SIGNATURE);
                      return SpuErrors::INVALID_SIGNATURE;
    }

    VersionType fileVersion = (VersionType) byteCodeArrayReadPtr[VERSION_INFO_POSITION];
    
    if (fileVersion != SpuVersion)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_VERSION);
                      return SpuErrors::INVALID_VERSION;
    }

    return SpuErrors::NO_ERR;
}

#undef SPU_CHECK
