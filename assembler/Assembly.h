#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

#include "../Common/Common.h"
#include "../InputOutput/InputOutput.h"

CommandsErrors BuildByteCodeArr(TextType* asmCode, int** byteCodeStorage, size_t* byteCodeSize);
CommandsErrors Assembl(FILE* inStream, FILE* outStream);

#endif