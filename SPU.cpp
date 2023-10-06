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

//--------------SpuType functions--------------

static SpuErrors SpuCtor(SpuType* spu, FILE* inStream);
static SpuErrors SpuDtor(SpuType* spu);

#define SPU_DUMP(SPU) SpuDump((SPU), __FILE__, __func__, __LINE__)
static SpuErrors SpuDump(SpuType* spu, const char* fileName,
                                                   const char* funcName,
                                                   const int line);

static SpuErrors SpuVerify(SpuType* spu);

//--------------Spu commands--------------

static SpuErrors CommandPush(SpuType* spu);
static SpuErrors CommandIn(SpuType* spu);

static SpuErrors CommandDiv(SpuType* spu);
static SpuErrors CommandMul(SpuType* spu);
static SpuErrors CommandSub(SpuType* spu);
static SpuErrors CommandAdd(SpuType* spu);

static SpuErrors CommandOut(SpuType* spu);

//----------Added info functions-------

static inline SpuErrors CheckAddedInfo(SpuType* spu);
static inline SpuErrors SkipAddedInfo(SpuType* spu);

//-----------Other functions-------------

static SpuErrors GetTwoLastValuesFromStack(StackType* stack, ElemType* firstVal, ElemType* secondVal);

static inline bool IsValidValues(const ElemType* firstVal, const ElemType* secondVal);

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

/// @brief Spu check with dtor
#define SPU_CHECK_PROCESSING_FUNCTION(SPU)          \
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

//----------constants-------------

static const VersionType SpuVersion = 1; 

//--------functions realization------

SpuErrors Processing(FILE* inStream)
{
    assert(inStream);

    SpuType spu = {};
    SpuCtor(&spu, inStream);

    SpuErrors addedInfoError = CheckAddedInfo(&spu);

    if (addedInfoError != SpuErrors::NO_ERR)
    {
        SPU_ERRORS_LOG_ERROR(addedInfoError);

        SpuDtor(&spu);

        return addedInfoError;
    }

    SkipAddedInfo(&spu);
    
    int command = -1;
    
    SpuErrors SpuError = SpuErrors::NO_ERR;

    SPU_CHECK_PROCESSING_FUNCTION(spu);

    while (true)
    {
        command = (int) strtol(spu.byteCodeArrayReadPtr, 
                              &spu.byteCodeArrayReadPtr, 
                               10);

        bool quitCycle = false;

        switch((Commands) command)
        {
            case Commands::PUSH_ID:
                SpuError = CommandPush(&spu);
                break;
            case Commands::IN_ID:
                SpuError = CommandIn(&spu);
                break;
            case Commands::DIV_ID:
                SpuError = CommandDiv(&spu);
                break;
            case Commands::ADD_ID:
                SpuError = CommandAdd(&spu);
                break;
            case Commands::SUB_ID:
                SpuError = CommandSub(&spu);
                break;
            case Commands::MUL_ID:
                SpuError = CommandMul(&spu);
                break;
            case Commands::OUT_ID:
                SpuError = CommandOut(&spu);
                break;
            case Commands::HLT_ID:
                quitCycle = true;
                break;
            default:
                SpuError = SpuErrors::INVALID_COMMAND;
                SPU_ERRORS_LOG_ERROR(SpuError);

                quitCycle = true;
                break;
        }

        if (SpuError != SpuErrors::NO_ERR)
            break;
        
        SPU_CHECK_PROCESSING_FUNCTION(spu);

        if (quitCycle)
            break;
    }

    SpuDtor(&spu);

    IF_ERR_RETURN(SpuError);

    return SpuErrors::NO_ERR;
}

#undef SPU_CHECK_PROCESSING_FUNCTION

/// @brief verifies spu and returns
#ifndef NDEBUG

    #define SPU_CHECK(SPU)                      \
    do                                                      \
    {                                                       \
        SpuErrors spuError = SpuVerify((SPU));  \
                                                            \
        if (spuError != SpuErrors::NO_ERR)                  \
        {                                                   \
            SPU_DUMP((SPU));                    \
            SPU_ERRORS_LOG_ERROR(spuError);                 \
            return spuError;                                \
        }                                                   \
    } while (0)
    
#else

    #define SPU_CHECK()

#endif

/// @brief push and return error
#define RETURN_STACK_PUSH_ERR(SPU, VALUE_TO_PUSH)                           \
do                                                                                \
{                                                                                 \
    StackErrorsType stackErr = StackPush(&(SPU)->stack, VALUE_TO_PUSH);     \
                                                                                  \
    if (stackErr)                                                                 \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);                               \
                      return SpuErrors::STACK_ERR;                                \
    }                                                                             \
                                                                                  \
    SPU_CHECK((SPU));                                                 \
                                                                                  \
    return SpuErrors::NO_ERR;                                                     \
} while (0)

static SpuErrors CommandPush(SpuType* spu)
{
    assert(spu);
    assert(spu->byteCodeFile);

    SPU_CHECK(spu);

    ElemType valueToPush = 0;

    char* tmpByteCodePtr = nullptr;
    //TODO: in types.h add something to automatic call of strtox;
    valueToPush = strtod(spu->byteCodeArrayReadPtr, &tmpByteCodePtr);
                    
    if (tmpByteCodePtr == spu->byteCodeArrayReadPtr)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::READING_FROM_BYTE_CODE_ERR);
                      return SpuErrors::READING_FROM_BYTE_CODE_ERR;
    }
    
    spu->byteCodeArrayReadPtr = tmpByteCodePtr;

    RETURN_STACK_PUSH_ERR(spu, valueToPush);
}

static SpuErrors CommandIn(SpuType* spu)
{
    assert(spu);
    assert(spu->byteCodeFile);

    SPU_CHECK(spu);

    ElemType valueToPush = 0;
    int scanfResult = scanf(ElemTypeFormat, &valueToPush);

    if (scanfResult != 1)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::READING_FROM_BYTE_CODE_ERR);
                      return SpuErrors::READING_FROM_BYTE_CODE_ERR;
    }
    
    RETURN_STACK_PUSH_ERR(spu, valueToPush);
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

static SpuErrors CommandDiv(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    SpuErrors errors = GetTwoLastValuesFromStack(&spu->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(errors);
    
    VALUES_CHECK(firstValue, secondValue);

    const ElemType Zero = 0; //хз как-то некрасиво выглядит
    if (!Equal(&secondValue, &Zero))
    {        
        RETURN_STACK_PUSH_ERR(spu, firstValue / secondValue);
    }


    StackPush(&spu->stack, INFINITY);

    SPU_ERRORS_LOG_ERROR(SpuErrors::TRYING_TO_DIVIDE_ON_ZERO);
                  return SpuErrors::TRYING_TO_DIVIDE_ON_ZERO;
}

static SpuErrors CommandMul(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    SpuErrors errors = GetTwoLastValuesFromStack(&spu->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(errors);

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(spu, firstValue * secondValue);
}

static SpuErrors CommandSub(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    SpuErrors errors = GetTwoLastValuesFromStack(&spu->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(errors);

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(spu, firstValue - secondValue);
}

static SpuErrors CommandAdd(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    ElemType secondValue = POISON;
    ElemType firstValue  = POISON;

    SpuErrors errors = GetTwoLastValuesFromStack(&spu->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(errors);

    VALUES_CHECK(firstValue, secondValue);

    RETURN_STACK_PUSH_ERR(spu, firstValue + secondValue);
}

#undef VALUES_CHECK

static SpuErrors CommandOut(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    ElemType equationResult = POISON;

    StackErrorsType stackError = StackPop(&spu->stack, &equationResult);

    if (stackError)
    {
        printf("Couldn't provide result. Getting result error occurred.\n");
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    printf("Equation result: " ElemTypeFormat "\n", equationResult);

    //PUSH_ON_STACK_BACK
    RETURN_STACK_PUSH_ERR(spu, equationResult);
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

static inline bool IsValidValues(const ElemType* firstVal, const ElemType* secondVal)
{
    return IsValidValue(firstVal) && IsValidValue(secondVal);
}

//----Spu structure functions--------

static SpuErrors SpuCtor(SpuType* spu, FILE* inStream)
{
    assert(spu);
    assert(inStream);

    spu->byteCodeFile         =          inStream;
    spu->byteCodeArray        = ReadText(inStream);

    spu->byteCodeArrayReadPtr = spu->byteCodeArray;

    StackErrorsType stackError = StackCtor(&spu->stack);

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

    fclose(spu->byteCodeFile);
    spu->byteCodeFile = nullptr;

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

    Log("Bytecode file adress                 : %p\n", spu->byteCodeFile);
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

    if (!spu->byteCodeFile)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::BYTE_CODE_FILE_IS_NULLPTR);
                      return SpuErrors::BYTE_CODE_FILE_IS_NULLPTR;
    }

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

    if (spu->byteCodeArrayReadPtr > 
        spu->byteCodeArray + GetFileSize(spu->byteCodeFile))
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
        case SpuErrors::BYTE_CODE_FILE_IS_NULLPTR:
            LOG_ERR("Byte code file is nullptr.\n");
            break;
        
        case SpuErrors::INVALID_SIGNATURE:
            LOG_ERR("Invalid signature.\n");
            break;
        case SpuErrors::INVALID_VERSION:
            LOG_ERR("Invalid version.\n");
            break;
        
        //TODO: add new errors
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

static inline SpuErrors CheckAddedInfo(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    char* byteCodeArrayReadPtr = spu->byteCodeArrayReadPtr;

    SignatureType fileSignature = (SignatureType) strtol(byteCodeArrayReadPtr, 
                                                        &byteCodeArrayReadPtr, 10);

    if (fileSignature != Signature)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_SIGNATURE);
                      return SpuErrors::INVALID_SIGNATURE;
    }

    VersionType fileVersion = (VersionType) strtol(byteCodeArrayReadPtr,
                                                  &byteCodeArrayReadPtr, 10);
    
    if (fileVersion != SpuVersion)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_VERSION);
                      return SpuErrors::INVALID_VERSION;
    }

    return SpuErrors::NO_ERR;
}
