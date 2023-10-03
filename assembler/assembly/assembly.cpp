#include <assert.h>
#include <string.h>

#include "Assembly.h"
#include "../../Log.h"

static inline void CopyLine(FILE* inStream, FILE* outStream);

CommandsErrors Assembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    static const size_t maxCommandLength = 5;
    static char command[maxCommandLength] = "";

    while (true)
    {
        fscanf(inStream, "%s", command);
        
        //TODO: hashing
        if (strcmp(command, PUSH) == 0)
            fprintf(outStream, "%d", (int) Commands::PUSH_ID);
        else if (strcmp(command, IN) == 0)
            fprintf(outStream, "%d", (int) Commands::IN_ID);
        else if (strcmp(command, DIV) == 0)
            fprintf(outStream, "%d", (int) Commands::DIV_ID);
        else if (strcmp(command, MUL) == 0)
            fprintf(outStream, "%d", (int) Commands::MUL_ID);
        else if (strcmp(command, SUB) == 0)
            fprintf(outStream, "%d", (int) Commands::SUB_ID);
        else if (strcmp(command, ADD) == 0)
            fprintf(outStream, "%d", (int) Commands::ADD_ID);
        else if (strcmp(command, OUT) == 0)
            fprintf(outStream, "%d", (int) Commands::OUT_ID);
        else if (strcmp(command, HLT) == 0)
        {
            fprintf(outStream, "%d", (int) Commands::HLT_ID);
            break;  
        }
        else
        {
            //TODO: надо чтобы он выводил строчку, откуда call произошел (просто макрос нацепить)
            CommandsErrorsLogError(CommandsErrors::INVALID_COMMAND_STRING);
                            return CommandsErrors::INVALID_COMMAND_STRING;
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