#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Assembly.h"
#include "../../InputOutput.h"

static const VersionType AssemblyVersion = 1;

static inline int GetRegisterId(const char* reg);
static inline char* CopyLine(const char* source, char* target);
static inline char* AddSpecificationInfo(char* byteCode);

//------------Printing commands--------------
static inline char* SprintfPush(char* byteCode, LineType* asmCode);
static inline char* SprintfPop (char* byteCode, LineType* asmCode);

//------------Consts------------------
static const size_t MaxRegisterNameLength = 5;

CommandsErrors Assembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    TextType asmCode = {};
    TextTypeCtor(&asmCode, inStream);

    char* byteCode    = (char*) calloc(asmCode.textSz + AddedInfoSizeByteCode, sizeof(*byteCode));
    char* byteCodePtr = byteCode;

    byteCodePtr = AddSpecificationInfo(byteCodePtr);

    static const size_t maxCommandLength  =  5;
    static char command[maxCommandLength] = "";

    for (size_t line = 0; line < asmCode.linesCnt; ++line)
    {
        sscanf(asmCode.lines[line].line, "%s", command);

        if (strcmp(command, PUSH) == 0)
            byteCodePtr = SprintfPush(byteCodePtr, &asmCode.lines[line]);
        else if (strcmp(command, POP) == 0)
            byteCodePtr = SprintfPop(byteCodePtr, &asmCode.lines[line]);
        else if (strcmp(command, IN) == 0)
            byteCodePtr += sprintf(byteCodePtr, "%d", (int) Commands::IN_ID);
        else if (strcmp(command, DIV) == 0)
            byteCodePtr += sprintf(byteCodePtr, "%d", (int) Commands::DIV_ID);
        else if (strcmp(command, MUL) == 0)
            byteCodePtr += sprintf(byteCodePtr, "%d", (int) Commands::MUL_ID);
        else if (strcmp(command, SUB) == 0)
            byteCodePtr += sprintf(byteCodePtr, "%d", (int) Commands::SUB_ID);
        else if (strcmp(command, ADD) == 0)
            byteCodePtr += sprintf(byteCodePtr, "%d", (int) Commands::ADD_ID);
        else if (strcmp(command, OUT) == 0)
            byteCodePtr += sprintf(byteCodePtr, "%d", (int) Commands::OUT_ID);
        else if (strcmp(command, HLT) == 0)
        {
            byteCodePtr += sprintf(byteCodePtr, "%d", (int) Commands::HLT_ID);
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

        byteCodePtr = CopyLine(asmCode.lines[line].line + strlen(command), byteCodePtr);
    }

    PrintText(byteCode, strlen(byteCode), outStream);

    TextTypeDestructor(&asmCode);

    free(byteCode);
    byteCode    = nullptr;
    byteCodePtr = nullptr;

    return CommandsErrors::NO_ERR;
}

static inline char* SprintfPush(char* byteCode, LineType* asmCode)
{
    assert(byteCode);
    assert(asmCode);

    static char registerName[MaxRegisterNameLength] = "";

    int scanfResult = sscanf(asmCode->line + strlen(PUSH), "%s", registerName);
    int registerId  = GetRegisterId(registerName);

    if (scanfResult == 0 || registerId == -1)
    {
        byteCode += sprintf(byteCode, "%d", (int) Commands::PUSH_ID);
        return byteCode;
    }
    
    byteCode += sprintf(byteCode, "%d %d", (int) Commands::PUSH_REGISTER_ID, registerId);

    return byteCode;
}

static inline char* SprintfPop(char* byteCode, LineType* asmCode)
{
    assert(byteCode);
    assert(asmCode);

    static char registerName[MaxRegisterNameLength] = "";

    int scanfResult = sscanf(asmCode->line + strlen(POP), "%s", registerName);
    int registerId  = GetRegisterId(registerName);

    if (scanfResult == 0 || registerId == -1)
    {
        byteCode += sprintf(byteCode, "%d", (int) Commands::POP_ID);
        return byteCode;
    }

    byteCode += sprintf(byteCode, "%d %d", (int) Commands::PUSH_REGISTER_ID, registerId);

    return byteCode;
}

static inline char* CopyLine(const char* source, char* target)
{
    assert(source);
    assert(target);

    const char* srcPtr  = source;
          char* targPtr = target;
    
    
    while (*srcPtr != '\n' && *srcPtr != '\0')
    {
        if (isalpha(*srcPtr))
        {
            ++srcPtr;
            continue;
        }

        *targPtr = *srcPtr;

        ++targPtr;
        ++srcPtr;
    }

    *targPtr = *srcPtr;
    ++targPtr;

    return targPtr;
}

static inline char* AddSpecificationInfo(char* byteCode)
{
    assert(byteCode);

    int addedInfoSizeof = sprintf(byteCode, "%u\n%u\n", Signature, AssemblyVersion);

    assert((size_t) addedInfoSizeof == AddedInfoSizeByteCode);

    return byteCode + addedInfoSizeof;
}

static inline int GetRegisterId(const char* reg)
{
    assert(reg);

    if (strlen(reg) != RegisterLength)
        return -1;
    
    if (reg[0] != 'r' || reg[2] != 'x' || reg[1] > 'd' || reg[1] < 'a')
        return -1;
    
    return reg[1] - 'a';
}
