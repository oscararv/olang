#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include "token.h"


int main() {
    struct tokenContext tc;
    tc.fileName = "test.c";
    SyntaxErrorInvalidChar(&tc, 'c', 1, 2, "expected no characters");
    return EXIT_SUCCESS;
}


//current segfault is to be ignored
