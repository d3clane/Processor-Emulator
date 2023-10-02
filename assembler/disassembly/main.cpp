#include <stdio.h>

#include "Disassembly.h"

int main(const int argc, const char* argv[]) //TODO: argv file in file out
{
    FILE* in = fopen("in.txt", "r");
    FILE* out = fopen("out.txt", "w");

    Disassembly(in, out);
}