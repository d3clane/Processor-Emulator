#include <stdio.h>

#include "Disassembly.h"

//docs - first command arg - in, second - out
int main(const int argc, const char* argv[])
{
    FILE* in  = nullptr;
    FILE* out = nullptr;

    if (argc > 1)
        in = fopen(argv[1], "r");
    else
        in = fopen("in.txt", "r");

    if (argc > 2)
        out = fopen(argv[2], "w");
    else
        out = fopen("out.txt", "w");

    Disassembly(in, out);
}