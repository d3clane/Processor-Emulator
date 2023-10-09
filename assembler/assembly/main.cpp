#include <stdio.h>

#include "assembly.h"

//docs - first command arg - in, second - out
int main(const int argc, const char* argv[])
{
    LogOpen(argv[0]);
    
    FILE* in  = nullptr;
    FILE* out = nullptr;

    if (argc > 1)
        in = fopen(argv[1], "r");
    else
        in = fopen("in.txt", "r");
    
    if (argc > 2)
        out = fopen(argv[2], "wb");
    else
        out = fopen("out.bin", "wb");

    Assembly(in, out);

    fclose(in);
    fclose(out);
}
