#include <assert.h>
#include <string.h>

#include "Assembly.h"
#include "../../InputOutput.h"

static inline char* CopyLine(const char* source, char* target);

CommandsErrors Assembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    TextType assemblerCode = {};
    TextTypeCtor(&assemblerCode, inStream);

    char* outText    = (char*) calloc(assemblerCode.textSz, sizeof(*outText));
    char* outTextPtr = outText;

    static const size_t maxCommandLength  = 5;
    static char command[maxCommandLength] = "";

    for (size_t line = 0; line < assemblerCode.linesCnt; ++line)
    {
        sscanf(assemblerCode.lines[line].line, "%s", command);

        if (strcmp(command, PUSH) == 0)
            outTextPtr += sprintf(outTextPtr, "%d", (int) Commands::PUSH_ID);
        else if (strcmp(command, IN) == 0)
            outTextPtr += sprintf(outTextPtr, "%d", (int) Commands::IN_ID);
        else if (strcmp(command, DIV) == 0)
            outTextPtr += sprintf(outTextPtr, "%d", (int) Commands::DIV_ID);
        else if (strcmp(command, MUL) == 0)
            outTextPtr += sprintf(outTextPtr, "%d", (int) Commands::MUL_ID);
        else if (strcmp(command, SUB) == 0)
            outTextPtr += sprintf(outTextPtr, "%d", (int) Commands::SUB_ID);
        else if (strcmp(command, ADD) == 0)
            outTextPtr += sprintf(outTextPtr, "%d", (int) Commands::ADD_ID);
        else if (strcmp(command, OUT) == 0)
            outTextPtr += sprintf(outTextPtr, "%d", (int) Commands::OUT_ID);
        else if (strcmp(command, HLT) == 0)
        {
            outTextPtr += sprintf(outTextPtr, "%d", (int) Commands::HLT_ID);
            break;  
        }
        else
        {
            TextTypeDestructor(&assemblerCode);
            free(outText);
            COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_STRING);
                               return CommandsErrors::INVALID_COMMAND_STRING;
        }

        outTextPtr = CopyLine(assemblerCode.lines[line].line + strlen(command), outTextPtr);
    }

    PrintText(outText, assemblerCode.textSz, outStream);

    TextTypeDestructor(&assemblerCode);
    free(outText);

    return CommandsErrors::NO_ERR;
}

static inline char* CopyLine(const char* source, char* target)
{
    assert(source);
    assert(target);

    const char* srcPtr  = source;
          char* targPtr = target;
    
    while (*srcPtr != '\n' && *srcPtr != '\0')
    {
        *targPtr = *srcPtr;

        ++targPtr;
        ++srcPtr;
    }

    *targPtr = *srcPtr;
    ++targPtr;

    return targPtr;
}