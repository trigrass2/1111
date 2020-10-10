#==================================================
#	工程：liblinux_os.so库
#	编译：依赖-pthread -lpaho-mqtt3c 动态库
#	连接：生成liblinux_os.so 动态库
#==================================================

PREVDIR ?= $(shell cd ..;cd ..;cd ..;cd ..;pwd)
include $(PREVDIR)/make/build.make
include $(PREVDIR)/make/*.make
ALLFILE = 1

SRCPATH = ../../../../source/src/system

ifdef ALLFILE
	SOURCE  :=	$(wildcard $(SRCPATH)/os/*.c)  		

	OBJS    := $(patsubst %.c,%.o,$(SOURCE)) 
else
	OBJS 	:= $(SRCPATH)/clock.o  $(SRCPATH)/os.o $(SRCPATH)/os_linux.o $(SRCPATH)/os_linux.o 
endif

SHARE_OBJ := $(notdir $(OBJS))

CCOMPILE = $(CC)  $(DEFINED_OPTION) -lstdc++ 
CPPCOMPILE = $(CPP)  $(DEFINED_OPTION)
COMPILEOPTION = -c  -DREENTRANT

LIBSO = liblinux_os.so

.PHONY : all 

all:$(LIBSO) install

$(LIBSO): $(OBJS)
	$(CC) -Wall -shared -o $@ $^
	@echo '------------------- liblinux_os  Module done!-------------------'

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

