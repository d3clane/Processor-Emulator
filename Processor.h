#ifndef PROCESSOR_H
#define PORCESSOR_H

#include <stdio.h>

enum class ProcessorErrors
{
    NO_ERR,

    STACK_ERR,

    VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC,
    TRYING_TO_DIVIDE_ON_ZERO,

    READING_FROM_FILE_ERROR,

    INVALID_COMMAND,
};

ProcessorErrors Processing(FILE* inStream = stdin);

#endif