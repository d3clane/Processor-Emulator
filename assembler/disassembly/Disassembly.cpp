#include <assert.h>
#include <string.h>

#include "Disassembly.h"
#include "../../InputOutput.h"

static inline char* CopyLine(const char* source, char* target);

CommandsErrors Disassembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    int command = -1;
    TextType byteCode = {};
    TextTypeCtor(&byteCode, inStream);

    static const size_t maxCommandLength = 5;
    char* asmCode    = (char*) calloc(byteCode.textSz * maxCommandLength, sizeof(*asmCode));
    char* asmCodePtr = asmCode;

    for (size_t line = 0; line < byteCode.linesCnt; ++line)
    {
        bool quitCycle = false;
        size_t commandLength = 0;
        sscanf(byteCode.lines[line].line, "%d", &command);

        switch((Commands) command)
        {
            case Commands::PUSH_ID:
                asmCodePtr += sprintf(asmCode, "%s", PUSH);
                commandLength = strlen(PUSH); 
                //Не знаю вместо стрлен можно просто константу пихать,
                //мне кажется компилятор должен оптимизировать сам, раз уж PUSH константна
                break;
            case Commands::IN_ID:
                asmCodePtr += sprintf(asmCode, "%s", IN);
                commandLength = strlen(IN);
                fprintf(outStream, IN);
                break;
            case Commands::DIV_ID:
                asmCodePtr += sprintf(asmCode, "%s", DIV);
                commandLength = strlen(DIV);
                break;
            case Commands::ADD_ID:
                asmCodePtr += sprintf(asmCode, "%s", ADD);
                commandLength = strlen(ADD);
                break;
            case Commands::SUB_ID:
                asmCodePtr += sprintf(asmCode, "%s", SUB);
                commandLength = strlen(SUB);
                break;
            case Commands::MUL_ID:
                asmCodePtr += sprintf(asmCode, "%s", MUL);
                commandLength = strlen(MUL);
                break;
            case Commands::OUT_ID:
                asmCodePtr += sprintf(asmCode, "%s", OUT);
                commandLength = strlen(OUT);
                break;
            case Commands::HLT_ID:
                asmCodePtr += sprintf(asmCode, "%s", HLT);
                commandLength = strlen(HLT);
                quitCycle = true;
                break;
            default:
                COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_ID);

                TextTypeDestructor(&byteCode);

                free(asmCode);
                asmCode    = nullptr;
                asmCodePtr = nullptr;

                return CommandsErrors::INVALID_COMMAND_ID;
        }

        if (quitCycle)
            break;

        asmCodePtr = CopyLine(byteCode.lines[line].line + commandLength, asmCodePtr);
    }

    PrintText(asmCode, strlen(asmCode), outStream);

    TextTypeDestructor(&byteCode);

    free(asmCode);
    asmCode    = nullptr;
    asmCodePtr = nullptr;

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
