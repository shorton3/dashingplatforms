Source = \
	OSResource.cpp \
	ResourceMonitor.cpp \
	ResourceMonitorTimerMessage.cpp \

LocalCppOptions =

IncludeDirs = \
        /usr/include \
        ${COMPILER_VERSION} \
        ${ACE_ROOT} \

LibraryDirs = \
        /usr/lib \
        ${ACE_ROOT}/ace \
	${ACE_ROOT}/lib \
	${DB_ROOT}/lib \

Libraries = \
	pq \
	pgtypes \
        ACE \
	ACE_RMCast \
	platformutilities \
	platformlogger \
	platformmsgmgr \
	platformmessages \
	platformopm \
	platformthreadmgr \
	platformdatamgr \
	platformfaultmgr \

Main    = ResourceMonitor

include $(DEV_ROOT)/make/Makefile
