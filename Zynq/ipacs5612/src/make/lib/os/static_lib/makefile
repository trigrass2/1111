#==================================================
#	工程：liblinux_os.a库
#	编译：依赖-pthread -lpaho-mqtt3c 动态库，在链接过程
#	连接：生成liblinux_os.a 静态库
#==================================================
PREVDIR ?= $(shell cd ..;cd ..;cd ..;cd ..;pwd)
include $(PREVDIR)/make/build.make
include $(PREVDIR)/make/*.make
ALLFILE = 1

C_COMPILE = $(CC) $(DEFINED_OPTION) -lstdc++ 
CPP_COMPILE = $(CPP) $(DEFINED_OPTION)
COMPILE_OPTION = -c  -DREENTRANT

SRCPATH = ../../../../source/src/system
TARGET_LIB = liblinux_os.a

ifdef ALLFILE
	SOURCE  := $(wildcard $(SRCPATH)/os/*.c)  		

	OBJS    := $(patsubst %.c,%.o,$(SOURCE)) 
else
	OBJS 	:= $(SRCPATH)/clock.o  $(SRCPATH)/os.o $(SRCPATH)/os_linux.o $(SRCPATH)/os_linux.o 
endif

.PHONY : all 

all:$(TARGET_LIB) install

$(TARGET_LIB): $(OBJS)
	$(AR) crs $@ $^
	@echo '------------------- liblinux_os.a  Module done!-------------------'

.PRECIOUS:%.cpp %.c %.C
.SUFFIXES:
.SUFFIXES:  .c .o .cpp .ecpp .pc .ec .C .cc .cxx

.cpp.o:
	$(CPP_COMPILE) -c  -o  $*.o $(COMPILE_OPTION) $(INCLUDEDIR) $*.cpp
	
.cc.o:
	$(C_COMPILE) -c -o  $*.o $(COMPILE_OPTION) $(INCLUDEDIR) $*.cx

.cxx.o:
	$(CPP_COMPILE) -c -o  $*.o $(COMPILE_OPTION) $(INCLUDEDIR) $*.cxx

.c.o:
	$(C_COMPILE) -c -o  $*.o $(COMPILE_OPTION) $(INCLUDEDIR) $*.c

.C.o:
	$(CPP_COMPILE) -c -o $*.o $(COMPILE_OPTION) $(INCLUDEDIR) $*.C	

.ecpp.C:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR)) $*.ecpp 
	
.ec.c:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ec
	
.pc.cpp:
	$(PROC)  CPP_SUFFIX=cpp $(PROC_OPTION) $*.pc

	
clean: 
	-rm -f $(OBJS) 
	-rm -f $(TARGET_LIB)

	
install:
	cp $(TARGET_LIB) $(INSTALL_LIB_PATH)/
ifdef CLEAN_OBJ_FILE 
	-rm -f $(OBJS) 
	-rm -f $(TARGET_LIB)
endif

