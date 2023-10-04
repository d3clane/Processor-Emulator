#include <stdio.h>

#include "SPU.h"
#include "Log.h"

int main(const int argc, const char* const argv[])
{
    LogOpen(argv[0]);
    
    FILE* in  = nullptr;
    if (argc > 1)
        in = fopen(argv[1], "r");
    else
        in = fopen("in.txt", "r");

    Processing(in);

    fclose(in);
}  