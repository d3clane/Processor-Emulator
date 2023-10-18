#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Disassembly.h"
#include "../../InputOutput/InputOutput.h"

//--------Extra info checking--------

static inline int* SkipAddedInfo(int* byteCode);
static inline int* MoveToTheByteCodeStart(int* byteCode);
static inline CommandsErrors FileVerify(int* byteCode);

//--------Copy functions---------

static inline size_t CopyValue(const int* source, char* target, char** targetEndPtr);
static inline size_t CopyRegister(const int* source, char* target, char** targetEndPtr);
static inline size_t CopyArguments(Commands command, const int* source, 
                                   char* target, char** targetEndPtr);
static inline char* SprintfRegisterName(char* targPtr, const size_t registerId);

//-----Disassembly version--------------

static const uint32_t DisassemblyVersion = 1;

//------Auto generating define----------

#define DEF_CMD(name, num, haveArgs, ...)                                               \
    case Commands::name ##_ID:                                                          \
        asmCodePtr += sprintf(asmCodePtr, "%s ", #name);                                \
        break;

CommandsErrors Disassembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    size_t byteCodeSize = 0;
    int* byteCode    = ReadByteCode(inStream, &byteCodeSize);
    int* byteCodePtr = byteCode;

    CommandsErrors addedInfoErrors = FileVerify(byteCode);

    if (addedInfoErrors != CommandsErrors::NO_ERR)
    {
        COMMANDS_ERRORS_LOG_ERROR(addedInfoErrors);
                           return addedInfoErrors;
    }

    byteCodePtr = SkipAddedInfo(byteCodePtr);

    static const size_t maxCommandLength = 15; //including register length
    //TODO: change on reading from the beginning of the file for example (all size of the unassemblered)
    char* asmCode    = (char*) calloc(byteCodeSize * maxCommandLength, sizeof(char));
    char* asmCodePtr = asmCode;

    while (true)
    {
        int command    = *byteCodePtr;
    
        byteCodePtr++; 

        assert(asmCodePtr < asmCode + byteCodeSize * maxCommandLength);
        
        switch((Commands) command)
        {

        #include "../../Common/Commands.h"

        default:
            COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_ID);

            free(byteCode);
            free(asmCode);

            byteCode    = nullptr;
            byteCodePtr = nullptr;

            asmCode    = nullptr;
            asmCodePtr = nullptr;

            return CommandsErrors::INVALID_COMMAND_ID;
        }

        byteCodePtr += CopyArguments((Commands) command, byteCodePtr, asmCodePtr, &asmCodePtr);
        *asmCodePtr++ = '\n';

        if (command == (int) Commands::HLT_ID)
            break;
    }

    assert(asmCodePtr >= asmCode);
    PrintText(asmCode, (size_t)(asmCodePtr - asmCode), outStream);

    free(byteCode);
    byteCode    = nullptr;
    byteCodePtr = nullptr;

    free(asmCode);
    asmCode    = nullptr;
    asmCodePtr = nullptr;

    return CommandsErrors::NO_ERR;
}

#undef DEF_CMD

static inline size_t CopyValue(const int* source, char* target, char** targetEndPtr)
{
    assert(source);
    assert(target);
    assert(targetEndPtr);
    assert(*targetEndPtr);

    char* targPtr = target;

    targPtr += sprintf(targPtr, "%d", *source);

    *targetEndPtr = targPtr;

    return 1;
}

static inline size_t CopyRegister(const int* source, char* target, char** targetEndPtr)
{
    assert(source);
    assert(target);
    assert(targetEndPtr);
    assert(*targetEndPtr);

    char* targPtr = target;

    int registerId  = *source;

    if (0 <= registerId && registerId < NumberOfRegisters)
        targPtr = SprintfRegisterName(targPtr, (size_t)registerId);

    *targetEndPtr = targPtr;

    return 1;
}

static inline size_t CopyArguments(Commands command, const int* source, 
                                   char* target, char** targetEndPtr)
{
    switch (command)
    {
    case Commands::PUSH_ID:
        return CopyValue(source, target, targetEndPtr);
        break;

    case Commands::POP_ID:
    case Commands::PUSH_REGISTER_ID:
        return CopyRegister(source, target, targetEndPtr);
        break;

    default:
        break;
    }

    return 0;
}

static inline char* SprintfRegisterName(char* targPtr, const size_t registerId)
{
    assert(targPtr);
    assert(0 <= registerId && registerId < NumberOfRegisters);

    return targPtr + sprintf(targPtr, "r%cx",(char)('a' + registerId));
}

static inline int* SkipAddedInfo(int* byteCode)
{
    assert(byteCode);

    return byteCode + AddedInfoSizeByteCode;
}

static inline int* MoveToTheByteCodeStart(int* byteCode)
{
    assert(byteCode);

    return byteCode - AddedInfoSizeByteCode;
}

static inline CommandsErrors FileVerify(int* byteCode)
{
    assert(byteCode);

    SignatureType fileSignature = byteCode[0];
    
    if (fileSignature != Signature)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_SIGNATURE);
                           return CommandsErrors::INVALID_SIGNATURE; 
    }
    
    VersionType fileVersion = byteCode[1];

    if (fileVersion != DisassemblyVersion)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_VERSIONS);
                           return CommandsErrors::INVALID_VERSIONS;
    }

    return CommandsErrors::NO_ERR;
}

static inline void FreeByteCode(int* byteCode)
{
    assert(byteCode);

    byteCode = MoveToTheByteCodeStart(byteCode);
    free(byteCode);
}
