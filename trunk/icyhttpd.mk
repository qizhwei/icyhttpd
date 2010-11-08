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
Date                   :=11/08/10
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
Libs                   :=$(LibrarySwitch)ws2_32 
LibPath                := "$(LibraryPathSwitch)." 


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
UNIT_TEST_PP_SRC_DIR:=C:\UnitTest++-1.3
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/fcpool$(ObjectSuffix) $(IntermediateDirectory)/fcinit$(ObjectSuffix) $(IntermediateDirectory)/fcproc$(ObjectSuffix) $(IntermediateDirectory)/fcreq$(ObjectSuffix) $(IntermediateDirectory)/fcdisp$(ObjectSuffix) $(IntermediateDirectory)/rtlheap$(ObjectSuffix) $(IntermediateDirectory)/rtlstr$(ObjectSuffix) $(IntermediateDirectory)/rtlpipe$(ObjectSuffix) $(IntermediateDirectory)/rtlfifo$(ObjectSuffix) \
	$(IntermediateDirectory)/rtlmap$(ObjectSuffix) $(IntermediateDirectory)/svinit$(ObjectSuffix) $(IntermediateDirectory)/svsite$(ObjectSuffix) $(IntermediateDirectory)/sventry$(ObjectSuffix) $(IntermediateDirectory)/svlisten$(ObjectSuffix) $(IntermediateDirectory)/svss$(ObjectSuffix) $(IntermediateDirectory)/svconn$(ObjectSuffix) $(IntermediateDirectory)/obtype$(ObjectSuffix) $(IntermediateDirectory)/obobject$(ObjectSuffix) $(IntermediateDirectory)/obdir$(ObjectSuffix) \
	$(IntermediateDirectory)/obinit$(ObjectSuffix) $(IntermediateDirectory)/fsinit$(ObjectSuffix) $(IntermediateDirectory)/fsfile$(ObjectSuffix) $(IntermediateDirectory)/fsvd$(ObjectSuffix) 

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

$(IntermediateDirectory)/fcdisp$(ObjectSuffix): fcdisp.c $(IntermediateDirectory)/fcdisp$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fcdisp.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fcdisp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fcdisp$(DependSuffix): fcdisp.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fcdisp$(ObjectSuffix) -MF$(IntermediateDirectory)/fcdisp$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fcdisp.c"

$(IntermediateDirectory)/fcdisp$(PreprocessSuffix): fcdisp.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fcdisp$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fcdisp.c"

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

$(IntermediateDirectory)/rtlmap$(ObjectSuffix): rtlmap.c $(IntermediateDirectory)/rtlmap$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/rtlmap.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/rtlmap$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/rtlmap$(DependSuffix): rtlmap.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/rtlmap$(ObjectSuffix) -MF$(IntermediateDirectory)/rtlmap$(DependSuffix) -MM "D:/CodeLite/icyhttpd/rtlmap.c"

$(IntermediateDirectory)/rtlmap$(PreprocessSuffix): rtlmap.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/rtlmap$(PreprocessSuffix) "D:/CodeLite/icyhttpd/rtlmap.c"

$(IntermediateDirectory)/svinit$(ObjectSuffix): svinit.c $(IntermediateDirectory)/svinit$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/svinit.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/svinit$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/svinit$(DependSuffix): svinit.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/svinit$(ObjectSuffix) -MF$(IntermediateDirectory)/svinit$(DependSuffix) -MM "D:/CodeLite/icyhttpd/svinit.c"

$(IntermediateDirectory)/svinit$(PreprocessSuffix): svinit.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/svinit$(PreprocessSuffix) "D:/CodeLite/icyhttpd/svinit.c"

$(IntermediateDirectory)/svsite$(ObjectSuffix): svsite.c $(IntermediateDirectory)/svsite$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/svsite.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/svsite$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/svsite$(DependSuffix): svsite.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/svsite$(ObjectSuffix) -MF$(IntermediateDirectory)/svsite$(DependSuffix) -MM "D:/CodeLite/icyhttpd/svsite.c"

$(IntermediateDirectory)/svsite$(PreprocessSuffix): svsite.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/svsite$(PreprocessSuffix) "D:/CodeLite/icyhttpd/svsite.c"

$(IntermediateDirectory)/sventry$(ObjectSuffix): sventry.c $(IntermediateDirectory)/sventry$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/sventry.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/sventry$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/sventry$(DependSuffix): sventry.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/sventry$(ObjectSuffix) -MF$(IntermediateDirectory)/sventry$(DependSuffix) -MM "D:/CodeLite/icyhttpd/sventry.c"

$(IntermediateDirectory)/sventry$(PreprocessSuffix): sventry.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/sventry$(PreprocessSuffix) "D:/CodeLite/icyhttpd/sventry.c"

$(IntermediateDirectory)/svlisten$(ObjectSuffix): svlisten.c $(IntermediateDirectory)/svlisten$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/svlisten.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/svlisten$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/svlisten$(DependSuffix): svlisten.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/svlisten$(ObjectSuffix) -MF$(IntermediateDirectory)/svlisten$(DependSuffix) -MM "D:/CodeLite/icyhttpd/svlisten.c"

$(IntermediateDirectory)/svlisten$(PreprocessSuffix): svlisten.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/svlisten$(PreprocessSuffix) "D:/CodeLite/icyhttpd/svlisten.c"

$(IntermediateDirectory)/svss$(ObjectSuffix): svss.c $(IntermediateDirectory)/svss$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/svss.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/svss$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/svss$(DependSuffix): svss.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/svss$(ObjectSuffix) -MF$(IntermediateDirectory)/svss$(DependSuffix) -MM "D:/CodeLite/icyhttpd/svss.c"

$(IntermediateDirectory)/svss$(PreprocessSuffix): svss.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/svss$(PreprocessSuffix) "D:/CodeLite/icyhttpd/svss.c"

$(IntermediateDirectory)/svconn$(ObjectSuffix): svconn.c $(IntermediateDirectory)/svconn$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/svconn.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/svconn$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/svconn$(DependSuffix): svconn.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/svconn$(ObjectSuffix) -MF$(IntermediateDirectory)/svconn$(DependSuffix) -MM "D:/CodeLite/icyhttpd/svconn.c"

$(IntermediateDirectory)/svconn$(PreprocessSuffix): svconn.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/svconn$(PreprocessSuffix) "D:/CodeLite/icyhttpd/svconn.c"

$(IntermediateDirectory)/obtype$(ObjectSuffix): obtype.c $(IntermediateDirectory)/obtype$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/obtype.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/obtype$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/obtype$(DependSuffix): obtype.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/obtype$(ObjectSuffix) -MF$(IntermediateDirectory)/obtype$(DependSuffix) -MM "D:/CodeLite/icyhttpd/obtype.c"

$(IntermediateDirectory)/obtype$(PreprocessSuffix): obtype.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/obtype$(PreprocessSuffix) "D:/CodeLite/icyhttpd/obtype.c"

$(IntermediateDirectory)/obobject$(ObjectSuffix): obobject.c $(IntermediateDirectory)/obobject$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/obobject.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/obobject$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/obobject$(DependSuffix): obobject.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/obobject$(ObjectSuffix) -MF$(IntermediateDirectory)/obobject$(DependSuffix) -MM "D:/CodeLite/icyhttpd/obobject.c"

$(IntermediateDirectory)/obobject$(PreprocessSuffix): obobject.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/obobject$(PreprocessSuffix) "D:/CodeLite/icyhttpd/obobject.c"

$(IntermediateDirectory)/obdir$(ObjectSuffix): obdir.c $(IntermediateDirectory)/obdir$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/obdir.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/obdir$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/obdir$(DependSuffix): obdir.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/obdir$(ObjectSuffix) -MF$(IntermediateDirectory)/obdir$(DependSuffix) -MM "D:/CodeLite/icyhttpd/obdir.c"

$(IntermediateDirectory)/obdir$(PreprocessSuffix): obdir.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/obdir$(PreprocessSuffix) "D:/CodeLite/icyhttpd/obdir.c"

$(IntermediateDirectory)/obinit$(ObjectSuffix): obinit.c $(IntermediateDirectory)/obinit$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/obinit.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/obinit$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/obinit$(DependSuffix): obinit.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/obinit$(ObjectSuffix) -MF$(IntermediateDirectory)/obinit$(DependSuffix) -MM "D:/CodeLite/icyhttpd/obinit.c"

$(IntermediateDirectory)/obinit$(PreprocessSuffix): obinit.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/obinit$(PreprocessSuffix) "D:/CodeLite/icyhttpd/obinit.c"

$(IntermediateDirectory)/fsinit$(ObjectSuffix): fsinit.c $(IntermediateDirectory)/fsinit$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fsinit.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fsinit$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fsinit$(DependSuffix): fsinit.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fsinit$(ObjectSuffix) -MF$(IntermediateDirectory)/fsinit$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fsinit.c"

$(IntermediateDirectory)/fsinit$(PreprocessSuffix): fsinit.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fsinit$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fsinit.c"

$(IntermediateDirectory)/fsfile$(ObjectSuffix): fsfile.c $(IntermediateDirectory)/fsfile$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fsfile.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fsfile$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fsfile$(DependSuffix): fsfile.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fsfile$(ObjectSuffix) -MF$(IntermediateDirectory)/fsfile$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fsfile.c"

$(IntermediateDirectory)/fsfile$(PreprocessSuffix): fsfile.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fsfile$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fsfile.c"

$(IntermediateDirectory)/fsvd$(ObjectSuffix): fsvd.c $(IntermediateDirectory)/fsvd$(DependSuffix)
	$(C_CompilerName) $(SourceSwitch) "D:/CodeLite/icyhttpd/fsvd.c" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/fsvd$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fsvd$(DependSuffix): fsvd.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MT$(IntermediateDirectory)/fsvd$(ObjectSuffix) -MF$(IntermediateDirectory)/fsvd$(DependSuffix) -MM "D:/CodeLite/icyhttpd/fsvd.c"

$(IntermediateDirectory)/fsvd$(PreprocessSuffix): fsvd.c
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fsvd$(PreprocessSuffix) "D:/CodeLite/icyhttpd/fsvd.c"


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
	$(RM) $(IntermediateDirectory)/fcdisp$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fcdisp$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fcdisp$(PreprocessSuffix)
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
	$(RM) $(IntermediateDirectory)/rtlmap$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/rtlmap$(DependSuffix)
	$(RM) $(IntermediateDirectory)/rtlmap$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/svinit$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/svinit$(DependSuffix)
	$(RM) $(IntermediateDirectory)/svinit$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/svsite$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/svsite$(DependSuffix)
	$(RM) $(IntermediateDirectory)/svsite$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/sventry$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/sventry$(DependSuffix)
	$(RM) $(IntermediateDirectory)/sventry$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/svlisten$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/svlisten$(DependSuffix)
	$(RM) $(IntermediateDirectory)/svlisten$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/svss$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/svss$(DependSuffix)
	$(RM) $(IntermediateDirectory)/svss$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/svconn$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/svconn$(DependSuffix)
	$(RM) $(IntermediateDirectory)/svconn$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/obtype$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/obtype$(DependSuffix)
	$(RM) $(IntermediateDirectory)/obtype$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/obobject$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/obobject$(DependSuffix)
	$(RM) $(IntermediateDirectory)/obobject$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/obdir$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/obdir$(DependSuffix)
	$(RM) $(IntermediateDirectory)/obdir$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/obinit$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/obinit$(DependSuffix)
	$(RM) $(IntermediateDirectory)/obinit$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fsinit$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fsinit$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fsinit$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fsfile$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fsfile$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fsfile$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fsvd$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fsvd$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fsvd$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe


