Source = \
	MessageTest.cpp \
	MessageTest1Message.cpp \
	MessageTest2Message.cpp \
	MessageTestTimerMessage.cpp \

IncludeDirs = \
	/usr/include \
	${COMPILER_VERSION} \
	${ACE_ROOT} \

LibraryDirs = \
        /usr/lib \
	${ACE_ROOT}/ace \
	${ACE_ROOT}/lib \

Libraries = \
	platformutilities \
	platformopm \
	platformlogger \
	platformthreadmgr \
	platformmsgmgr \
	ACE \
	ACE_RMCast \

Main      = MessageTest1

include $(DEV_ROOT)/make/Makefile
