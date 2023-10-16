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
    int* byteCodeArray;
    int* byteCodeArrayReadPtr;
    size_t byteCodeArraySize;

    int registers[NumberOfRegisters];
};

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
static inline SpuErrors CommandPow (int inValue, int* outValue);
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

typedef SpuErrors (CommandFuncType)(int value);

static SpuErrors CallBinaryCommand(SpuErrors (*Command)(int, int, int*), SpuType* spu);
static SpuErrors CallUnaryCommand (SpuErrors (*Command)(int,      int*), SpuType* spu);

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

/// @brief Spu check with dtor
#define SPU_CHECK(SPU)          \
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

    SpuErrors addedInfoError = FileVerify(&spu);

    if (addedInfoError != SpuErrors::NO_ERR)
    {
        SPU_ERRORS_LOG_ERROR(addedInfoError);

        SpuDtor(&spu);

        return addedInfoError;
    }

    SkipAddedInfo(&spu);
    
    SpuErrors SpuError = SpuErrors::NO_ERR;

    SPU_CHECK(spu);

    int command = -1;
    while (true)
    {
        bool quitCycle = false;
        
        command = *spu.byteCodeArrayReadPtr++;
        switch((Commands) command)
        {
        case Commands::PUSH_ID:
            SpuError = CommandPush(&spu);
            break;
        case Commands::PUSH_REGISTER_ID:
            SpuError = CommandPushRegister(&spu);
            break;
        case Commands::POP_ID:
            SpuError = CommandPop(&spu);
            break;
        case Commands::IN_ID:
            SpuError = CommandIn(&spu);
            break;
        case Commands::DIV_ID:
            SpuError = CallBinaryCommand(CommandDiv, &spu);
            break;
        case Commands::ADD_ID:
            SpuError = CallBinaryCommand(CommandAdd, &spu);
            break;
        case Commands::SUB_ID:
            SpuError = CallBinaryCommand(CommandSub, &spu);
            break;
        case Commands::MUL_ID:
            SpuError = CallBinaryCommand(CommandMul, &spu);
            break;

        case Commands::SIN_ID:
            CallUnaryCommand(CommandSin, &spu);
            break;
        case Commands::COS_ID:
            CallUnaryCommand(CommandCos, &spu);
            break;
        case Commands::TAN_ID:
            CallUnaryCommand(CommandTan, &spu);
            break;
        case Commands::COT_ID:
            CallUnaryCommand(CommandCot, &spu);
            break;  
        case Commands::POW_ID:
            CallUnaryCommand(CommandPow, &spu);
            break;
        case Commands::SQRT_ID:
            CallUnaryCommand(CommandSqrt, &spu);
            break;
        
        case Commands::MEOW_ID:
            SpuError = CommandMeow();
            break;
        case Commands::BARK_ID:
            SpuError = CommandBark();
            break;
        case Commands::SLEEP_ID:
            SpuError = CommandSleep();
            break;
        case Commands::BOTAY_ID:
            SpuError = CommandBotay();
            break;
        
        case Commands::OUT_ID:
            SpuError = CallUnaryCommand(CommandOut, &spu);
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

        SPU_CHECK(spu);

        if (quitCycle)
            break;
    }

    SpuDtor(&spu);

    IF_ERR_RETURN(SpuError);

    return SpuErrors::NO_ERR;
}

#undef SPU_CHECK

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

static SpuErrors CommandPush(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int valueToPush = *spu->byteCodeArrayReadPtr++;
    valueToPush *= CalculatingPrecision;

    STACK_PUSH(spu, valueToPush);

    SPU_CHECK(spu);
    
    return SpuErrors::NO_ERR;
}

static SpuErrors CommandPushRegister(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int registerId = *spu->byteCodeArrayReadPtr++;

    if (registerId < 0 || (size_t)registerId >= NumberOfRegisters)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_REGISTER);
                      return SpuErrors::INVALID_REGISTER;
    }

    assert(0 <= registerId && (size_t) registerId < NumberOfRegisters);

    STACK_PUSH(spu, spu->registers[registerId]);

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;
}

static SpuErrors CommandPop(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int registerId = *spu->byteCodeArrayReadPtr++;

    if (registerId < 0 || (size_t)registerId >= NumberOfRegisters)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_REGISTER);
                      return SpuErrors::INVALID_REGISTER;
    }

    assert(0 <= registerId && (size_t)registerId < NumberOfRegisters);

    StackErrorsType stackError = StackPop(&spu->stack, &spu->registers[registerId]);

    if (stackError)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;
}

static SpuErrors CommandIn(SpuType* spu)
{
    assert(spu);
    assert(spu->byteCodeFile);

    SPU_CHECK(spu);

    int valueToPush = POISON;
    
    printf("Enter value: ");
    int scanfResult = scanf(ElemTypeFormat, &valueToPush);
    
    if (scanfResult != 1)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::READING_FROM_BYTE_CODE_ERR);
                      return SpuErrors::READING_FROM_BYTE_CODE_ERR;
    }
    
    valueToPush *= CalculatingPrecision;

    STACK_PUSH(spu, valueToPush);

    SPU_CHECK(spu);
    
    return SpuErrors::NO_ERR;
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

static inline SpuErrors CommandDiv(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    if (inSecondValue == 0)
    {
        *outValue = POISON;
        SPU_ERRORS_LOG_ERROR(SpuErrors::TRYING_TO_DIVIDE_ON_ZERO);
                      return SpuErrors::TRYING_TO_DIVIDE_ON_ZERO;
    }

    *outValue = CalculatingPrecision * inFirstValue / inSecondValue;
    
    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandMul(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = inFirstValue * inSecondValue / CalculatingPrecision;
    
    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandSub(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = inFirstValue - inSecondValue;
    
    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandAdd(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = inFirstValue + inSecondValue;
    
    return SpuErrors::NO_ERR;
}

#define VALUE_CHECK(inValue)                                                      \
do                                                                                \
{                                                                                 \
    if (!IsValidValue(&inValue))                                                  \
    {                                                                             \
        SPU_ERRORS_LOG_ERROR(SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC); \
                      return SpuErrors::VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC;  \
    }                                                                             \
} while (0)

static inline SpuErrors CommandSin(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(sin(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandCos(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(cos(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandTan(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(tan(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandCot(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(CalculatingPrecision / tan(1.0 * inValue / CalculatingPrecision));

    return SpuErrors::NO_ERR;
}

//TODO:
static inline SpuErrors CommandPow(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandSqrt(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(sqrt(inValue) * sqrt(CalculatingPrecision));

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandMeow()
{
    system("say cello Meow meow meow meow");

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandBark()
{
    system("say Bad Bark bark bark bark");

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandSleep()
{
    system("say Bad It's time to sleep");

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandBotay()
{
    system("say время ботать");

    return SpuErrors::NO_ERR;
}

static inline SpuErrors CommandOut(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    printf("Equation result: " "%lf" "\n", 1.0 * inValue / CalculatingPrecision);

    return SpuErrors::NO_ERR;
}

#undef VALUES_CHECK
#undef VALUE_CHECK

static SpuErrors CallUnaryCommand (SpuErrors (*Command)(int, int*), SpuType* spu)
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

static SpuErrors CallBinaryCommand(SpuErrors (*Command)(int, int, int*), SpuType* spu)
{
    assert(Command);
    assert(spu);

    SPU_CHECK(spu);

    int firstValue  = POISON;
    int secondValue = POISON;

    SpuErrors error = GetTwoLastValuesFromStack(&spu->stack, &firstValue, &secondValue);

    IF_ERR_RETURN(error);

    if (!IsValidValue(&firstValue) || !IsValidValue(&secondValue))
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

    spu->byteCodeFile         =              inStream;
    spu->byteCodeArray        = ReadByteCode(inStream, &spu->byteCodeArraySize);

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


    SignatureType fileSignature = (SignatureType) *byteCodeArrayReadPtr++;

    if (fileSignature != Signature)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_SIGNATURE);
                      return SpuErrors::INVALID_SIGNATURE;
    }

    VersionType fileVersion = (VersionType) *byteCodeArrayReadPtr++;
    
    if (fileVersion != SpuVersion)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_VERSION);
                      return SpuErrors::INVALID_VERSION;
    }

    return SpuErrors::NO_ERR;
}
