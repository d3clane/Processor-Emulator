#include <assert.h>
#include <string.h>

#include "assembly.h"
#include "../Commands.h"

static inline void CopyLine(FILE* inStream, FILE* outStream);

void Assembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    static const char* push   = "push";
    static const char* div    =  "div";
    static const char* mul    =  "mul";
    static const char* sub    =  "sub";
    static const char* add    =  "add";
    static const char* out    =  "out";
    static const char* hlt    =  "hlt";

    static const size_t maxCommandLength = 5;
    static char command[maxCommandLength] = "";

    while (true)
    {
        fscanf(inStream, "%s", command);
        
        if (strcmp(command, push) == 0)
            fprintf(outStream, "%d", (int) Commands::PUSH);
        else if (strcmp(command, div) == 0)
            fprintf(outStream, "%d", (int) Commands::DIV);
        else if (strcmp(command, mul) == 0)
            fprintf(outStream, "%d", (int) Commands::MUL);
        else if (strcmp(command, sub) == 0)
            fprintf(outStream, "%d", (int) Commands::SUB);
        else if (strcmp(command, add) == 0)
            fprintf(outStream, "%d", (int) Commands::ADD);
        else if (strcmp(command, out) == 0)
            fprintf(outStream, "%d", (int) Commands::OUT);
        else if (strcmp(command, hlt) == 0)
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