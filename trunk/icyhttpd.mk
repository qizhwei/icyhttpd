##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=icyhttpd
ConfigurationName      :=Debug
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "D:\CodeLite\icyhttpd"
ProjectPath            := "D:\CodeLite\icyhttpd"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=iceboy
Date                   :=10/30/10
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
LinkOptions            :=  
IncludePath            :=  "$(IncludeSwitch)." "$(IncludeSwitch)." 
RcIncludePath          :=
Libs                   :=
LibPath                := "$(LibraryPathSwitch)." 


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
UNIT_TEST_PP_SRC_DIR:=C:\UnitTest++-1.3
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/fcpool$(ObjectSuffix) $(IntermediateDirectory)/fcinit$(ObjectSuffix) $(IntermediateDirectory)/fcproc$(ObjectSuffix) $(IntermediateDirectory)/fcreq$(ObjectSuffix) $(IntermediateDirectory)/fcstd$(ObjectSuffix) $(IntermediateDirectory)/rtlheap$(ObjectSuffix) $(IntermediateDirectory)/rtlstr$(ObjectSuffix) $(IntermediateDirectory)/rtlpipe$(ObjectSuffix) $(IntermediateDirectory)/rtlfifo$(ObjectSuffix) \
	

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
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/main.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "D:/CodeLite/icyhttpd/main.c"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "D:/CodeLite/icyhttpd/main.c"

$(IntermediateDirectory)/fcpool$(ObjectSuffix): fcpool.c $(IntermediateDirectory)/fcpool$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fcpool.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fcpool$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fcpool$(DependSuffix): fcpool.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fcpool$(ObjectSuffix) -MF$(IntermediateDirectory)/fcpool$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fcpool.c"

$(IntermediateDirectory)/fcpool$(PreprocessSuffix): fcpool.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fcpool$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fcpool.c"

$(IntermediateDirectory)/fcinit$(ObjectSuffix): fcinit.c $(IntermediateDirectory)/fcinit$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fcinit.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fcinit$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fcinit$(DependSuffix): fcinit.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fcinit$(ObjectSuffix) -MF$(IntermediateDirectory)/fcinit$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fcinit.c"

$(IntermediateDirectory)/fcinit$(PreprocessSuffix): fcinit.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fcinit$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fcinit.c"

$(IntermediateDirectory)/fcproc$(ObjectSuffix): fcproc.c $(IntermediateDirectory)/fcproc$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fcproc.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fcproc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fcproc$(DependSuffix): fcproc.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fcproc$(ObjectSuffix) -MF$(IntermediateDirectory)/fcproc$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fcproc.c"

$(IntermediateDirectory)/fcproc$(PreprocessSuffix): fcproc.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fcproc$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fcproc.c"

$(IntermediateDirectory)/fcreq$(ObjectSuffix): fcreq.c $(IntermediateDirectory)/fcreq$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fcreq.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fcreq$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fcreq$(DependSuffix): fcreq.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fcreq$(ObjectSuffix) -MF$(IntermediateDirectory)/fcreq$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fcreq.c"

$(IntermediateDirectory)/fcreq$(PreprocessSuffix): fcreq.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fcreq$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fcreq.c"

$(IntermediateDirectory)/fcstd$(ObjectSuffix): fcstd.c $(IntermediateDirectory)/fcstd$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fcstd.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fcstd$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fcstd$(DependSuffix): fcstd.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fcstd$(ObjectSuffix) -MF$(IntermediateDirectory)/fcstd$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fcstd.c"

$(IntermediateDirectory)/fcstd$(PreprocessSuffix): fcstd.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fcstd$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fcstd.c"

$(IntermediateDirectory)/rtlheap$(ObjectSuffix): rtlheap.c $(IntermediateDirectory)/rtlheap$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/rtlheap.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/rtlheap$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rtlheap$(DependSuffix): rtlheap.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/rtlheap$(ObjectSuffix) -MF$(IntermediateDirectory)/rtlheap$(DependSuffix) -MM "D:/CodeLite/icyhttpd/rtlheap.c"

$(IntermediateDirectory)/rtlheap$(PreprocessSuffix): rtlheap.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rtlheap$(PreprocessSuffix) "D:/CodeLite/icyhttpd/rtlheap.c"

$(IntermediateDirectory)/rtlstr$(ObjectSuffix): rtlstr.c $(IntermediateDirectory)/rtlstr$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/rtlstr.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/rtlstr$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rtlstr$(DependSuffix): rtlstr.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/rtlstr$(ObjectSuffix) -MF$(IntermediateDirectory)/rtlstr$(DependSuffix) -MM "D:/CodeLite/icyhttpd/rtlstr.c"

$(IntermediateDirectory)/rtlstr$(PreprocessSuffix): rtlstr.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rtlstr$(PreprocessSuffix) "D:/CodeLite/icyhttpd/rtlstr.c"

$(IntermediateDirectory)/rtlpipe$(ObjectSuffix): rtlpipe.c $(IntermediateDirectory)/rtlpipe$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/rtlpipe.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/rtlpipe$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rtlpipe$(DependSuffix): rtlpipe.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/rtlpipe$(ObjectSuffix) -MF$(IntermediateDirectory)/rtlpipe$(DependSuffix) -MM "D:/CodeLite/icyhttpd/rtlpipe.c"

$(IntermediateDirectory)/rtlpipe$(PreprocessSuffix): rtlpipe.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rtlpipe$(PreprocessSuffix) "D:/CodeLite/icyhttpd/rtlpipe.c"

$(IntermediateDirectory)/rtlfifo$(ObjectSuffix): rtlfifo.c $(IntermediateDirectory)/rtlfifo$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/rtlfifo.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/rtlfifo$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rtlfifo$(DependSuffix): rtlfifo.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/rtlfifo$(ObjectSuffix) -MF$(IntermediateDirectory)/rtlfifo$(DependSuffix) -MM "D:/CodeLite/icyhttpd/rtlfifo.c"

$(IntermediateDirectory)/rtlfifo$(PreprocessSuffix): rtlfifo.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rtlfifo$(PreprocessSuffix) "D:/CodeLite/icyhttpd/rtlfifo.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fcpool$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fcpool$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fcpool$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fcinit$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fcinit$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fcinit$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fcproc$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fcproc$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fcproc$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fcreq$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fcreq$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fcreq$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fcstd$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fcstd$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fcstd$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/rtlheap$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/rtlheap$(DependSuffix)
	$(RM) $(IntermediateDirectory)/rtlheap$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/rtlstr$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/rtlstr$(DependSuffix)
	$(RM) $(IntermediateDirectory)/rtlstr$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/rtlpipe$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/rtlpipe$(DependSuffix)
	$(RM) $(IntermediateDirectory)/rtlpipe$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/rtlfifo$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/rtlfifo$(DependSuffix)
	$(RM) $(IntermediateDirectory)/rtlfifo$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe


