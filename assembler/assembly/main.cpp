#include <stdio.h>

#include "assembly.h"

int main(const int argc, const char* argv[]) //TODO: argv file in file out
{
    FILE* in = fopen("in.txt", "r");
    FILE* out = fopen("out.txt", "w");

    Assembly(in, out);
}