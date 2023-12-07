#ifndef SPU_H
#define SPU_H

#include <stdio.h>

#include "Common/Common.h"
#include "Stack/Stack.h"

enum class SpuErrors
{
    NO_ERR,

    STACK_ERR,

    VALUES_COULD_NOT_BE_USED_FOR_ARITHMETIC,
    TRYING_TO_DIVIDE_ON_ZERO,

    READING_FROM_BYTE_CODE_ERR,

    INVALID_COMMAND,

    BYTE_CODE_ARR_IS_NULLPTR,
    BYTE_CODE_ARR_PTR_OUT_OF_RANGE,

    INVALID_VERSION,
    INVALID_SIGNATURE,

    INVALID_REGISTER,
};

struct SpuType
{
    StackType stack;
    StackType stackRet;

    int* byteCode;
    size_t ip;

    int registers[NumberOfRegisters];
    int ram[RamSize];
};

#define SPU_ERRORS_LOG_ERROR(ERROR) SpuErrorsLogError((ERROR), __FILE__, \
                                                               __func__, \
                                                               __LINE__)

void SpuErrorsLogError(SpuErrors error, const char* fileName,
                                        const char* funcName,
                                        const int line);

SpuErrors ExecuteByteCode(SpuType* spu);
SpuErrors ExecuteByteCode(FILE* inStream = stdin);

#endif // SPU_H