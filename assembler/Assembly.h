#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

#include "../Common/Common.h"
#include "../InputOutput/InputOutput.h"

CommandsErrors BuildByteCodeArr(TextType* asmCode);
CommandsErrors Assembl(FILE* inStream, FILE* outStream);

#endif