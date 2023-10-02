#ifndef COMMANDS_H
#define COMMANDS_H

//TODO подумать че сделать с этими константами
static const char* const Push   = "push";
static const char* const Div    =  "div";
static const char* const Mul    =  "mul";
static const char* const Sub    =  "sub";
static const char* const Add    =  "add";
static const char* const Out    =  "out";
static const char* const Hlt    =  "hlt";

enum class Commands
{
    PUSH,
    MUL,
    ADD, 
    DIV,
    SUB,
    OUT,
    HLT,
};


#endif
