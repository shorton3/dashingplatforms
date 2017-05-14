Source = \
	ConnectionSet.cpp \
	DataManager.cpp \
	DbConnection.cpp \
	DbConnectionHandle.cpp \
	PgConnection.cpp \

LocalCppOptions =

IncludeDirs = \
        /usr/include \
        ${COMPILER_VERSION} \
        ${ACE_ROOT} \
	${DB_ROOT}/include \

LibraryDirs = \
        /usr/lib \
        ${ACE_ROOT}/ace \
	${ACE_ROOT}/lib \
	${DB_ROOT}/lib \

Libraries = \
	pq \
        ACE \
	ACE_RMCast \
	platformlogger \
	platformopm \

Main    = 

include $(DEV_ROOT)/make/Makefile
