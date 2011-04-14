# This makefile is for mingw32
# You should modify Constant.h to set DispatcherThreadCount = 1
#   since mingw32 exception cannot cross thread
# If mingw32 version <= 4.5, you should add `-D nullptr=NULL' to CXXFLAGS

CXX := g++
CXXFLAGS := -std=gnu++0x -D __MSVCRT_VERSION__=0x0601 -g
LDFLAGS := -g
LIBS := -lws2_32
BINDIR := bin
OBJDIR := obj
SOURCES := $(wildcard *.cpp)
HEADERS := $(wildcard *.h)
OBJS := $(SOURCES:.cpp=.o)

build: $(BINDIR)/icyhttpd.exe

rebuild: clean build

clean:
	@if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	@if exist $(BINDIR) rmdir /s /q $(BINDIR)

$(BINDIR)/icyhttpd.exe: $(addprefix $(OBJDIR)/,$(OBJS)) | $(BINDIR)
	@echo LD	$@
	@$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	@echo CXX	$@
	@$(CXX) -c $(CXXFLAGS) -o $@ $<

$(BINDIR):
	@mkdir $(BINDIR)

$(OBJDIR):
	@mkdir $(OBJDIR)
