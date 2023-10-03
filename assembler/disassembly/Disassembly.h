#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include "../Commands.h"

CommandsErrors Disassembly(FILE* inStream, FILE* outStream);

#endif