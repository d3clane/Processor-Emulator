#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "Disassembly.h"
#include "../InputOutput/InputOutput.h"

//--------Extra info processing--------

static inline int* SkipAddedInfo(int* byteCode, const size_t addedInfoSizeByteCode);
static inline int* MoveToTheByteCodeStart(int* byteCode, const size_t addedInfoSizeByteCode);
static inline int ReadDisasmFileSize(int* byteCode);
static inline CommandsErrors FileVerify(int* byteCode);

//--------Copy functions---------

static inline size_t CopyValue(const int* source, char* target, char** targetEndPtr);
static inline size_t CopyRegister(const int* source, char* target, char** targetEndPtr);
static inline size_t CopyArguments(CommandArguments argId, const int* source, 
                                   char* target, char** targetEndPtr);
static inline char* SprintfRegisterName(char* targPtr, const size_t registerId);

//-----Disassembly version--------------

static const uint32_t DisassemblyVersion = 1;

//------Auto generating define----------

#define DEF_CMD(name, num, asmCode, argId, ...)                                         \
    case Commands::name ##_ID:                                                          \
        asmCodePtr  += sprintf(asmCodePtr, "%s ", #name);                               \
        byteCodePtr += CopyArguments(argId, byteCodePtr, asmCodePtr, &asmCodePtr);      \
        break;

CommandsErrors Disassembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    size_t byteCodeSize = 0;
    int* byteCode       = ReadByteCode(inStream, &byteCodeSize);
    int* byteCodePtr    = byteCode;

    CommandsErrors addedInfoErrors = FileVerify(byteCode);

    if (addedInfoErrors != CommandsErrors::NO_ERR)
    {
        COMMANDS_ERRORS_LOG_ERROR(addedInfoErrors);
                           return addedInfoErrors;
    }

    static const size_t asmCodeSizeMultiplier = 2;
    const int disasmFileSize = ReadDisasmFileSize(byteCodePtr) * asmCodeSizeMultiplier;
    assert(disasmFileSize > 0);

    byteCodePtr = SkipAddedInfo(byteCodePtr, AddedInfoSizeByteCode);

    char* asmCode    = (char*) calloc(disasmFileSize, sizeof(char));
    char* asmCodePtr = asmCode;

    while (true)
    {
        int command    = *byteCodePtr;
    
        byteCodePtr++; 

        assert(asmCodePtr - asmCode < disasmFileSize);
        
        switch((Commands) command)
        {

        #include "../Common/Commands.h"

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

        if (command == (int) Commands::HLT_ID)
            break;
    }

    assert(asmCodePtr - asmCode < disasmFileSize);
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

static inline size_t CopyArguments(CommandArguments argId, const int* source, 
                                   char* target, char** targetEndPtr)
{
    switch (argId)
    {
        case CommandArguments::ONE_INT_VALUE_ID:
            return CopyValue(source, target, targetEndPtr);
        case CommandArguments::ONE_REGISTER_ID:
            return CopyRegister(source, target, targetEndPtr);

        case CommandArguments::NO_ARGS_ID:
        default:
            break;
    }

    return 0;
}

static inline char* SprintfRegisterName(char* targPtr, const size_t registerId)
{
    assert(targPtr);
    assert(0 <= registerId && registerId < NumberOfRegisters);

    return targPtr + sprintf(targPtr, "r%cx", (char)('a' + registerId));
}

static inline int* SkipAddedInfo(int* byteCode, const size_t addedInfoSizeByteCode)
{
    assert(byteCode);

    return byteCode + addedInfoSizeByteCode;
}

static inline int ReadDisasmFileSize(int* byteCode)
{
    assert(byteCode);

    int disasmFileSize = byteCode[DISASM_FILE_SIZE_INFO_POSITION];

    assert(disasmFileSize > 0);

    return disasmFileSize;
}

static inline int* MoveToTheByteCodeStart(int* byteCode, const size_t addedInfoSizeByteCode)
{
    assert(byteCode);

    return byteCode - addedInfoSizeByteCode;
}

static inline CommandsErrors FileVerify(int* byteCode)
{
    assert(byteCode);

    int disasmFileSize = byteCode[DISASM_FILE_SIZE_INFO_POSITION];

    if (disasmFileSize < 0)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_ADDED_INFO);
                    return CommandsErrors::INVALID_ADDED_INFO;  
    }

    SignatureType fileSignature = byteCode[SIGNATURE_INFO_POSITION];
    
    if (fileSignature != Signature)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_SIGNATURE);
                           return CommandsErrors::INVALID_SIGNATURE; 
    }
    
    VersionType fileVersion = byteCode[VERSION_INFO_POSITION];

    if (fileVersion != DisassemblyVersion)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_VERSIONS);
                           return CommandsErrors::INVALID_VERSIONS;
    }

    return CommandsErrors::NO_ERR;
}
