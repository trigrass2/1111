#==================================================
#	工程：libsystem.so库
#	编译：依赖-pthread -lpaho-mqtt3c 动态库
#	连接：生成libsystem.so动态库
#==================================================

PREVDIR ?= $(shell cd ..;cd ..;cd ..;cd ..;pwd)
include $(PREVDIR)/make/build.make
include $(PREVDIR)/make/*.make
ALLFILE = 1

SRCPATH = ../../../../source/src/system

ifdef ALLFILE
	SOURCE  :=	$(wildcard $(SRCPATH)/comm/*.c)  $(wildcard $(SRCPATH)/comm/lan/*.c)  $(wildcard $(SRCPATH)/comm/serial/*.c)  $(wildcard $(SRCPATH)/comm/serial/uart/*.c) \
				$(wildcard $(SRCPATH)/common/*.c)   $(wildcard $(SRCPATH)/drv/*.c)  $(wildcard $(SRCPATH)/bsp/*.c)	 

	OBJS    := $(patsubst %.c,%.o,$(SOURCE))  $(SRCPATH)/myio/yx/di.o  $(SRCPATH)/myio/yc/ai.o  $(SRCPATH)/myio/yk/do.o
else
	OBJS 	:= $(SRCPATH)/clock.o  $(SRCPATH)/os.o $(SRCPATH)/os_linux.o $(SRCPATH)/os_linux.o 
endif

SHARE_OBJ := $(notdir $(OBJS))

CCOMPILE = $(CC)  $(DEFINED_OPTION) -lstdc++ 
CPPCOMPILE = $(CPP)  $(DEFINED_OPTION)
COMPILEOPTION = -c  -DREENTRANT

LIBSO = libsystem.so

.PHONY : all 

all:$(LIBSO) install

$(LIBSO): $(OBJS)
	$(CC) -Wall -shared -o $@ $^ -L. $(SHAREDLIB)
	@echo '------------------- libsystem.so  Module done!-------------------'

.PRECIOUS:%.cpp %.c %.C
.SUFFIXES:
.SUFFIXES:  .c .o .cpp .ecpp .pc .ec .C .cc .cxx

.cpp.o:
	$(CPPCOMPILE) -c  -o  $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp
	
.cc.o:
	$(CCOMPILE) -c -o  $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cx

.cxx.o:
	$(CPPCOMPILE) -c -o  $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cxx

.c.o:
	$(CCOMPILE) -c -o  $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.c

.C.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.C	

.ecpp.C:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ecpp 
	
.ec.c:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ec
	
.pc.cpp:
	$(PROC)  CPP_SUFFIX=cpp $(PROC_OPTION)  $*.pc

	
clean: 
	-rm -f $(OBJS) 
	-rm -f $(LIBSO)

	
install:
	cp $(LIBSO) $(INSTALL_LIB_PATH)/
ifdef CLEAN_OBJ_FILE 
	-rm -f $(OBJS) 	
	-rm -f $(LIBSO)
endif