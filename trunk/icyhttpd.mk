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
Date                   :=2011/2/12
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
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/server$(ObjectSuffix) $(IntermediateDirectory)/socket$(ObjectSuffix) $(IntermediateDirectory)/process$(ObjectSuffix) $(IntermediateDirectory)/mem$(ObjectSuffix) $(IntermediateDirectory)/conn$(ObjectSuffix) $(IntermediateDirectory)/dict$(ObjectSuffix) $(IntermediateDirectory)/str$(ObjectSuffix) 

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

$(IntermediateDirectory)/server$(ObjectSuffix): server.c $(IntermediateDirectory)/server$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/server.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/server$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/server$(DependSuffix): server.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/server$(ObjectSuffix) -MF$(IntermediateDirectory)/server$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/server.c"

$(IntermediateDirectory)/server$(PreprocessSuffix): server.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/server$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/server.c"

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

$(IntermediateDirectory)/conn$(ObjectSuffix): conn.c $(IntermediateDirectory)/conn$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/conn.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/conn$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/conn$(DependSuffix): conn.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/conn$(ObjectSuffix) -MF$(IntermediateDirectory)/conn$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/conn.c"

$(IntermediateDirectory)/conn$(PreprocessSuffix): conn.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/conn$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/conn.c"

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


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/server$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/server$(DependSuffix)
	$(RM) $(IntermediateDirectory)/server$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/socket$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/socket$(DependSuffix)
	$(RM) $(IntermediateDirectory)/socket$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/process$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/process$(DependSuffix)
	$(RM) $(IntermediateDirectory)/process$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/mem$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/mem$(DependSuffix)
	$(RM) $(IntermediateDirectory)/mem$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/conn$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/conn$(DependSuffix)
	$(RM) $(IntermediateDirectory)/conn$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/dict$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/dict$(DependSuffix)
	$(RM) $(IntermediateDirectory)/dict$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/str$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/str$(DependSuffix)
	$(RM) $(IntermediateDirectory)/str$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe


