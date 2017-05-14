Source = \
	ProcessController.cpp \
	ProcessDeathHandler.cpp \
	ProcessManager.cpp \

LocalCppOptions = -DProcessController_BUILD_DLL

IncludeDirs = \
        /usr/include \
        ${COMPILER_VERSION} \
        ${ACE_ROOT} \

LibraryDirs = \
        /usr/lib \
        ${ACE_ROOT}/ace \
	${ACE_ROOT}/lib \

Libraries = \
        ACE \
	ACE_RMCast \
	platformutilities \
	platformlogger \
	platformopm \
	platformthreadmgr \
	platformmsgmgr \

Main    = ProcessManager

include $(DEV_ROOT)/make/Makefile
