#ifndef COMMANDS_H
#define COMMANDS_H

#include <assert.h>

#include "../InputOutput.h"
#include "../Errors.h"
#include "../Log.h"

enum class CommandsErrors
{
    NO_ERR,

    INVALID_COMMAND_SYNTAX,
    INVALID_COMMAND_STRING,
    INVALID_COMMAND_ID,

    INVALID_SIGNATURE,
    INVALID_VERSIONS,
};

typedef int SignatureType;
typedef int VersionType;

#pragma GCC diagnostic ignored "-Wmultichar"
static const SignatureType Signature = 'COCK';
#pragma GCC diagnostic warning "-Wmultichar"

// Signature in chars(10) + new line(1) + version in chars(1) + new line(1)
static const size_t AddedInfoSizeByteCode         = 2;
            
//TODO подумать че сделать с этими константами
static const char* const PUSH   = "push";
static const char* const IN     =   "in";

static const char* const POP    =  "pop";

static const char* const DIV    =  "div";
static const char* const MUL    =  "mul";
static const char* const SUB    =  "sub";
static const char* const ADD    =  "add";

static const char* const SIN    =  "sin";
static const char* const COS    =  "cos";
static const char* const TAN    =  "tan";
static const char* const COT    =  "cot";
static const char* const SQRT   = "sqrt";
static const char* const POW    =  "pow";

static const char* const MEOW   = "meow";
static const char* const BARK   = "bark";
static const char* const SLEEP  = "sleep";
static const char* const BOTAY  = "botay";

static const char* const OUT    =  "out";
static const char* const HLT    =  "hlt";

static const size_t NumberOfRegisters    = 4;      
static const size_t RegisterStringLength = 3;

#define DEF_CMD(name, num, ...) name##_ID = num,

enum class Commands
{
    #include "Commands.h"
};

#undef DEF_CMD

#define COMMANDS_ERRORS_LOG_ERROR(error) CommandsErrorsLogError(error,                          \
                                                                __FILE__, __func__, __LINE__)

static inline void CommandsErrorsLogError(CommandsErrors error, const char* fileName, 
                                                                const char* funcName, 
                                                                const int line)
{
    assert(fileName);
    assert(funcName);

    if (error == CommandsErrors::NO_ERR)
        return;
    
    LOG_BEGIN();

    Log("Error occurred in file %s, function %s, line %d\n", fileName, funcName, line);

    switch(error)
    {
    case CommandsErrors::NO_ERR:
        break;

    case CommandsErrors::INVALID_COMMAND_ID:
        LOG_ERR("Invalid command id in byte-code file.\n");
        break;
    case CommandsErrors::INVALID_COMMAND_STRING:
        LOG_ERR("Invalid command string in assembler file.\n");
        break;

    case CommandsErrors::INVALID_VERSIONS:
        LOG_ERR("Disassembler and assembler versions doesn't match.\n");
        break;
    case CommandsErrors::INVALID_SIGNATURE:
        LOG_ERR("Wrong signature in file. Can't disassembler.\n");
        break;
    
    case CommandsErrors::INVALID_COMMAND_SYNTAX:
        LOG_ERR("Invalid syntax.\n");
        break;

    default:
        LOG_ERR("Unknown error.\n");
        break;
    }    

    LOG_END();
}

//TODO: пора создать Commands.cpp и туда пихать реализации, а то странно жесть
static inline int* ReadByteCode(FILE* inStream, size_t* byteCodeArrSize)
{
    assert(inStream);
    size_t byteCodeFileSize = GetFileSize(inStream);

    *byteCodeArrSize = byteCodeFileSize / sizeof(int) + 1;
    int* byteCode = (int*) calloc(*byteCodeArrSize, sizeof(*byteCode));

    fread(byteCode, sizeof(*byteCode), *byteCodeArrSize, inStream);

    return byteCode;    
}

#endif
