#include <assert.h>
#include <string.h>

#include "Disassembly.h"

static inline void CopyLine(FILE* inStream, FILE* outStream);

CommandsErrors Disassembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    int command = -1;
    
    while (true)
    {
        fscanf(inStream, "%d", &command);

        switch((Commands) command)
        {
            case Commands::PUSH_ID:
                fprintf(outStream, PUSH);
                break;
            case Commands::IN_ID:
                fprintf(outStream, IN);
                break;
            case Commands::DIV_ID:
                fprintf(outStream, DIV);
                break;
            case Commands::ADD_ID:
                fprintf(outStream, ADD);
                break;
            case Commands::SUB_ID:
                fprintf(outStream, SUB);
                break;
            case Commands::MUL_ID:
                fprintf(outStream, MUL);
                break;
            case Commands::OUT_ID:
                fprintf(outStream, OUT);
                break;
            case Commands::HLT_ID:
                fprintf(outStream, HLT);
                return CommandsErrors::NO_ERR;
            default:
                COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_ID);
                                   return CommandsErrors::INVALID_COMMAND_ID;
        }

        CopyLine(inStream, outStream);
    }

    return CommandsErrors::NO_ERR;
}

static inline void CopyLine(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    int ch = 0;

    while (true)
    {
        ch = getc(inStream);
        putc(ch, outStream);

        if (ch == '\n' || ch == EOF) 
            break;
    }
}