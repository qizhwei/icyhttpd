#include "daemon.h"

int main(int argc, char *argv[])
{
	IoInitSystem();
	DmInitServer();
	DmInitStaticHandler();
	DmConfigParserEntry();
	IoMainLoop();
}
