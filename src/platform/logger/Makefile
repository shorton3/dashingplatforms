Source = \
	Logger.cpp \
	LoggerCommon.cpp \
	LoggerSMConfig.cpp \
	LoggerSMConfigValues.cpp \
	LoggerSMQueue.cpp \
	LogMessage.cpp \
	LogProcessor.cpp \

IncludeDirs = \
	/usr/include \
	${COMPILER_VERSION} \
	${ACE_ROOT} \

LibraryDirs = \
        /usr/lib \
	${ACE_ROOT}/ace \

Libraries = \
	ACE \
	platformutilities \

Main      = Logger

include $(DEV_ROOT)/make/Makefile
