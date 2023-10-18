#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include "../../Common/Common.h"

CommandsErrors Assembly(FILE* inStream, FILE* outStream);

#endif