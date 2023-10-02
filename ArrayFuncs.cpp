#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ArrayFuncs.h"

void FillArray(ElemType* firstBorder, ElemType* secondBorder, const ElemType value)
{
    assert(firstBorder);
    assert(secondBorder);

    if (firstBorder > secondBorder)
        Swap(&firstBorder, &secondBorder, sizeof(firstBorder));

    for (ElemType* arrIterator = firstBorder; arrIterator < secondBorder; ++arrIterator)
        *arrIterator = value;
}

void Swap(void* const element1, void* const element2, const size_t elemSize)
{
    assert(element1);
    assert(element2);
    assert(elemSize > 0);

    static const size_t bufferSz = 128;
    static char buffer[bufferSz] =  "";

    size_t elemLeftToSwap = elemSize;
    char* elem1 = (char*)element1;
    char* elem2 = (char*)element2;
    while (elemLeftToSwap > 0)
    {
        assert(buffer);
        assert(elem1);
        memcpy(buffer, elem1, bufferSz);
    
        assert(elem2);
        assert(elem1);
        memcpy(elem1, elem2, bufferSz);

        assert(elem2);
        assert(buffer);
        memcpy(elem2, buffer, bufferSz);

        elemLeftToSwap -= bufferSz;
        elem1 += bufferSz;
        elem2 += bufferSz;
    }
}
