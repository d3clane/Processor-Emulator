#ifndef ARRAY_FUNCS_H
#define ARRAY_FUNCS_H
#include "Types.h"

/// @file
/// @brief Functions for working with array

/// @brief Fills array from one border to another with some value. ElemType is declared in Types.h
///
/// @details If firstBorder > secondBorder function fills [secondBorder; firstBorder)
/// @details fills without right border.
/// @param [in]firstBorder border to fill from
/// @param [in]secondBorder border to fill to
/// @param [in]value value to fill with
void FillArray(ElemType* firstBorder, 
               ElemType* secondBorder, 
               const ElemType value);


/// @brief Swaps two elements
/// @param [out]element1 
/// @param [out]element2 
/// @param [in]elemSize size of one element for swapping
void Swap(void* const element1, void* const element2, const size_t elemSize);

#endif // ARRAY_FUNCS_H
