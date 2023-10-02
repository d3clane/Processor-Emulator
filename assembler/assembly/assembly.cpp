#include <assert.h>
#include <string.h>

#include "Assembly.h"
#include "../Commands.h"

static inline void CopyLine(FILE* inStream, FILE* outStream);

void Assembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    static const size_t maxCommandLength = 5;
    static char command[maxCommandLength] = "";

    while (true)
    {
        fscanf(inStream, "%s", command);
        
        if (strcmp(command, Push) == 0)
            fprintf(outStream, "%d", (int) Commands::PUSH);
        else if (strcmp(command, Div) == 0)
            fprintf(outStream, "%d", (int) Commands::DIV);
        else if (strcmp(command, Mul) == 0)
            fprintf(outStream, "%d", (int) Commands::MUL);
        else if (strcmp(command, Sub) == 0)
            fprintf(outStream, "%d", (int) Commands::SUB);
        else if (strcmp(command, Add) == 0)
            fprintf(outStream, "%d", (int) Commands::ADD);
        else if (strcmp(command, Out) == 0)
            fprintf(outStream, "%d", (int) Commands::OUT);
        else if (strcmp(command, Hlt) == 0)
        {
            fprintf(outStream, "%d", (int) Commands::HLT);
            break;  
        }
        else
            return; 
            //TODO: мб какой-то обработчик того, что в файле лежит говно, какую-нибудь ошибку возвращать
    
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