#ifndef COMMANDS_H
#define COMMANDS_H

#include <assert.h>
#include "../Errors.h"
#include "../Log.h" //

enum class CommandsErrors
{
    NO_ERR,

    INVALID_COMMAND_STRING,
    INVALID_COMMAND_ID,
};

//TODO подумать че сделать с этими константами
static const char* const PUSH   = "push";
static const char* const IN     =   "in";

static const char* const DIV    =  "div";
static const char* const MUL    =  "mul";
static const char* const SUB    =  "sub";
static const char* const ADD    =  "add";

static const char* const OUT    =  "out";
static const char* const HLT    =  "hlt";

enum class Commands
{
    PUSH_ID,
    IN_ID,

    MUL_ID,
    ADD_ID, 
    DIV_ID,
    SUB_ID,
    OUT_ID,
    HLT_ID,
};

#define COMMANDS_ERRORS_LOG_ERROR(error) CommandsErrorsLogError(error,                \
                                                                __FILE__,             \
                                                                __func__,             \
                                                                __LINE__)

inline void CommandsErrorsLogError(CommandsErrors error, const char* fileName, 
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

        default:
            LOG_ERR("Unknown error.\n");
            break;
    }    

    LOG_END();
}

#endif
