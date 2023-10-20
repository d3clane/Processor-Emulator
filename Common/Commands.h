#define PUSH_PRINT_ASM  \
{   \
    int value = -1;     \
    int scanfResult = sscanf(asmCode.lines[line].line + strlen(PUSH), "%d", &value); \
    \
    if (scanfResult == 1)       \
    {   \
        *byteCodePtr++ = (int)Commands::PUSH_ID;   \
        *byteCodePtr++ = value;    \
    } \
    else \
    {   \
        char registerName[RegisterStringLength + 1] = ""; \
                                                                 \
        scanfResult = sscanf(asmCode.lines[line].line + strlen(PUSH), "%s", registerName); \
        int registerId = GetRegisterId(registerName); \
                                        \
        if (registerId == -1) \
        { \
            COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_SYNTAX); \
        }   \
        else \
        {   \
            *byteCodePtr++ = (int)Commands::PUSH_REGISTER_ID; \
            *byteCodePtr++ = registerId; \
        } \
    }   \
}

DEF_CMD(PUSH_REGISTER, 0,
{
    PUSH_PRINT_ASM;
},
CommandArguments::ONE_REGISTER_ID,
{
    SpuError = CommandPushRegister(&spu);
},

static SpuErrors CommandPushRegister(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int registerId = *spu->byteCodeArrayReadPtr++;

    if (registerId < 0 || (size_t)registerId >= NumberOfRegisters)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_REGISTER);
                      return SpuErrors::INVALID_REGISTER;
    }

    assert(0 <= registerId && (size_t) registerId < NumberOfRegisters);

    STACK_PUSH(spu, spu->registers[registerId]);

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;
}
)

//------------------------

DEF_CMD(PUSH, 1, 
{
    PUSH_PRINT_ASM
},
CommandArguments::ONE_INT_VALUE_ID,
{
    SpuError = CommandPush(&spu);
},

static SpuErrors CommandPush(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int valueToPush = *spu->byteCodeArrayReadPtr++;
    valueToPush *= CalculatingPrecision;

    STACK_PUSH(spu, valueToPush);

    SPU_CHECK(spu);
    
    return SpuErrors::NO_ERR;
}
)

#undef PUSH_PRINT_ASM

//---------------------

#define PRINT_COMMAND_ID_ASM(num) *byteCodePtr++ = num;

DEF_CMD(IN, 2, 
{
    PRINT_COMMAND_ID_ASM(2);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CommandIn(&spu);
},

static SpuErrors CommandIn(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int valueToPush = POISON;
    
    printf("Enter value: ");
    int scanfResult = scanf(ElemTypeFormat, &valueToPush);
    
    if (scanfResult != 1)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::READING_FROM_BYTE_CODE_ERR);
                      return SpuErrors::READING_FROM_BYTE_CODE_ERR;
    }
    
    valueToPush *= CalculatingPrecision;

    STACK_PUSH(spu, valueToPush);

    SPU_CHECK(spu);
    
    return SpuErrors::NO_ERR;
}
)

//-------------------------------

DEF_CMD(POP, 3, 
{
    PRINT_COMMAND_ID_ASM(3);
    int copyResult = CopyRegisterArgument(asmCode.lines[line].line + strlen(POP), byteCodePtr, &byteCodePtr);

    if (copyResult == 0)
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_SYNTAX);
},
CommandArguments::ONE_REGISTER_ID,
{
    SpuError = CommandPop(&spu);
},

static SpuErrors CommandPop(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int registerId = *spu->byteCodeArrayReadPtr++;

    if (registerId < 0 || (size_t)registerId >= NumberOfRegisters)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::INVALID_REGISTER);
                      return SpuErrors::INVALID_REGISTER;
    }

    assert(0 <= registerId && (size_t)registerId < NumberOfRegisters);

    StackErrorsType stackError = StackPop(&spu->stack, &spu->registers[registerId]);

    if (stackError)
    {
        SPU_ERRORS_LOG_ERROR(SpuErrors::STACK_ERR);
                      return SpuErrors::STACK_ERR;
    }

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(MUL, 4,  
{
    PRINT_COMMAND_ID_ASM(4);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallBinaryCommand(CommandMul, &spu);
},

static inline SpuErrors CommandMul(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = inFirstValue * inSecondValue / CalculatingPrecision;

    return SpuErrors::NO_ERR;
}
)

//-----------------------


DEF_CMD(ADD, 5,  
{
    PRINT_COMMAND_ID_ASM(5);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallBinaryCommand(CommandAdd, &spu);
},

static inline SpuErrors CommandAdd(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = inFirstValue + inSecondValue;

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(DIV, 6,  
{
    PRINT_COMMAND_ID_ASM(6);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallBinaryCommand(CommandDiv, &spu);
},

static inline SpuErrors CommandDiv(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    if (inSecondValue == 0)
    {
        *outValue = POISON;
        SPU_ERRORS_LOG_ERROR(SpuErrors::TRYING_TO_DIVIDE_ON_ZERO);
                      return SpuErrors::TRYING_TO_DIVIDE_ON_ZERO;
    }

    *outValue = CalculatingPrecision * inFirstValue / inSecondValue;
    
    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(SUB, 7,  
{
    PRINT_COMMAND_ID_ASM(7);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallBinaryCommand(CommandSub, &spu);
},

static inline SpuErrors CommandSub(int inFirstValue, int inSecondValue, int* outValue)
{
    assert(outValue);

    VALUES_CHECK(inFirstValue, inSecondValue);

    *outValue = inFirstValue - inSecondValue;

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(SIN, 8,  
{
    PRINT_COMMAND_ID_ASM(8);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallUnaryCommand(CommandSin, &spu);
},

static inline SpuErrors CommandSin(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(sin(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(COS,  9,  
{
    PRINT_COMMAND_ID_ASM(9);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallUnaryCommand(CommandCos, &spu);
},

static inline SpuErrors CommandCos(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(cos(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(TAN, 10,  
{
    PRINT_COMMAND_ID_ASM(10);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallUnaryCommand(CommandTan, &spu);
},

static inline SpuErrors CommandTan(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(tan(1.0 * inValue / CalculatingPrecision) * CalculatingPrecision);

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(COT, 11,  
{
    PRINT_COMMAND_ID_ASM(11);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallUnaryCommand(CommandCot, &spu);
},

static inline SpuErrors CommandCot(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(CalculatingPrecision / tan(1.0 * inValue / CalculatingPrecision));

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(SQRT, 12,  
{
    PRINT_COMMAND_ID_ASM(12);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallUnaryCommand(CommandSqrt, &spu);
},

static inline SpuErrors CommandSqrt(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = (int)(sqrt(inValue) * sqrt(CalculatingPrecision));

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(POW2, 13,  
{
    PRINT_COMMAND_ID_ASM(13);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallUnaryCommand(CommandPow2, &spu);
},

static inline SpuErrors CommandPow2(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    *outValue = inValue * inValue / CalculatingPrecision;

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(MEOW , 14,  
{
    PRINT_COMMAND_ID_ASM(14);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CommandMeow();
},

static inline SpuErrors CommandMeow()
{
    system("say -v cello \"Meow meow meow meow\"");

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(BARK , 15,  
{
    PRINT_COMMAND_ID_ASM(15);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CommandBark();
},

static inline SpuErrors CommandBark()
{
    system("say -v \"Bad Bark bark bark bark\"");

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(SLEEP, 16,  
{
    PRINT_COMMAND_ID_ASM(16);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CommandSleep();
},

static inline SpuErrors CommandSleep()
{
    system("say -v Bad \"It's time to sleep\"");

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(BOTAY, 17,  
{
    PRINT_COMMAND_ID_ASM(17);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CommandBotay();
},

static inline SpuErrors CommandBotay()
{
    system("say \"время ботать\"");

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(OUT, 18,  
{
    PRINT_COMMAND_ID_ASM(18);
},
CommandArguments::NO_ARGS_ID,
{
    SpuError = CallUnaryCommand(CommandOut, &spu);
},

static inline SpuErrors CommandOut(int inValue, int* outValue)
{
    assert(outValue);

    VALUE_CHECK(inValue);

    printf("Equation result: " "%lf" "\n", 1.0 * inValue / CalculatingPrecision);

    return SpuErrors::NO_ERR;
}
)

//-----------------------

DEF_CMD(HLT, 19,  
{
    PRINT_COMMAND_ID_ASM(19);
},
CommandArguments::NO_ARGS_ID,
{
    quitCycle = true;
},

static inline void CommandHlt()
{
    return;
}
)

#define PRINT_JMP_ASM(name, num) \
{   \
    PRINT_COMMAND_ID_ASM(num); \
\
    int copyResult = CopyIntArgument(asmCode.lines[line].line + strlen(#name), byteCodePtr, &byteCodePtr); \
\
    if (copyResult == 0) \
    { \
        static const char maxLabelLength      = 32;\
        static char labelName[maxLabelLength] = "";\
\
        sscanf(asmCode.lines[line].line + strlen(#name),  "%s", labelName);\
\
        LabelType* tmpLabel = GetLabel(labels, maxNumberOfLabels, labelName);\
\
        if (tmpLabel == nullptr)\
            *byteCodePtr++ = -1;\
        else\
            *byteCodePtr++ = tmpLabel->jmpAdress;\
    }\
}

DEF_CMD(JMP, 20,
{
    PRINT_JMP_ASM(JMP, 20);
},
CommandArguments::ONE_INT_VALUE_ID,
{
    SpuError = CommandJmp(&spu);
},

static inline SpuErrors CommandJmp(SpuType* spu)
{
    assert(spu);

    SPU_CHECK(spu);

    int posToJump = *spu->byteCodeArrayReadPtr++;

    assert(posToJump > 0);

    spu->byteCodeArrayReadPtr = spu->byteCodeArray + posToJump;

    SPU_CHECK(spu);

    return SpuErrors::NO_ERR;    
}
)

#define COMMAND_JUMP_IF(comparisonSign) \
{\
    assert(spu);\
\
    SPU_CHECK(spu);\
\
    int firstValue  = POISON;\
    int secondValue = POISON;\
\
    SpuErrors error = GetTwoLastValuesFromStack(&spu->stack, &firstValue, &secondValue);\
\
    IF_ERR_RETURN(error);\
    \
    if (firstValue comparisonSign secondValue)\
    {\
        error = CommandJmp(spu);\
    }\
\
    return error; \
}

DEF_CMD(JB, 21, 
{
    PRINT_JMP_ASM(JB, 21);
},
CommandArguments::ONE_INT_VALUE_ID,
{
    SpuError = CommandJb(&spu);
},

static inline SpuErrors CommandJb(SpuType* spu)
{
    COMMAND_JUMP_IF(<);
}
)

//-----------------------

DEF_CMD(JA, 22, 
{
    PRINT_JMP_ASM(JA, 22);
},
CommandArguments::ONE_INT_VALUE_ID,
{
    SpuError = CommandJa(&spu);
},

static inline SpuErrors CommandJa(SpuType* spu)
{
    COMMAND_JUMP_IF(>);
}
)

//-----------------------

DEF_CMD(JAE, 23, 
{
    PRINT_JMP_ASM(JAE, 23);
},
CommandArguments::ONE_INT_VALUE_ID,
{
    SpuError = CommandJae(&spu);
},

static inline SpuErrors CommandJae(SpuType* spu)
{
    COMMAND_JUMP_IF(>=);
}
)

//-----------------------

DEF_CMD(JBE, 24, 
{
    PRINT_JMP_ASM(JBE, 24);
},
CommandArguments::ONE_INT_VALUE_ID,
{
    SpuError = CommandJbe(&spu);
},

static inline SpuErrors CommandJbe(SpuType* spu)
{
    COMMAND_JUMP_IF(<=);
}
)

//-----------------------

DEF_CMD(JE, 25, 
{
    PRINT_JMP_ASM(JE, 25);
},
CommandArguments::ONE_INT_VALUE_ID,
{
    SpuError = CommandJe(&spu);
},

static inline SpuErrors CommandJe(SpuType* spu)
{
    COMMAND_JUMP_IF(==);
}
)

//-----------------------

DEF_CMD(JNE, 26, 
{
    PRINT_JMP_ASM(JNE, 26);
},
CommandArguments::ONE_INT_VALUE_ID,
{
    SpuError = CommandJne(&spu);
},

static inline SpuErrors CommandJne(SpuType* spu)
{
    COMMAND_JUMP_IF(!=);
}
)

#undef PRINT_JMP_ASM
#undef COMMAND_JUMP_IF
//-----------------------

#undef PRINT_COMMAND_ID_ASM