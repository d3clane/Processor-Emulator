#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

#include "../Common/Common.h"

CommandsErrors BuildAsmCode(char* asmCode, int* byteCodePtr, 
                            char** asmCodeEndPtr, int** byteCodeEndPtr,
                            const size_t disasmFileSize);
    
CommandsErrors Disassembl(FILE* inStream, FILE* outStream);

#endif