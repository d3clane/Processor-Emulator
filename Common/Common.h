#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

enum class CommandsErrors
{
    NO_ERR,

    INVALID_COMMAND_SYNTAX,
    INVALID_COMMAND_STRING,
    INVALID_COMMAND_ID,

    INVALID_SIGNATURE,
    INVALID_VERSIONS,
    INVALID_ADDED_INFO,
};

typedef int SignatureType;
typedef int VersionType;

#pragma GCC diagnostic ignored "-Wmultichar"
static const SignatureType Signature = 'COCK';
#pragma GCC diagnostic warning "-Wmultichar"

#define DEF_CMD(name, ...) static const char* const name = #name;

//Creating constants PUSH = "push", POP = "pop", ...
#include "Commands.h"

#undef DEF_CMD

static const size_t AddedInfoSizeByteCode = 3;

enum AddedInfoPositions
{
    DisasmFileSizeInfoPosition = 0,
    SignatureInfoPosition      = 1,
    VersionInfoPosition        = 2,

};

static const size_t NumberOfRegisters    = 4;      
static const size_t RegisterStringLength = 3;

#define DEF_CMD(name, num, ...) name##_ID = num,

enum class Commands
{
    //Creating enums PUSH_ID, POP_ID, ...
    #include "Commands.h"
};

#undef DEF_CMD

#define COMMANDS_ERRORS_LOG_ERROR(error) CommandsErrorsLogError(error,                          \
                                                                __FILE__, __func__, __LINE__)

void CommandsErrorsLogError(CommandsErrors error, const char* fileName, 
                                                                const char* funcName, 
                                                                const int line);

int* ReadByteCode(FILE* inStream, size_t* byteCodeArrSize);

#endif // COMMON_H
