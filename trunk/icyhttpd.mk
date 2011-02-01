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
Date                   :=2011/2/1
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
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/shrimp$(ObjectSuffix) $(IntermediateDirectory)/semaphore$(ObjectSuffix) $(IntermediateDirectory)/timer$(ObjectSuffix) $(IntermediateDirectory)/server$(ObjectSuffix) $(IntermediateDirectory)/socket$(ObjectSuffix) 

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

$(IntermediateDirectory)/shrimp$(ObjectSuffix): shrimp.c $(IntermediateDirectory)/shrimp$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/shrimp.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/shrimp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/shrimp$(DependSuffix): shrimp.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/shrimp$(ObjectSuffix) -MF$(IntermediateDirectory)/shrimp$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/shrimp.c"

$(IntermediateDirectory)/shrimp$(PreprocessSuffix): shrimp.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/shrimp$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/shrimp.c"

$(IntermediateDirectory)/semaphore$(ObjectSuffix): semaphore.c $(IntermediateDirectory)/semaphore$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/semaphore.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/semaphore$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/semaphore$(DependSuffix): semaphore.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/semaphore$(ObjectSuffix) -MF$(IntermediateDirectory)/semaphore$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/semaphore.c"

$(IntermediateDirectory)/semaphore$(PreprocessSuffix): semaphore.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/semaphore$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/semaphore.c"

$(IntermediateDirectory)/timer$(ObjectSuffix): timer.c $(IntermediateDirectory)/timer$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd3/timer.c" $(C_CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/timer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/timer$(DependSuffix): timer.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/timer$(ObjectSuffix) -MF$(IntermediateDirectory)/timer$(DependSuffix) -MM "D:/CodeLite/icyhttpd3/timer.c"

$(IntermediateDirectory)/timer$(PreprocessSuffix): timer.c
	@$(C_CompilerName) $(C_CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/timer$(PreprocessSuffix) "D:/CodeLite/icyhttpd3/timer.c"

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


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/shrimp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/shrimp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/shrimp$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/semaphore$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/semaphore$(DependSuffix)
	$(RM) $(IntermediateDirectory)/semaphore$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/timer$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/timer$(DependSuffix)
	$(RM) $(IntermediateDirectory)/timer$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/server$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/server$(DependSuffix)
	$(RM) $(IntermediateDirectory)/server$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/socket$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/socket$(DependSuffix)
	$(RM) $(IntermediateDirectory)/socket$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe


