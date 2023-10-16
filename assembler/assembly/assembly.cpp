#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Assembly.h"
#include "../../InputOutput.h"

static const VersionType AssemblyVersion = 1;

static inline int GetRegisterId(const char* reg);
static inline int* CopyArgument(const char* source, int* target);
static inline int* AddSpecificationInfo(int* byteCode);

//------------Writing to array commands--------------
static inline int* WritePushCommand(int* byteCode, LineType* asmCode);
static inline int* WritePopCommand (int* byteCode, LineType* asmCode);

//-----------Printing commands---------------
static inline void PrintByteCode(int* byteCode, const size_t length, FILE* outStream);

//------------Consts------------------

CommandsErrors Assembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    TextType asmCode = {};
    TextTypeCtor(&asmCode, inStream);

    int* byteCode    = (int*) calloc(asmCode.textSz + AddedInfoSizeByteCode, sizeof(*byteCode));
    int* byteCodePtr = byteCode;

    byteCodePtr = AddSpecificationInfo(byteCodePtr);

    static const size_t maxCommandLength  =  5;
    static char command[maxCommandLength] = "";

    for (size_t line = 0; line < asmCode.linesCnt; ++line)
    {
        sscanf(asmCode.lines[line].line, "%s", command);

        if (strcmp(command, PUSH) == 0)
            byteCodePtr = WritePushCommand(byteCodePtr, &asmCode.lines[line]);
        else if (strcmp(command, POP) == 0)
            byteCodePtr = WritePopCommand(byteCodePtr, &asmCode.lines[line]);
        else if (strcmp(command, IN) == 0)
            *byteCodePtr++ = (int)Commands::IN_ID;
        else if (strcmp(command, DIV) == 0)
            *byteCodePtr++ = (int)Commands::DIV_ID;
        else if (strcmp(command, MUL) == 0)
            *byteCodePtr++ = (int)Commands::MUL_ID;
        else if (strcmp(command, SUB) == 0)
            *byteCodePtr++ = (int)Commands::SUB_ID;
        else if (strcmp(command, ADD) == 0)
            *byteCodePtr++ = (int)Commands::ADD_ID;
    
        else if (strcmp(command, SIN) == 0)
            *byteCodePtr++ = (int)Commands::SIN_ID;
        else if (strcmp(command, COS) == 0)
            *byteCodePtr++ = (int)Commands::COS_ID;
        else if (strcmp(command, TAN) == 0)
            *byteCodePtr++ = (int)Commands::TAN_ID;
        else if (strcmp(command, COT) == 0)
            *byteCodePtr++ = (int)Commands::COT_ID;
        else if (strcmp(command, SQRT) == 0)
            *byteCodePtr++ = (int)Commands::SQRT_ID;
        else if (strcmp(command, POW) == 0)
            *byteCodePtr++ = (int)Commands::POW_ID;
    
        else if (strcmp(command, MEOW) == 0)
            *byteCodePtr++ = (int)Commands::MEOW_ID;
        else if (strcmp(command, BARK) == 0)
            *byteCodePtr++ = (int)Commands::BARK_ID;
        else if (strcmp(command, SLEEP) == 0)
            *byteCodePtr++ = (int)Commands::SLEEP_ID;
        else if (strcmp(command, BOTAY) == 0)
            *byteCodePtr++ = (int)Commands::BOTAY_ID;

        else if (strcmp(command, OUT) == 0)
            *byteCodePtr++ = (int)Commands::OUT_ID;

        else if (strcmp(command, HLT) == 0)
        {
            *byteCodePtr++ = (int)Commands::HLT_ID;
            break;  
        }
        else
        {
            TextTypeDestructor(&asmCode);

            free(byteCode);
            byteCode    = nullptr;
            byteCodePtr = nullptr;

            COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_STRING);
                               return CommandsErrors::INVALID_COMMAND_STRING;
        }

        byteCodePtr = CopyArgument(asmCode.lines[line].line + strlen(command), byteCodePtr);
    }

    assert(byteCodePtr - byteCode > 0);
    PrintByteCode(byteCode, (size_t)(byteCodePtr - byteCode), outStream);

    TextTypeDestructor(&asmCode);

    free(byteCode);
    byteCode    = nullptr;
    byteCodePtr = nullptr;

    return CommandsErrors::NO_ERR;
}

static inline int* WritePushCommand(int* byteCode, LineType* asmCode)
{
    assert(byteCode);
    assert(asmCode);

    static char registerName[RegisterStringLength + 1] = "";

    int scanfResult = sscanf(asmCode->line + strlen(PUSH), "%s", registerName);
    int registerId  = GetRegisterId(registerName);

    if (scanfResult == 0 || registerId == -1)
    {
        *byteCode++ = (int)Commands::PUSH_ID;
        return byteCode;
    }
    
    *byteCode++ = (int)Commands::PUSH_REGISTER_ID;
    *byteCode++ = registerId;

    return byteCode;
}

static inline int* WritePopCommand (int* byteCode, LineType* asmCode)
{
    assert(byteCode);
    assert(asmCode);

    static char registerName[RegisterStringLength + 1] = "";

    int scanfResult = sscanf(asmCode->line + strlen(POP), "%s", registerName);
    int registerId  = GetRegisterId(registerName);

    *byteCode++ = (int)Commands::POP_ID;

    if (scanfResult == 0 || registerId == -1)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_SYNTAX);
        return byteCode;
    }

    *byteCode++ = registerId;

    return byteCode;
}

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

static inline int* AddSpecificationInfo(int* byteCode)
{
    assert(byteCode);

    *byteCode++ = Signature;
    *byteCode++ = AssemblyVersion;

    return byteCode;
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
