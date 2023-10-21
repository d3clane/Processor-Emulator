#include <assert.h>

#include "../InputOutput/InputOutput.h"
#include "Errors.h"
#include "Log.h"
#include "Common.h"

void CommandsErrorsLogError(CommandsErrors error, const char* fileName,
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
    case CommandsErrors::INVALID_ADDED_INFO:
        LOG_ERR("Invalid added info.\n");
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

int* ReadByteCode(FILE* inStream, size_t* byteCodeArrSize)
{
    assert(inStream);

    size_t byteCodeFileSize = GetFileSize(inStream);

    *byteCodeArrSize = byteCodeFileSize / sizeof(int) + 1;
    int* byteCode = (int*) calloc(*byteCodeArrSize, sizeof(*byteCode));

    fread(byteCode, sizeof(*byteCode), *byteCodeArrSize, inStream);

    return byteCode;    
}
