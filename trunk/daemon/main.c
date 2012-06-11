#include "daemon.h"
#include "handler.h"

static void DmInitHandlers(void)
{
	DM_HANDLER_TYPE **type;
	CSTATUS status;

	LogPrintf("Loading handlers...\n");
	for (type = DmHandlerList; *type != NULL; ++type) {
		status = DmRegisterHandler(*type);
		if (SUCCESS(status)) {
			LogPrintf("  %s\n", (*type)->Name);
		} else {
			LogPrintf("  %s [failed with %d]\n", (*type)->Name, (int)status);
		}
	}
}

int main(int argc, char *argv[])
{
	IoInitSystem();
	DmInitServer();
	DmInitHandlers();
	DmConfigParserEntry();
	IoMainLoop();
}
