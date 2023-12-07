#include <math.h>
#include <execinfo.h> 
#include <string.h>

#include "ArrayFuncs.h"
#include "Stack.h"
#include "Common/Log.h"

//----------static functions------------

static StackErrorsType StackRealloc(StackType* stk, bool increase);

static inline ElemType* MovePtr(ElemType* const data, const size_t moveSz, const int times);

static inline size_t StackGetSizeForCalloc(StackType* const stk);

static void StackDataFill(StackType* const stk);

static inline bool StackIsFull(StackType* stk);

static inline bool StackIsTooBig(StackType* stk);

//--------CANARY PROTECTION----------

#ifdef STACK_CANARY_PROTECTION

    #define CanaryTypeFormat "%#0llx"
    const CanaryType Canary = 0xDEADBABE;

    const size_t Aligning = 8;

    static inline void CanaryCtor(void* storage)
    {
        assert(storage);
        CanaryType* strg = (CanaryType*) (storage);

        *strg = Canary;
    }

    static inline ElemType* GetAfterFirstCanaryAdr(const StackType* const stk)
    {
        return MovePtr(stk->data, sizeof(CanaryType), 1);
    }

    static inline ElemType* GetFirstCanaryAdr(const StackType* const stk)
    {
        return MovePtr(stk->data, sizeof(CanaryType), -1);
    }

    static inline ElemType* GetSecondCanaryAdr(const StackType* const stk)
    {
        return (ElemType*)((char*)(stk->data + stk->capacity) +
                           Aligning - (stk->capacity * sizeof(ElemType)) % Aligning);
    }

#endif

//-----------HASH PROTECTION---------------

#ifdef STACK_HASH_PROTECTION

    static inline HashType CalcDataHash(const StackType* stk)
    {
        return stk->HashFunc(stk->data, stk->capacity * sizeof(ElemType), 0);        
    }

    static inline void UpdateDataHash(StackType* stk)
    {
        stk->dataHash = CalcDataHash(stk);      
    }

    static inline void UpdateStructHash(StackType* stk)
    {
        stk->structHash = 0;                                  
        stk->structHash = MurmurHash(stk, sizeof(*stk));        
    }
    
#endif

//--------------Consts-----------------

static const size_t STANDARD_CAPACITY = 64;

//---------------

#ifndef NDEBUG

    #define STACK_CHECK(stk)                    \
    do                                          \
    {                                           \
        StackErrorsType stackErr = StackVerify(stk); \
                                                \
        if (stackErr != 0)                      \
        {                                       \
            STACK_DUMP(stk);                    \
            return stackErr;                    \
        }                                       \
    } while (0)

    #define STACK_CHECK_NO_RETURN(stk)         \
    do                                         \
    {                                          \
        StackErrorsType stackErr = StackVerify(stk);\
                                               \
        if (stackErr != 0)                     \
        {                                      \
            STACK_DUMP(stk);                   \
        }                                      \
    } while (0)

#else

    #define STACK_CHECK(stk)           
    #define STACK_CHECK_NO_RETURN(stk) 

#endif

//---------------

#define IF_ERR_RETURN(ERR)              \
do                                      \
{                                       \
    if (ERR)                            \
        return ERR;                     \
} while (0)

//---------------

#ifdef STACK_HASH_PROTECTION
StackErrorsType StackCtor(StackType* const stk, const size_t capacity, 
                          const HashFuncType HashFunc)
{
    assert(stk);

    ON_HASH
    (
        stk->HashFunc = HashFunc;
    )

    //--------SET STRUCT CANARY-------
    ON_CANARY
    (
        stk->structCanaryLeft  = Canary;
        stk->structCanaryRight = Canary;
    )

    StackErrorsType errors = 0;
    stk->size = 0;

    if (capacity > 0) stk->capacity = capacity;
    else              stk->capacity = STANDARD_CAPACITY;

    //----Callocing Memory-------

    stk->data = (ElemType*) calloc(StackGetSizeForCalloc(stk), sizeof(*stk->data));

    if (stk->data == nullptr)
    {
                StackPrintError(StackErrors::STACK_MEMORY_ALLOCATION_ERROR);  
        return AddError(errors, StackErrors::STACK_MEMORY_ALLOCATION_ERROR);   
    }

    //-----------------------

    StackDataFill(stk);

    ON_CANARY
    (
        stk->data = GetAfterFirstCanaryAdr(stk);
    )

    ON_HASH
    (
        UpdateDataHash(stk);
        UpdateStructHash(stk);
    )

    STACK_CHECK(stk);

    return errors;
}
#else 
StackErrorsType StackCtor(StackType* const stk, const size_t capacity)
{
    assert(stk);

    //--------SET STRUCT CANARY-------
    ON_CANARY
    (
        stk->structCanaryLeft  = Canary;
        stk->structCanaryRight = Canary;
    )

    StackErrorsType errors = 0;
    stk->size = 0;

    if (capacity > 0) stk->capacity = capacity;
    else              stk->capacity = STANDARD_CAPACITY;

    //----Callocing Memory-------

    stk->data = (ElemType*) calloc(StackGetSizeForCalloc(stk), sizeof(*stk->data));

    if (stk->data == nullptr)
    {
                StackPrintError(StackErrors::STACK_MEMORY_ALLOCATION_ERROR);  
        return AddError(errors, StackErrors::STACK_MEMORY_ALLOCATION_ERROR);   
    }

    //-----------------------

    StackDataFill(stk);

    ON_CANARY
    (
        stk->data = GetAfterFirstCanaryAdr(stk);
    )

    STACK_CHECK(stk);

    return errors;    
}
#endif

StackErrorsType StackDtor(StackType* const stk)
{
    assert(stk);

    STACK_CHECK(stk);

    for (size_t i = 0; i < stk->size; ++i)
    {
        stk->data[i] = POISON;
    }

    ON_CANARY
    (
        stk->data = GetFirstCanaryAdr(stk);
    )

    free(stk->data);
    stk->data = nullptr;

    stk->size     = 0;
    stk->capacity = 0;

    ON_HASH
    (
        stk->dataHash = 0;
    )

    ON_CANARY
    (
        stk->structCanaryLeft  = 0;
        stk->structCanaryRight = 0;
    )

    return 0;
}

StackErrorsType StackPush(StackType* stk, const ElemType val)
{
    assert(stk);
    
    STACK_CHECK(stk);

    StackErrorsType stackReallocErr = 0;
    if (StackIsFull(stk)) stackReallocErr = StackRealloc(stk, true);

    IF_ERR_RETURN(stackReallocErr);

    stk->data[stk->size++] = val;

    ON_HASH
    (
        UpdateDataHash(stk);
        UpdateStructHash(stk);
    )

    STACK_CHECK(stk);

    return 0;
}

StackErrorsType StackPop(StackType* stk, ElemType* retVal)
{
    assert(stk);

    STACK_CHECK(stk);

    StackErrorsType errors = 0;
    
    if (StackIsEmpty(stk))
    { 
        errors = AddError(errors, StackErrors::STACK_EMPTY_ERR);
                  StackPrintError(StackErrors::STACK_EMPTY_ERR);
        return errors;
    }

    --stk->size;
    if (retVal) *retVal = stk->data[stk->size];
    stk->data[stk->size] = POISON;

    ON_HASH
    (
        UpdateDataHash(stk);
        UpdateStructHash(stk);
    )

    if (StackIsTooBig(stk))
    {
        StackErrorsType stackReallocErr = StackRealloc(stk, false);

        STACK_CHECK(stk);

        IF_ERR_RETURN(stackReallocErr);

        ON_HASH
        (
            UpdateDataHash(stk);
            UpdateStructHash(stk);
        )
    }

    STACK_CHECK(stk);

    return 0;
}

StackErrorsType StackVerify(StackType* stk)
{
    assert(stk);

    StackErrorsType errors = 0;

    if (stk->data == nullptr)
    {
       errors = AddError(errors, StackErrors::STACK_IS_NULLPTR);
                 StackPrintError(StackErrors::STACK_IS_NULLPTR);
    }

    if (stk->capacity <= 0)
    {  
        errors = AddError(errors, StackErrors::STACK_CAPACITY_OUT_OF_RANGE);
                  StackPrintError(StackErrors::STACK_CAPACITY_OUT_OF_RANGE);
    }

    if (stk->size > stk->capacity)
    {
        errors = AddError(errors, StackErrors::STACK_SIZE_OUT_OF_RANGE);
                  StackPrintError(StackErrors::STACK_SIZE_OUT_OF_RANGE);
    }

    //-----------Canary checking----------

    ON_CANARY
    (
        if (*(CanaryType*)(GetFirstCanaryAdr(stk)) != Canary)
        {
            errors = AddError(errors, StackErrors::STACK_INVALID_CANARY);
                      StackPrintError(StackErrors::STACK_INVALID_CANARY);
        }

        if (*(CanaryType*)(GetSecondCanaryAdr(stk)) != Canary)
        {
            errors = AddError(errors, StackErrors::STACK_INVALID_CANARY);
                      StackPrintError(StackErrors::STACK_INVALID_CANARY);
        }

        if (stk->structCanaryLeft != Canary)
        {
            errors = AddError(errors, StackErrors::STACK_INVALID_CANARY);
                      StackPrintError(StackErrors::STACK_INVALID_CANARY);
        }

        if (stk->structCanaryRight != Canary)
        {
            errors = AddError(errors, StackErrors::STACK_INVALID_CANARY);
                      StackPrintError(StackErrors::STACK_INVALID_CANARY);
        }

    )

    //------------Hash checking----------

    ON_HASH
    (
        if (CalcDataHash(stk) != stk->dataHash)
        {
            errors = AddError(errors, StackErrors::STACK_INVALID_DATA_HASH);
                      StackPrintError(StackErrors::STACK_INVALID_DATA_HASH);
        }

        StackErrorsType prevStructHash = stk->structHash;
        UpdateStructHash(stk);

        if (prevStructHash != stk->structHash)
        {
            errors = AddError(errors, StackErrors::STACK_INVALID_STRUCT_HASH);
                      StackPrintError(StackErrors::STACK_INVALID_STRUCT_HASH);

            stk->structHash = prevStructHash;
        }
    )

    if (errors)
    {
        STACK_DUMP(stk);
    }
    
    return errors;
}

void StackDump(const StackType* stk, const char* const fileName, 
                                     const char* const funcName,
                                     const int lineNumber)
{
    assert(stk);
    assert(fileName);
    assert(funcName);
    assert(lineNumber > 0);
    
    LOG_BEGIN();

    Log("StackDump was called in file %s, function %s, line %d\n", fileName, funcName, lineNumber);
    Log("Stk[%p]\n{\n", stk);
    Log("\tStk capacity: %zu, \n"
        "\tStk size    : %zu,\n",
        stk->capacity, stk->size);
    
    ON_CANARY
    (
        Log("\tLeft struct canary : " CanaryTypeFormat ",\n", 
            stk->structCanaryLeft);
        Log("\tRight struct canary: " CanaryTypeFormat ",\n", 
            stk->structCanaryRight);

        Log("\tLeft data canary : " CanaryTypeFormat ",\n", 
            *(CanaryType*)(GetFirstCanaryAdr(stk)));
        Log("\tRight data canary: " CanaryTypeFormat ",\n", 
            *(CanaryType*)(GetSecondCanaryAdr(stk)));
    )

    ON_HASH
    (
        Log("\tData hash  : %llu\n", stk->dataHash);
        Log("\tStruct hash: %llu\n", stk->structHash);
    )

    Log("\tdata data[%p]\n\t{\n", stk->data);

    if (stk->data != nullptr)
    {
        for (size_t i = 0; i < (stk->size < stk->capacity ? stk->size : stk->capacity); ++i)
        {
            Log("\t\t*[%zu] = " ElemTypeFormat, i, stk->data[i]);

            if (Equal(&stk->data[i], &POISON)) Log(" (POISON)");

            Log("\n");
        }

        Log("\t\tNot used values:\n");

        for(size_t i = stk->size; i < stk->capacity; ++i)
        {
            Log("\t\t*[%zu] = " ElemTypeFormat, i, stk->data[i]);
            
            if (Equal(&stk->data[i], &POISON)) Log(" (POISON)");

            Log("\n");
        }
    }

    Log("\t}\n}\n");

    LOG_END();
}

StackErrorsType StackRealloc(StackType* stk, bool increase)
{
    assert(stk);

    STACK_CHECK(stk);
    
    if (increase) stk->capacity <<= 1;
    else          stk->capacity >>= 1;

    if (!increase) 
        FillArray(stk->data + stk->size, stk->data + stk->capacity, POISON);

    //--------Moves data to the first canary-------
    ON_CANARY
    (
        stk->data = GetFirstCanaryAdr(stk);
    )

    ElemType* tmpStack = (ElemType*) realloc(stk->data, 
                                             StackGetSizeForCalloc(stk) * sizeof(*stk->data));

    if (tmpStack == nullptr)
    {
        StackPrintError(StackErrors::STACK_MEMORY_ALLOCATION_ERROR);

        assert(stk);
        if (increase) stk->capacity >>= 1;
        else          stk->capacity <<= 1;

        STACK_CHECK(stk);

        return 0;
    }

    stk->data = tmpStack;

    // -------Moving forward after reallocing-------
    ON_CANARY
    (
        stk->data = GetAfterFirstCanaryAdr(stk);
    )

    if (increase)
        FillArray(stk->data + stk->size, stk->data + stk->capacity, POISON);

    // -------Putting canary at the end-----------
    ON_CANARY
    (
        CanaryCtor(GetSecondCanaryAdr(stk));
    )

    ON_HASH
    (
        UpdateDataHash(stk);
        UpdateStructHash(stk);
    )
    
    STACK_CHECK(stk);

    return 0;
}

static inline bool StackIsFull(StackType* stk)
{
    assert(stk);

    STACK_CHECK_NO_RETURN(stk);

    return stk->size >= stk->capacity;
}

static inline bool StackIsTooBig(StackType* stk)
{
    assert(stk);

    STACK_CHECK_NO_RETURN(stk);

    return (stk->size * 4 <= stk->capacity) & (stk->capacity > STANDARD_CAPACITY);
}

static inline ElemType* MovePtr(ElemType* const data, const size_t moveSz, const int times)
{
    assert(data);
    assert(moveSz > 0);
    
    return (ElemType*)((char*)data + times * (long long)moveSz);
}

// NO stack check because doesn't fill hashes
static void StackDataFill(StackType* const stk)
{
    assert(stk);
    assert(stk->data);
    assert(stk->capacity > 0);

    ON_CANARY
    (
        CanaryCtor(stk->data);
        stk->data = GetAfterFirstCanaryAdr(stk);
    )

    FillArray(stk->data, stk->data + stk->capacity, POISON);

    ON_CANARY
    (
        CanaryCtor(GetSecondCanaryAdr(stk));
        stk->data = GetFirstCanaryAdr(stk);
    )

    // No stack check because doesn't fill hashes
}

// no STACK_CHECK because can be used for callocing memory (data could be nullptr at this moment)
static inline size_t StackGetSizeForCalloc(StackType* const stk)
{
    assert(stk);
    assert(stk->capacity > 0);

    ON_CANARY
    (
        return stk->capacity + 3 * sizeof(CanaryType) / sizeof(*stk->data);
    )

    return stk->capacity;
}

#undef STACK_CHECK
#undef IF_ERR_RETURN
#undef GetAfterFirstCanaryAdr
#undef GetFirstCanaryAdr
#undef GetSecondCanaryAdr

#define LOG_ERR(X) Log(HTML_RED_HEAD_BEGIN "\n" X "\n" HTML_HEAD_END "\n")
void StackPrintError(StackErrors error)
{
    LOG_BEGIN();

    switch(error)
    {
        case StackErrors::STACK_CAPACITY_OUT_OF_RANGE:
            LOG_ERR("Stack capacity is out of range.\n");
            break;
        case StackErrors::STACK_IS_NULLPTR:
            LOG_ERR("Stack is nullptr.\n");
            break;
        case StackErrors::STACK_EMPTY_ERR:
            LOG_ERR("Trying to pop from empty stack.\n");
            break;
        case StackErrors::STACK_SIZE_OUT_OF_RANGE:
            LOG_ERR("Stack size is out of range.\n");
            break;
        case StackErrors::STACK_MEMORY_ALLOCATION_ERROR:
            LOG_ERR("Couldn't allocate more memory for stack.\n");
            break;
        case StackErrors::STACK_INVALID_CANARY:
            LOG_ERR("Stack canary is invalid.\n");
            break;
        case StackErrors::STACK_INVALID_DATA_HASH:
            LOG_ERR("Stack data hash is invalid.\n");
            break;
        case StackErrors::STACK_INVALID_STRUCT_HASH:
            LOG_ERR("Stack struct hash is invalid.\n");

        case StackErrors::STACK_NO_ERR:
        default:
            break;
    }

    LOG_END();
}
#undef PRINT_ERR
