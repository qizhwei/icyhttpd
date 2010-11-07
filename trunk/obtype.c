#include "obp.h"
#include "rtl.h"

void ObInitializeObjectType(ObObjectType *type, ObCloseRoutine *close, ObParseRoutine *parse, ObInsertRoutine *insert)
{
	type->Close = close;
	type->Parse = parse;
	type->Insert = insert;
}
