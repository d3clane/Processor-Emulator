#ifndef STACK_H
#define STACK_H

/// @file
/// @brief Contains functions to work with stack

#include <assert.h>

#include "../Common/Errors.h"
#include "Types.h"
#include "HashFuncs.h"

#define STACK_CANARY_PROTECTION

#define STACK_HASH_PROTECTION

#ifdef STACK_CANARY_PROTECTION

    #define ON_CANARY(...) __VA_ARGS__
    typedef unsigned long long CanaryType;

#else
    
    #define ON_CANARY(...)

#endif

#ifdef STACK_HASH_PROTECTION

    #define ON_HASH(...) __VA_ARGS__

#else

    #define ON_HASH(...) 

#endif

///@brief stack dump that substitutes __FILE__, __func__, __LINE__
#define STACK_DUMP(STK) StackDump((STK), __FILE__, __func__, __LINE__)

ON_HASH
(
    ///@brief HashType for hasing function in stack
    typedef HashType HashFuncType(const void* hashingArr, const size_t length, const uint64_t seed);
)

/// @brief Contains all info about data to use it 
struct StackType
{
    ON_CANARY
    (
        CanaryType structCanaryLeft; ///< left canary for the struct
    )

    ElemType* data;       ///< data with values. Have to be a dynamic array.
    size_t size;          ///< pos to push/pop values (actual size of the data at this moment).
    
    ON_HASH
    (
        
        HashType dataHash;      ///< hash of all elements in data.

        HashFuncType* HashFunc; ///< hashing function

        HashType structHash;    ///< hash of all elements in struct.
    )

    size_t capacity;     ///< REAL size of the data at this moment (calloced more than need at this moment).

    ON_CANARY
    (
        CanaryType structCanaryRight; ///< right canary for the struct
    )
};

typedef uint64_t StackErrorsType; ///< type for containing errors as bits

/// @brief Errors that can occure while stack is working. 
enum class StackErrors
{
    STACK_NO_ERR,

    STACK_MEMORY_ALLOCATION_ERROR,
    STACK_EMPTY_ERR, 
    STACK_IS_NULLPTR,
    STACK_CAPACITY_OUT_OF_RANGE,
    STACK_SIZE_OUT_OF_RANGE,
    STACK_INVALID_CANARY, 
    STACK_INVALID_DATA_HASH,
    STACK_INVALID_STRUCT_HASH,
};

#ifdef STACK_HASH_PROTECTION
    /// @brief Constructor
    /// @param [out]stk stack to fill
    /// @param [in]capacity size to reserve for the stack
    /// @param [in]HashFunc hash function for calculating hash
    /// @return errors that occurred
    StackErrorsType StackCtor(StackType* const stk, const size_t capacity = 0, 
                        const HashFuncType HashFunc = MurmurHash);
#else
    /// @brief Constructor
    /// @param [out]stk stack to fill
    /// @param [in]capacity size to reserve for the stack
    /// @return errors that occurred
    StackErrorsType StackCtor(StackType* const stk, const size_t capacity = 0);
#endif

/// @brief Destructor
/// @param [out]stk stack to destruct
/// @return errors that occurred
StackErrorsType StackDtor(StackType* const stk);

/// @brief Pushing value to the stack
/// @param [out]stk stack to push in
/// @param [in]val  value to push
/// @return errors that occurred
StackErrorsType StackPush(StackType* stk, const ElemType val);

/// @brief Popping value to the stack
/// @param [out]stk stack to pop
/// @param [out]retVal popped value
/// @return errors that occurred
StackErrorsType StackPop(StackType* stk, ElemType* retVal = nullptr);

/// @brief Verifies if stack is used right
/// @param [in]stk stack to verify
/// @return StackErrors in stack
StackErrorsType StackVerify(StackType* stk);

/// @brief Prints stack to log-file 
/// @param [in]stk stack to print out
/// @param [in]fileName __FILE__
/// @param [in]funcName __func__
/// @param [in]lineNumber __LINE__
void StackDump(const StackType* stk, const char* const fileName, 
                                     const char* const funcName,
                                     const int lineNumber);

/// @brief Checks if stack is empty
/// @param [in]stk stack to check 
/// @return true if stack is empty otherwise false
static inline bool StackIsEmpty(const StackType* stk)
{
    assert(stk);
    assert(stk->data);
    
    return stk->size == 0;
}

/// @brief Adds error as a bit to the errros
/// @param [in]errors where to add
/// @param [in]error  what to add
/// @return errors with added error
static inline StackErrorsType AddError(const StackErrorsType errors, const StackErrors error)
{
    if (error == StackErrors::STACK_NO_ERR)
        return errors;
    
    return (errors | ((StackErrorsType)1 << (StackErrorsType)(error)));
}

/// @brief Prints stack error to log file
/// @param [in]error error to print
void StackPrintError(StackErrors error);

#endif // STACK_H