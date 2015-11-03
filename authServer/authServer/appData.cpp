#include "preCompiled.h"
#include "appData.h"

//전역 디비 객체
defDb db_;

bool validDurationTokenType(int durationTokenType)
{
    if (durationTokenType < oneHourToken ||
        durationTokenType >= maxToken)
        return false;
    return true;
}