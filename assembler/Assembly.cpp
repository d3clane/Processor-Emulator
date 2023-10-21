#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Assembly.h"
#include "../InputOutput/InputOutput.h"

struct LabelType
{
    int jmpAdress;
    const char* labelName;
};

static inline void LabelTypeCtor(LabelType* label);
static        void LabelsArrayCtor(LabelType* arr, const size_t size);
static inline void LabelTypeDtor(LabelType* label);
static        void LabelsArrayDtor(LabelType* arr, const size_t size);
static inline LabelType* SetLabel(LabelType* arr, const size_t pos,  const char* labelName, 
                                                                     const int   labelAdress);
static        LabelType* GetLabel(LabelType* arr, const size_t size, const char* labelName);
static inline int CalcJumpAdress(const int *byteCodePtr, const int* byteCodeBegin);
static inline bool IsLabel(const char* labelName);
//HAVE TO BE LABLE ON INPUT
static inline size_t GetLabelLength(const char* labelName);

static CommandsErrors ParseCommand(char* command, TextType* asmCode, const size_t line,
                                        int* byteCode, int* byteCodePtr, int** byteCodeEndPtr,
                                        LabelType* labels, const size_t maxNumberOfLabels);
static CommandsErrors BuildLabelsArray(TextType* asmCode, 
                                       int* byteCode, int* byteCodePtr,
                                       LabelType* labels, const size_t maxNumberOfLabels);
static CommandsErrors BuildJumps(TextType* asmCode, 
                                 int* byteCode, int* byteCodePtr,
                                 LabelType* labels, const size_t maxNumberOfLabels,
                                 int** byteCodeEndPtr);

static const VersionType AssemblyVersion = 1;

static inline int GetRegisterId(const char* reg);
static inline int CopyIntArgument(const char* source, int* target, int** targetEndPtr);
static inline int CopyRegisterArgument(const char* source, int* target, int** targetEndPtr);
static inline int* AddSpecificationInfo(int* byteCode, const size_t asmFileSize, 
                                                       const size_t addedInfoSizeByteCode);

//-----------Printing commands---------------

static inline void PrintByteCode(int* byteCode, const size_t length, FILE* outStream);

//------------Consts------------------

CommandsErrors Assembl(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    TextType asmCode = {};
    TextTypeCtor(&asmCode, inStream);

    int *byteCode        = nullptr;
    size_t byteCodeSize  = 0;
    BuildByteCodeArr(&asmCode, &byteCode, &byteCodeSize);

    PrintByteCode(byteCode, byteCodeSize, outStream);

    TextTypeDestructor(&asmCode);

    free(byteCode);
    byteCode = nullptr;

    return CommandsErrors::NO_ERR;
}

CommandsErrors BuildByteCodeArr(TextType* asmCode, int** byteCodeStorage, size_t* byteCodeSize)
{
    int* byteCode    = (int*) calloc(asmCode->textSz + AddedInfoSizeByteCode, sizeof(*byteCode));
    int* byteCodePtr = byteCode;

    byteCodePtr = AddSpecificationInfo(byteCodePtr, asmCode->textSz, AddedInfoSizeByteCode);

    static const size_t maxNumberOfLabels      = 10;
    static LabelType labels[maxNumberOfLabels] = {};
    LabelsArrayCtor(labels, maxNumberOfLabels);    

    CommandsErrors error = BuildLabelsArray(asmCode, byteCode, byteCodePtr, labels, maxNumberOfLabels);
    error = BuildJumps(asmCode, byteCode, byteCodePtr, labels, maxNumberOfLabels, &byteCodePtr);

    if (error != CommandsErrors::NO_ERR)
    {
        free(byteCode);
        COMMANDS_ERRORS_LOG_ERROR(error);
                           return error;
    }

    assert(byteCodePtr - byteCode > 0);
    *byteCodeStorage = byteCode;
    *byteCodeSize    = (size_t)(byteCodePtr - byteCode);

    return CommandsErrors::NO_ERR;
}

static inline int CopyIntArgument(const char* source, int* target, int** targetEndPtr)
{
    assert(source);
    assert(target);
    assert(targetEndPtr);

    int scanfResult = sscanf(source, "%d", target);
    
    if (scanfResult != 0)
        *targetEndPtr = target + 1;

    return scanfResult;
}

static inline int CopyRegisterArgument(const char* source, int* target, int** targetEndPtr)
{
    assert(source);
    assert(target);
    assert(targetEndPtr);

    static char registerName[RegisterStringLength + 1] = "";

    int scanfResult = sscanf(source, "%s", registerName);
    int registerId  = GetRegisterId(registerName);

    if (registerId == -1)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_SYNTAX);
        return 0;
    }

    *target++ = registerId;

    *targetEndPtr = target;

    return scanfResult;
}

static inline void PrintByteCode(int* byteCode, const size_t length, FILE* outStream)
{
    assert(byteCode);
    assert(outStream);

    size_t nWrite = fwrite(byteCode, sizeof(*byteCode), length, outStream);

    assert(nWrite == length);
}

static inline int* AddSpecificationInfo(int* byteCode, const size_t asmFileSize, 
                                                       const size_t addedInfoSizeByteCode)
{
    assert(byteCode);

    byteCode[DISASM_FILE_SIZE_INFO_POSITION] = asmFileSize;
    byteCode[SIGNATURE_INFO_POSITION]      = Signature;
    byteCode[VERSION_INFO_POSITION]        = AssemblyVersion;

    return byteCode + addedInfoSizeByteCode;
}

static inline int GetRegisterId(const char* reg)
{
    assert(reg);

    if (strlen(reg) != RegisterStringLength)
        return -1;
    
    if (reg[0] != 'r' || reg[2] != 'x' || reg[1] > 'd' || reg[1] < 'a')
        return -1;
    
    return reg[1] - 'a';
}

static inline void LabelTypeCtor(LabelType* label)
{
    assert(label);

    label->jmpAdress = -1;
    label->labelName = nullptr;
}

static inline void LabelsArrayCtor(LabelType* arr, const size_t size)
{
    assert(arr);
    
    for (size_t i = 0; i < size; ++i)
    {
        LabelTypeCtor(arr + i);
    }
}

static inline void LabelTypeDtor(LabelType* label)
{
    assert(label);

    label->jmpAdress = -1;

    if (label->labelName)
        free((void*)label->labelName);

    label->labelName = nullptr;
}

static inline void LabelsArrayDtor(LabelType* arr, const size_t size)
{
    assert(arr);

    for (size_t i = 0; i < size; ++i)
    {
        LabelTypeDtor(arr + i);
    }
}

//Creating dynamic array
static inline LabelType* SetLabel(LabelType* arr, const size_t pos,  const char* labelName, 
                                                                     const int   labelAdress)
{
    assert(arr);
    assert(labelName);


    if (!IsLabel(labelName))
    {
        return nullptr;
    }

    arr[pos].labelName = strdup(labelName);
    arr[pos].jmpAdress = labelAdress;

    return arr + pos;
}

static inline LabelType* GetLabel(LabelType* arr, const size_t size, const char* labelName)
{
    assert(arr);
    assert(labelName);

    for (size_t i = 0; i < size; ++i)
    {
        if (strcmp(labelName, arr[i].labelName) == 0)
        {
            return arr + i;
        }
    }

    return nullptr;
}

static inline int CalcJumpAdress(const int *byteCodePtr, const int* byteCodeBegin)
{
    assert(byteCodePtr);
    assert(byteCodeBegin);

    return (int)(byteCodePtr - byteCodeBegin);
}

static inline bool IsLabel(const char* labelName)
{
    assert(labelName);

    const char* labelEndPtr = labelName + strlen(labelName) - 1;

    while (labelEndPtr > labelName)
    {

        if (*labelEndPtr == ':')
            return true;
        
        if (!isspace(*labelEndPtr))
            return false;

        labelEndPtr--;
    }

    return false;
}

static inline size_t GetLabelLength(const char* labelName)
{
    assert(labelName);

    return (size_t)(strchr(labelName, ':') - labelName - 1);
}

static CommandsErrors BuildLabelsArray(TextType* asmCode, 
                                       int* byteCode, int* byteCodePtr,
                                       LabelType* labels, const size_t maxNumberOfLabels)
{
    assert(asmCode);
    assert(byteCode);
    assert(byteCodePtr);

    static const size_t maxCommandLength  = 32;
    static char command[maxCommandLength] = "";

    size_t labelSetPosition = 0;
    for (size_t line = 0; line < asmCode->linesCnt; ++line)
    {
        sscanf(asmCode->lines[line].line, "%s", command);

        if (IsLabel(command))
        {
            assert(labelSetPosition < maxNumberOfLabels);

            SetLabel(labels, labelSetPosition, command, CalcJumpAdress(byteCodePtr, byteCode));

            ++labelSetPosition;
            continue;
        }

        CommandsErrors error = ParseCommand(command, asmCode, line, 
                                            byteCode, byteCodePtr, &byteCodePtr,
                                            labels, maxNumberOfLabels);
        
        if (error != CommandsErrors::NO_ERR)
        {
            COMMANDS_ERRORS_LOG_ERROR(error);
                               return error;   
        }
    }

    return CommandsErrors::NO_ERR;
}

static CommandsErrors BuildJumps(TextType* asmCode, 
                                 int* byteCode, int* byteCodePtr,
                                 LabelType* labels, const size_t maxNumberOfLabels,
                                 int** byteCodeEndPtr)
{
    assert(asmCode);
    assert(byteCode);
    assert(byteCodePtr);
    assert(byteCodeEndPtr);

    static const size_t maxCommandLength  = 32;
    static char command[maxCommandLength] = "";

    for (size_t line = 0; line < asmCode->linesCnt; ++line)
    {
        sscanf(asmCode->lines[line].line, "%s", command);

        if (IsLabel(command))
            continue;

        CommandsErrors error = ParseCommand(command, asmCode, line, 
                                                byteCode, byteCodePtr, &byteCodePtr,
                                                labels, maxNumberOfLabels);

        if (error != CommandsErrors::NO_ERR)
        {
            COMMANDS_ERRORS_LOG_ERROR(error);       
                               return error;   
        }
    }

    *byteCodeEndPtr = byteCodePtr;
    return CommandsErrors::NO_ERR;     
}

#define DEF_CMD(name, num, instructionPrintingCode, ...)                                    \
    if (strcasecmp(command, #name) == 0)                                                    \
    {                                                                                       \
        instructionPrintingCode;                                                            \
    }                                                                                       \
    else 

static CommandsErrors ParseCommand(char* command, TextType* asmCode, const size_t line,
                                           int* byteCode, int* byteCodePtr, int** byteCodeEndPtr,
                                           LabelType* labels, const size_t maxNumberOfLabels)
{
    assert(command);
    assert(asmCode);
    assert(byteCode);
    assert(byteCodePtr);

    #include "../Common/Commands.h"

    /* else */
    {
        printf("Invalid command: %s\n", command);
        TextTypeDestructor(asmCode);

        free(byteCode);
        byteCode        = nullptr;
        byteCodePtr     = nullptr;
        *byteCodeEndPtr = nullptr;

        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_STRING);
                            return CommandsErrors::INVALID_COMMAND_STRING;
    }

    *byteCodeEndPtr = byteCodePtr;
    return CommandsErrors::NO_ERR;
}

#undef DEF_CMD
