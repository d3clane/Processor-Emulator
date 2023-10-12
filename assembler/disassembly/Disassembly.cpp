#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Disassembly.h"
#include "../../InputOutput.h"

static inline int* SkipAddedInfo(int* byteCode);
static inline int* MoveToTheByteCodeStart(int* byteCode);
static inline CommandsErrors FileVerify(int* byteCode);

static inline size_t CopyArgument(const int* source, char* target, char** targetEndPtr);
static inline size_t CopyRegister(const int* source, char* target, char** targetEndPtr);
static inline char* SprintfRegisterName(char* targPtr, const size_t registerId);

static const uint32_t DisassemblyVersion = 1;

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

    static const size_t maxCommandLength = 8; //including register length
    //TODO: change on reading from the beginning of the file for example (all size of the unassemblered)
    char* asmCode    = (char*) calloc(byteCodeSize * maxCommandLength, sizeof(char));
    char* asmCodePtr = asmCode;

    while (true)
    {
        int command    = *byteCodePtr;
        byteCodePtr++;
        bool quitCycle = false;

        assert(asmCodePtr < asmCode + byteCodeSize * maxCommandLength);
        
        switch((Commands) command)
        {
        case Commands::PUSH_ID:
            asmCodePtr  += sprintf(asmCodePtr, "%s ", PUSH);
            byteCodePtr += CopyArgument(byteCodePtr, asmCodePtr, &asmCodePtr);
            break;
        case Commands::PUSH_REGISTER_ID:
            asmCodePtr  += sprintf(asmCodePtr, "%s ", PUSH);
            byteCodePtr += CopyRegister(byteCodePtr, asmCodePtr, &asmCodePtr);
            break;
        case Commands::POP_ID:
            asmCodePtr  += sprintf(asmCodePtr, "%s ", POP);
            byteCodePtr += CopyRegister(byteCodePtr, asmCodePtr, &asmCodePtr);
            break;
        
        case Commands::IN_ID:
            asmCodePtr += sprintf(asmCodePtr, "%s", IN);
            break;
        case Commands::DIV_ID:
            asmCodePtr += sprintf(asmCodePtr, "%s", DIV);
            break;
        case Commands::ADD_ID:
            asmCodePtr += sprintf(asmCodePtr, "%s", ADD);
            break;
        case Commands::SUB_ID:
            asmCodePtr += sprintf(asmCodePtr, "%s", SUB);
            break;
        case Commands::MUL_ID:
            asmCodePtr += sprintf(asmCodePtr, "%s", MUL);
            break;
        case Commands::OUT_ID:
            asmCodePtr += sprintf(asmCodePtr, "%s", OUT);
            break;
        case Commands::HLT_ID:
            asmCodePtr += sprintf(asmCodePtr, "%s", HLT);
            quitCycle = true;
            break;

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
        
        *asmCodePtr++ = '\n';

        if (quitCycle)
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

static inline size_t CopyArgument(const int* source, char* target, char** targetEndPtr)
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
