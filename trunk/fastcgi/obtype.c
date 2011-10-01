#include "obp.h"
#include "rtl.h"

void ObInitializeObjectType(ObObjectType *type, ObObjectType *parentType, ObCloseRoutine *close, ObParseRoutine *parse, ObInsertRoutine *insert)
{
	type->ParentType = parentType;
	type->Close = close;
	type->Parse = parse;
	type->Insert = insert;
}
