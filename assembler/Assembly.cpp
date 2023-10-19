#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Assembly.h"
#include "../InputOutput/InputOutput.h"

static const VersionType AssemblyVersion = 1;

static inline int GetRegisterId(const char* reg);
static inline int* CopyArgument(const char* source, int* target);
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

    static const size_t maxCommandLength  =  8;
    static char command[maxCommandLength] = "";

    for (size_t line = 0; line < asmCode.linesCnt; ++line)
    {
        sscanf(asmCode.lines[line].line, "%s", command);

        #include "../Common/Commands.h"

        /* else */
        {
            TextTypeDestructor(&asmCode);

            free(byteCode);
            byteCode    = nullptr;
            byteCodePtr = nullptr;

            COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_STRING);
                               return CommandsErrors::INVALID_COMMAND_STRING;
        }

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

static inline int* CopyArgument(const char* source, int* target)
{
    assert(source);
    assert(target);

    int scannedChars = 0;

    sscanf(source, "%d%n", target, &scannedChars);

    return target + scannedChars;
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

    byteCode[DisasmFileSizeInfoPosition] = asmFileSize;
    byteCode[SignatureInfoPosition]      = Signature;
    byteCode[VersionInfoPosition]        = AssemblyVersion;

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
