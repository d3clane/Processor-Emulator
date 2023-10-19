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

#define NO_ARGS_INSTRUCTIONS_PRINT_ASM(num) *byteCodePtr++ = num;

DEF_CMD(IN, 2, 
{
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(2);
},
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
    char registerName[RegisterStringLength + 1] = "";

    int scanfResult = sscanf(asmCode.lines[line].line + strlen(POP), "%s", registerName);
    int registerId  = GetRegisterId(registerName);

    *byteCodePtr++ = (int)Commands::POP_ID;

    if (scanfResult == 0 || registerId == -1)
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_SYNTAX);
    else
        *byteCodePtr++ = registerId;
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(4);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(5);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(6);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(7);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(8);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(9);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(10);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(11);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(12);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(13);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(14);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(15);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(16);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(17);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(18);
},
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
    NO_ARGS_INSTRUCTIONS_PRINT_ASM(19);
},
{
    quitCycle = true;
},

static inline void CommandHlt()
{
    return;
}
)

//-----------------------