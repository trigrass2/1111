#==================================================
#	工程：jc进程
#	编译：依赖liblinux_os.a,libsystem.a 静态库
#	连接：生成jc进程可执行文件
#==================================================
PREVDIR ?= $(shell cd ..;cd ..;pwd)
include $(PREVDIR)/make/build.make
include $(PREVDIR)/make/*.make
ALLFILE = 1
DIFFERENCE ?=   $(ARRER_JS_DEF)

SRCPATH = ../../source/src/system/myio

SRCPATH1 = ../../source/src/protocol

SRCPATH2 = ../../source/src/protocols

SRCPATH3 = ../../source/src/bmlinux

SRCPATH4 = ../../source/src/product/mmi

SRCPATH5 = ../../source/src/product/protect

ifdef ALLFILE
	SOURCE  := 	$(wildcard $(SRCPATH)/*.c) \
				$(wildcard $(SRCPATH)/drv/*.c) \
				$(wildcard $(SRCPATH3)/*.c) \
				$(wildcard $(SRCPATH4)/*.c) \
				$(wildcard $(SRCPATH5)/*.c)
				
	OBJS    := 	$(patsubst %.c,%.o,$(SOURCE)) $(SRCPATH1)/maint/maint.o  $(SRCPATH1)/public/pbase.o $(SRCPATH1)/public/pslave.o $(SRCPATH1)/public/pmaster.o $(SRCPATH1)/public/pcol.o \
				$(SRCPATH1)/commtest/CTPri.o $(SRCPATH1)/commtest/CTSec.o  $(SRCPATH1)/gb104/gb104s.o  ../../source/src/system/myio/yx/yx.o  ../../source/src/system/myio/yc/yc.o \
				../../source/src/system/myio/yc/Extyc.o ../../source/src/system/myio/yc/drv/ad7913.o    ../../source/src/system/myio/yk/yk.o  ../../source/src/product/pb/pb.o  ../../source/src/main.o  $(SRCPATH4)/extmmi/extdisp.o  \
				$(SRCPATH2)/gb101/gb101m.o	$(SRCPATH2)/gb101/gb101s.o	$(SRCPATH1)/gb104/gb104m.o	$(SRCPATH2)/modbusm/modbusm.o 
				
else
	OBJS 	:= 
endif

CCOMPILE = $(CC)  $(DIFFERENCE) -lstdc++  -Wall -O2
CPPCOMPILE = $(CPP)  $(DIFFERENCE)
COMPILEOPTION = -c  -DREENTRANT
LINK = $(CPP)
LINKOPTION = -Wall -O2 -o jc

LIBDIRS = -L $(INSTALL_LIB_PATH)
OUTPUT = jc
SHAREDLIB = -pthread -llinux_os -lrt -lsystem 
APPENDLIB = 

.PHONY : all 

all:$(OUTPUT) install


.PRECIOUS:%.cpp %.c %.C
.SUFFIXES:
.SUFFIXES:  .c .o .cpp .ecpp .pc .ec .C .cc .cxx

.cpp.o:
	$(CPPCOMPILE) -c  -o  $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp
	
.cc.o:
	$(CCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cx

.cxx.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cxx

.c.o:
	$(CCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.c

.C.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.C	

.ecpp.C:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ecpp 
	
.ec.c:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ec
	
.pc.cpp:
	$(PROC)  CPP_SUFFIX=cpp $(PROC_OPTION)  $*.pc

$(OUTPUT): $(OBJS) $(APPENDLIB)
	$(LINK) $(LINKOPTION) $(OBJS)   $(LIBDIRS) $(SHAREDLIB) $(APPENDLIB) 
	@echo '------------------- jc  Module done!-------------------'	
clean: 
	-rm -f $(OBJS)
	-rm -f $(OUTPUT)

	
install:
	cp $(OUTPUT) $(INSTALL_BIN_PATH)/
ifdef CLEAN_OBJ_FILE 
	-rm -f $(OBJS)
	-rm -f $(OUTPUT)
endif