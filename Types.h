#ifndef TYPES_H
#define TYPES_H

/// @file 
/// @brief Contains types for stack

#include <assert.h>
#include <math.h>
#include <stdio.h>

/// @brief ElemType for stack
typedef int ElemType;

/// @brief Format for printing ElemType
#undef  ElemTypeFormat
#define ElemTypeFormat "%d"

/// @brief Chosen POISON value for stack
static const ElemType POISON = 0xBEEF;

/// @brief Function for checking if two ElemType values are equal 
/// @param [in]a first value
/// @param [in]b second value
/// @return true if they are equal otherwise false
static inline bool Equal(const ElemType* const a, const ElemType* const b)
{
    assert(a);
    assert(b);

    return a == b;
}

static inline bool IsValidValue(const ElemType* value)
{
    return !Equal(value, &POISON);
}
/*static inline bool Equal(const ElemType* const a, const ElemType* const b)
{
    assert(a);
    assert(b);

    int floatClassA = fpclassify(*a);
    int floatClassB = fpclassify(*b);

    if (floatClassA != FP_NORMAL)
    {
        if (floatClassA == floatClassB)
            return true;
        return false;
    }
    
    static const ElemType EPS = 1e-7;

    return fabs(*a - *b) < EPS;
}

static inline bool IsValidValue(const ElemType* value)
{
    return isfinite(*value);
}*/

#endif // TYPES_H