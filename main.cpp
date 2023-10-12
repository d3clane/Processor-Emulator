#include <stdio.h>

#include "Log.h"
#include "SPU.h"

int main(const int argc, const char* const argv[])
{
    LogOpen(argv[0]);

    FILE* in  = nullptr;
    if (argc > 1)
        in = fopen(argv[1], "r");
    else
        in = fopen("in.bin", "rb");

    Processing(in);

    fclose(in);
}