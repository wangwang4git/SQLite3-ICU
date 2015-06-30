#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unicode/ustring.h>
#include <unicode/uregex.h>

int main(int argc, char** argv)
{
    UErrorCode status = U_ZERO_ERROR;
    UParseError parseErr;
    char* pattern = "\\p{script=han}{1}";
    URegularExpression* reExp = uregex_openC(pattern, 0, &parseErr, &status);
    if (U_FAILURE(status))
    {
        printf("error in pattern: \"%s\" at position %d\n", u_errorName(status), parseErr.offset);
        exit(-1);
    }

    char* input = "哈哈";
    UChar stringToExamine[strlen(input) + 1];
    u_uastrcpy(stringToExamine, input);

    status = U_ZERO_ERROR;
    uregex_setText(reExp, stringToExamine, u_strlen(stringToExamine), &status);
    if (U_FAILURE(status))
    {
        printf("uregex_setUText error: %s\n", u_errorName(status));
        exit(1);
    }

    status = U_ZERO_ERROR;
    UBool doMatch = uregex_matches64(reExp, 0, &status);
    if (U_FAILURE(status))
    {
        printf("uregex_matches64 error: %s\n", u_errorName(status));
        exit(1);
    }

    printf("uregex_matches64 %s\n", doMatch == TRUE ? "TRUE" : "FALSE");

    uregex_close(reExp);

    return 0;
}
