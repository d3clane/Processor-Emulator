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
static inline int* CallFunctionWithArgs(const char* commandName, 
                                    int* byteCode, LineType* asmCode);

static inline int* WritePushCommand(int* byteCode, LineType* asmCode);
static inline int* WritePopCommand (int* byteCode, LineType* asmCode);

//-----------Printing commands---------------
static inline void PrintByteCode(int* byteCode, const size_t length, FILE* outStream);

//------------Consts------------------

#define DEF_CMD(name, num, have_args, ...)                                                  \
    if (strcasecmp(command, #name) == 0)                                                    \
    {                                                                                       \
        if (have_args)                                                                      \
            byteCodePtr = CallFunctionWithArgs(command, byteCodePtr, &asmCode.lines[line]); \
        else                                                                                \
            *byteCodePtr++ = num;                                                           \
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
 
    byteCodePtr = AddSpecificationInfo(byteCodePtr);

    static const size_t maxCommandLength  =  5;
    static char command[maxCommandLength] = "";

    for (size_t line = 0; line < asmCode.linesCnt; ++line)
    {
        sscanf(asmCode.lines[line].line, "%s", command);

        #include "../Commands.h"

        /* else */
        {
            TextTypeDestructor(&asmCode);

            free(byteCode);
            byteCode    = nullptr;
            byteCodePtr = nullptr;

            COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_STRING);
                               return CommandsErrors::INVALID_COMMAND_STRING;
        }

        //TODO: перенести короче в копиаргумент перекопирование регистра (или наоборот убрать эту функцию подумать крч)
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

#undef DEF_CMD

static inline int* CallFunctionWithArgs(const char* commandName, 
                                        int* byteCode, LineType* asmCode)
{
    assert(commandName);
    assert(byteCode);
    assert(asmCode);

    if (strcasecmp(commandName, PUSH) == 0)
        return WritePushCommand(byteCode, asmCode);
    else if (strcasecmp(commandName, POP) == 0)
        return WritePopCommand(byteCode, asmCode);

    return nullptr;
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
