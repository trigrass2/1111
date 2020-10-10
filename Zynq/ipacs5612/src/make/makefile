#===========================================================================
#工程：	编译所有项目
# 		'IPACS_5612T HUAWEI 版本 Makefile文件使用说明'
# 		'clean  - 清除所有临时文件'
# 		'all    - 缺省选项，完全编译，文件保存在exe/目录下'
# 		'       - 此目录结构与终端下的/mnt目录结构完全相同'
# 		'       - 使用FTP方式安装时，根据此目录结构上传所有文件，并赋予执行权限即可运行'
# 		'lib_a_	- 静态库编译，库保存在/exe/lib目录下'
#		'lib_so_- 动态库编译，库保存在/exe/lib目录下'
#		'app_   - 可执行程序编译，可执行程序保存在/exe/bin目录下'
# 		'help   - 帮助说明，就是当前所看到的内容'
#===========================================================================
PREVDIR ?= $(shell pwd)
include $(PREVDIR)/build.make

#所有需要编译的平台层模块
MAKE_MODS= lib_a_ app_ 
all: $(MAKE_MODS) 
lib_so_:linux_so_   system_so_
lib_a_:linux_a_   system_a_

#编译库
linux_a_:
	cd lib/os/static_lib && $(MAKE) 
linux_so_:
	cd lib/os/dynamic_lib && $(MAKE) 

system_a_:
	cd lib/system/static_lib && $(MAKE)
system_so_:
	cd lib/system/dynamic_lib && $(MAKE)

#编译可执行程序
app_:
	cd exe && $(MAKE) 
	
install:

	
clean: clean_linux_  clean_system_  clean_app_

clean_linux_:  
	cd lib/os/static_lib && $(MAKE) clean
	cd lib/os/dynamic_lib && $(MAKE) clean

clean_system_:
	cd lib/system/static_lib && $(MAKE) clean
	cd lib/system/dynamic_lib && $(MAKE) clean

clean_app_:
	cd exe && $(MAKE) clean
	
help:
	@echo 'IPACS_5612T HUAWEI Edition  Instructions for the Use of Documents Makefile '
	@echo ' '
	@echo 'clean   - Clear all temporary files '
	@echo ' '	
	@echo 'all     - Default options, full compiled, the file saved in exe/ directory'
	@echo '        - This directory structure is identical to the /mnt directory structure in the terminal '
	@echo '        - When installing in FTP mode, upload all files according to this directory '
	@echo '          structure and grant execution privileges to run.'
	@echo ''
	@echo 'lib_a_  - Static library compilation, library stored in /exe/lib directory '
	@echo ' '
	@echo 'lib_so_ - Dynamic library compilation, library stored in /exe/lib directory '
	@echo ' '
	@echo 'app_    - Executable program compilation, executable program saved in /exe/bin directory'
	@echo ' '
	@echo 'help    - Help note, is what you see at the moment 	'
	
