#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include "../Common.h"

CommandsErrors Disassembly(FILE* inStream, FILE* outStream);

#endif