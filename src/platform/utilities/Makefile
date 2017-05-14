Source = \
	Conversions.cpp \
	DebugUtils.cpp \
	SharedMemoryManager.cpp \
	SystemInfo.cpp \
	UnboundedSMQueue.cpp \

IncludeDirs = \
        /usr/include \
        ${COMPILER_VERSION} \
        ${ACE_ROOT} \
                                                                                                                        
LibraryDirs = \
        /usr/lib \
        ${ACE_ROOT}/ace \
                                                                                                                        
# NOTE: The 'utilities' package should NOT depend on the LOGGER as the logger uses this for shared memory
Libraries = \
        ACE \

Main      = 

include $(DEV_ROOT)/make/Makefile
