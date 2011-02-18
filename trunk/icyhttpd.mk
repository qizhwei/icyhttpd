##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=icyhttpd
ConfigurationName      :=Debug
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "D:\CodeLite\icyhttpd3"
ProjectPath            := "D:\CodeLite\icyhttpd3"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=iceboy
Date                   :=2011/2/18
CodeLitePath           :="C:\Program Files\CodeLite"
LinkerName             :=gcc
ArchiveTool            :=ar rcus
SharedObjectLinkerName :=gcc -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
CompilerName           :=gcc
C_CompilerName         :=gcc
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
MakeDirCommand         :=makedir
CmpOptions             := -g $(Preprocessors)
C_CmpOptions           := -g $(Preprocessors)
LinkOptions            :=  
IncludePath            :=  "$(IncludeSwitch)." "$(IncludeSwitch)." 
RcIncludePath          :=
Libs                   :=$(LibrarySwitch)ws2_32 
LibPath                := "$(LibraryPathSwitch)." 


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/socket$(ObjectSuffix) $(IntermediateDirectory)/process$(ObjectSuffix) $(IntermediateDirectory)/mem$(ObjectSuffix) $(IntermediateDirectory)/dict$(ObjectSuffix) $(IntermediateDirectory)/str$(ObjectSuffix) $(IntermediateDirectory)/message$(ObjectSuffix) $(IntermediateDirectory)/buf$(ObjectSuffix) $(IntermediateDirectory)/echo_handler$(ObjectSuffix) $(IntermediateDirectory)/node$(ObjectSuffix) \
	$(IntermediateDirectory)/runtime$(ObjectSuffix) $(IntermediateDirectory)/endpoint$(ObjectSuffix) $(IntermediateDirectory)/conn$(ObjectSuffix) $(IntermediateDirectory)/event$(ObjectSuffix) $(IntermediateDirectory)/fifo$(ObjectSuffix) 

##
## Main Build Targets 
##
all: $(OutputFile)

$(OutputFile): makeDirStep $(Objects)
	@$(MakeDirCommand) $(@D)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

makeDirStep:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main$(ObjectSuffix): main.c $(IntermediateDirectory)/main$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/main.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/main.c"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/main.c"

$(IntermediateDirectory)/socket$(ObjectSuffix): socket.c $(IntermediateDirectory)/socket$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/socket.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/socket$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/socket$(DependSuffix): socket.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/socket$(ObjectSuffix) -MF$(IntermediateDirectory)/socket$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/socket.c"

$(IntermediateDirectory)/socket$(PreprocessSuffix): socket.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/socket$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/socket.c"

$(IntermediateDirectory)/process$(ObjectSuffix): process.c $(IntermediateDirectory)/process$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/process.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/process$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/process$(DependSuffix): process.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/process$(ObjectSuffix) -MF$(IntermediateDirectory)/process$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/process.c"

$(IntermediateDirectory)/process$(PreprocessSuffix): process.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/process$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/process.c"

$(IntermediateDirectory)/mem$(ObjectSuffix): mem.c $(IntermediateDirectory)/mem$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/mem.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/mem$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mem$(DependSuffix): mem.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mem$(ObjectSuffix) -MF$(IntermediateDirectory)/mem$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/mem.c"

$(IntermediateDirectory)/mem$(PreprocessSuffix): mem.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mem$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/mem.c"

$(IntermediateDirectory)/dict$(ObjectSuffix): dict.c $(IntermediateDirectory)/dict$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/dict.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/dict$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/dict$(DependSuffix): dict.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/dict$(ObjectSuffix) -MF$(IntermediateDirectory)/dict$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/dict.c"

$(IntermediateDirectory)/dict$(PreprocessSuffix): dict.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/dict$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/dict.c"

$(IntermediateDirectory)/str$(ObjectSuffix): str.c $(IntermediateDirectory)/str$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/str.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/str$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/str$(DependSuffix): str.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/str$(ObjectSuffix) -MF$(IntermediateDirectory)/str$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/str.c"

$(IntermediateDirectory)/str$(PreprocessSuffix): str.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/str$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/str.c"

$(IntermediateDirectory)/message$(ObjectSuffix): message.c $(IntermediateDirectory)/message$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/message.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/message$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/message$(DependSuffix): message.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/message$(ObjectSuffix) -MF$(IntermediateDirectory)/message$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/message.c"

$(IntermediateDirectory)/message$(PreprocessSuffix): message.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/message$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/message.c"

$(IntermediateDirectory)/buf$(ObjectSuffix): buf.c $(IntermediateDirectory)/buf$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/buf.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/buf$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/buf$(DependSuffix): buf.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/buf$(ObjectSuffix) -MF$(IntermediateDirectory)/buf$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/buf.c"

$(IntermediateDirectory)/buf$(PreprocessSuffix): buf.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/buf$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/buf.c"

$(IntermediateDirectory)/echo_handler$(ObjectSuffix): echo_handler.c $(IntermediateDirectory)/echo_handler$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/echo_handler.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/echo_handler$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/echo_handler$(DependSuffix): echo_handler.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/echo_handler$(ObjectSuffix) -MF$(IntermediateDirectory)/echo_handler$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/echo_handler.c"

$(IntermediateDirectory)/echo_handler$(PreprocessSuffix): echo_handler.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/echo_handler$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/echo_handler.c"

$(IntermediateDirectory)/node$(ObjectSuffix): node.c $(IntermediateDirectory)/node$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/node.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/node$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/node$(DependSuffix): node.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/node$(ObjectSuffix) -MF$(IntermediateDirectory)/node$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/node.c"

$(IntermediateDirectory)/node$(PreprocessSuffix): node.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/node$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/node.c"

$(IntermediateDirectory)/runtime$(ObjectSuffix): runtime.c $(IntermediateDirectory)/runtime$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/runtime.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/runtime$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/runtime$(DependSuffix): runtime.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/runtime$(ObjectSuffix) -MF$(IntermediateDirectory)/runtime$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/runtime.c"

$(IntermediateDirectory)/runtime$(PreprocessSuffix): runtime.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/runtime$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/runtime.c"

$(IntermediateDirectory)/endpoint$(ObjectSuffix): endpoint.c $(IntermediateDirectory)/endpoint$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/endpoint.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/endpoint$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/endpoint$(DependSuffix): endpoint.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/endpoint$(ObjectSuffix) -MF$(IntermediateDirectory)/endpoint$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/endpoint.c"

$(IntermediateDirectory)/endpoint$(PreprocessSuffix): endpoint.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/endpoint$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/endpoint.c"

$(IntermediateDirectory)/conn$(ObjectSuffix): conn.c $(IntermediateDirectory)/conn$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/conn.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/conn$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/conn$(DependSuffix): conn.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/conn$(ObjectSuffix) -MF$(IntermediateDirectory)/conn$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/conn.c"

$(IntermediateDirectory)/conn$(PreprocessSuffix): conn.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/conn$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/conn.c"

$(IntermediateDirectory)/event$(ObjectSuffix): event.c $(IntermediateDirectory)/event$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/event.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/event$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/event$(DependSuffix): event.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/event$(ObjectSuffix) -MF$(IntermediateDirectory)/event$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/event.c"

$(IntermediateDirectory)/event$(PreprocessSuffix): event.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/event$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/event.c"

$(IntermediateDirectory)/fifo$(ObjectSuffix): fifo.c $(IntermediateDirectory)/fifo$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/fifo.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fifo$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fifo$(DependSuffix): fifo.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/fifo$(ObjectSuffix) -MF$(IntermediateDirectory)/fifo$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/fifo.c"

$(IntermediateDirectory)/fifo$(PreprocessSuffix): fifo.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fifo$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/fifo.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/socket$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/socket$(DependSuffix)
	$(RM) $(IntermediateDirectory)/socket$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/process$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/process$(DependSuffix)
	$(RM) $(IntermediateDirectory)/process$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/mem$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/mem$(DependSuffix)
	$(RM) $(IntermediateDirectory)/mem$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/dict$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/dict$(DependSuffix)
	$(RM) $(IntermediateDirectory)/dict$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/str$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/str$(DependSuffix)
	$(RM) $(IntermediateDirectory)/str$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/message$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/message$(DependSuffix)
	$(RM) $(IntermediateDirectory)/message$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/buf$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/buf$(DependSuffix)
	$(RM) $(IntermediateDirectory)/buf$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/echo_handler$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/echo_handler$(DependSuffix)
	$(RM) $(IntermediateDirectory)/echo_handler$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/node$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/node$(DependSuffix)
	$(RM) $(IntermediateDirectory)/node$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/runtime$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/runtime$(DependSuffix)
	$(RM) $(IntermediateDirectory)/runtime$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/endpoint$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/endpoint$(DependSuffix)
	$(RM) $(IntermediateDirectory)/endpoint$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/conn$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/conn$(DependSuffix)
	$(RM) $(IntermediateDirectory)/conn$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/event$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/event$(DependSuffix)
	$(RM) $(IntermediateDirectory)/event$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fifo$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fifo$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fifo$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe


