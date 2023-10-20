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
static inline void LabelsArrayCtor(LabelType* arr, const size_t size);
static inline void LabelTypeDtor(LabelType* label);
static inline void LabelsArrayDtor(LabelType* arr, const size_t size);
static inline LabelType* SetLabel(LabelType* arr, const size_t pos,  const char* labelName, 
                                                                     const int   labelAdress);
static inline LabelType* GetLabel(LabelType* arr, const size_t size, const char* labelName);
static inline int GetLabelAdress(const int *byteCodePtr, const int* byteCodeBegin);
static inline bool IsLabel(const char* labelName);

//HAVE TO BE LABLE ON INPUT
static inline size_t GetLabelLength(const char* labelName);

static const VersionType AssemblyVersion = 1;

static inline int GetRegisterId(const char* reg);
static inline int CopyIntArgument(const char* source, int* target, int** targetEndPtr);
static inline int CopyRegisterArgument(const char* source, int* target, int** targetEndPtr);
static inline int* AddSpecificationInfo(int* byteCode, const size_t asmFileSize, 
                                                       const size_t addedInfoSizeByteCode);

//-----------Printing commands---------------

static inline void PrintByteCode(int* byteCode, const size_t length, FILE* outStream);

//------------Consts------------------

#define DEF_CMD(name, num, instructionPrintingCode, ...)                                    \
    if (strcasecmp(command, #name) == 0)                                                    \
    {                                                                                       \
        instructionPrintingCode;                                                            \
    }                                                                                       \
    else 

CommandsErrors Assembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    TextType asmCode = {};
    TextTypeCtor(&asmCode, inStream);

    int* byteCode    = (int*) calloc(asmCode.textSz + AddedInfoSizeByteCode, sizeof(*byteCode));
    int* byteCodePtr = byteCode;

    byteCodePtr = AddSpecificationInfo(byteCodePtr, asmCode.textSz, AddedInfoSizeByteCode);

    static const size_t maxCommandLength  = 32;
    static char command[maxCommandLength] = "";

    static const size_t maxNumberOfLabels      = 10;
    static LabelType labels[maxNumberOfLabels] = {{}};
    LabelsArrayCtor(labels, maxNumberOfLabels);

    const size_t numberOfCompilations = 2;
          size_t labelSetPosition     = 0;

    for (size_t compilationId = 0; compilationId < numberOfCompilations; ++compilationId)
    {
        for (size_t line = 0; line < asmCode.linesCnt; ++line)
        {
            sscanf(asmCode.lines[line].line, "%s", command);

            if (compilationId == 0 && IsLabel(command))
            {
                assert(labelSetPosition < maxNumberOfLabels);

                SetLabel(labels, labelSetPosition, command, GetLabelAdress(byteCodePtr, byteCode));

                ++labelSetPosition;
                continue;
            }

            //TODO: какой-то костыль, мб создать функцию firstCompilation(), secondCompilation(), а то как-то супер не оч
            if (IsLabel(command)) continue;

            #include "../Common/Commands.h"

            /* else */
            {
                printf("Invalid command: %s\n", command);
                TextTypeDestructor(&asmCode);

                free(byteCode);
                byteCode    = nullptr;
                byteCodePtr = nullptr;

                COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_STRING);
                                   return CommandsErrors::INVALID_COMMAND_STRING;
            }
        }

        //TODO: запихать в функцию какую-нибудь потому что вот это плохо выглядит(
        if (compilationId != numberOfCompilations - 1)
            byteCodePtr = byteCode + AddedInfoSizeByteCode;
    }

    assert(byteCodePtr - byteCode > 0);

    PrintByteCode(byteCode, (size_t)(byteCodePtr - byteCode), outStream);

    TextTypeDestructor(&asmCode);

    free(byteCode);
    byteCode    = nullptr;
    byteCodePtr = nullptr;

    return CommandsErrors::NO_ERR;
}

#undef DEF_CMD

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

static inline int GetLabelAdress(const int *byteCodePtr, const int* byteCodeBegin)
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

    return (size_t) (strchr(labelName, ':') - labelName - 1);
}
