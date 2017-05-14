Source = \
	FaultManager.cpp \
	FaultMessage.cpp \
	FaultSMQueue.cpp \
	Faults.cpp \

IncludeDirs = \
	/usr/include \
	${COMPILER_VERSION} \
	${ACE_ROOT} \

# ACE libs are in 'ace' in RH3, and in 'lib' in RH4
LibraryDirs = \
        /usr/lib \
	${ACE_ROOT}/ace \
	${ACE_ROOT}/lib \
	${DB_ROOT}/lib \

Libraries = \
	platformlogger \
	platformmsgmgr\
	platformmessages \
	platformthreadmgr \
	platformopm \
	platformdatamgr \
	platformutilities \
	ACE \
	ACE_RMCast \
	pq \
	pgtypes \

Main      = FaultManager

include $(DEV_ROOT)/make/Makefile
