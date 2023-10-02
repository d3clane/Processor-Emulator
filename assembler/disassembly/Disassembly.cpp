#include <assert.h>
#include <string.h>

#include "Disassembly.h"
#include "../Commands.h"

static inline void CopyLine(FILE* inStream, FILE* outStream);

void Disassembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    int command = -1;
    
    while (true)
    {
        fscanf(inStream, "%d", &command);

        switch((Commands) command)
        {
            case Commands::PUSH:
                fprintf(outStream, Push);
                break;
            case Commands::DIV:
                fprintf(outStream, Div);
                break;
            case Commands::ADD:
                fprintf(outStream, Add);
                break;
            case Commands::SUB:
                fprintf(outStream, Sub);
                break;
            case Commands::MUL:
                fprintf(outStream, Mul);
                break;
            case Commands::OUT:
                fprintf(outStream, Out);
                break;
            case Commands::HLT:
                fprintf(outStream, Hlt);
                return;
            default:
                return; //TODO: error handler
            
        }

        CopyLine(inStream, outStream);
    }
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