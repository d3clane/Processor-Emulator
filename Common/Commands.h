DEF_CMD(PUSH_REGISTER, 0, 1,
{
    SpuError = CommandPushRegister(&spu);
})

DEF_CMD(PUSH, 1, 1,
{
    SpuError = CommandPush(&spu);
})
DEF_CMD(IN, 2, 0,
{
    SpuError = CommandIn(&spu);
})

DEF_CMD(POP, 3, 1,
{
    SpuError = CommandPop(&spu);
})

DEF_CMD(MUL, 4, 0,
{
    SpuError = CallBinaryCommand(CommandMul, &spu);
})

DEF_CMD(ADD, 5, 0,
{
    SpuError = CallBinaryCommand(CommandAdd, &spu);
})

DEF_CMD(DIV, 6, 0,
{
    SpuError = CallBinaryCommand(CommandDiv, &spu);
})

DEF_CMD(SUB, 7, 0,
{
    SpuError = CallBinaryCommand(CommandSub, &spu);
})

DEF_CMD(SIN, 8, 0,
{
    SpuError = CallUnaryCommand(CommandSin, &spu);
})

DEF_CMD(COS,  9, 0,
{
    SpuError = CallUnaryCommand(CommandCos, &spu);
})
DEF_CMD(TAN, 10, 0,
{
    SpuError = CallUnaryCommand(CommandTan, &spu);
})

DEF_CMD(COT, 11, 0,
{
    SpuError = CallUnaryCommand(CommandCot, &spu);
})

DEF_CMD(SQRT, 12, 0,
{
    SpuError = CallUnaryCommand(CommandSqrt, &spu);
})

DEF_CMD(POW , 13, 0,
{
    SpuError = CallUnaryCommand(CommandPow2, &spu);
})

DEF_CMD(MEOW , 14, 0,
{
    SpuError = CommandMeow();
})

DEF_CMD(BARK , 15, 0,
{
    SpuError = CommandBark();
})

DEF_CMD(SLEEP, 16, 0,
{
    SpuError = CommandSleep();
})

DEF_CMD(BOTAY, 17, 0,
{
    SpuError = CommandBotay();
})

DEF_CMD(OUT, 18, 0,
{
    SpuError = CallUnaryCommand(CommandOut, &spu);
})

DEF_CMD(HLT, 19, 0,
{
    quitCycle = true;
})
