#ifndef COMMANDS_H
#define COMMANDS_H

//TODO подумать че сделать с этими константами
static const char* const PUSH   = "push";
static const char* const IN     =   "in";

static const char* const DIV    =  "div";
static const char* const MUL    =  "mul";
static const char* const SUB    =  "sub";
static const char* const ADD    =  "add";

static const char* const OUT    =  "out";
static const char* const HLT    =  "hlt";

enum class Commands
{
    PUSH_ID,
    IN_ID,

    MUL_ID,
    ADD_ID, 
    DIV_ID,
    SUB_ID,
    OUT_ID,
    HLT_ID,
};

#endif
