#include "obp.h"
#include "rtl.h"

void ObInitializeObjectType(ObObjectType *type, ObCloseRoutine *close, ObParseRoutine *parse)
{
	type->Close = close;
	type->Parse = parse;
}
