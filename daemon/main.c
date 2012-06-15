#include "daemon.h"
#include "handler.h"

static void DmInitHandlers(void)
{
	DM_HANDLER_TYPE **type;
	CSTATUS status;

	for (type = DmHandlerList; *type != NULL; ++type) {
		status = DmRegisterHandler(*type);
		if (!SUCCESS(status)) {
			LogPrintf("failed to load \'%s\'\n", (*type)->Name, (int)status);
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
