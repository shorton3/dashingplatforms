Source = \
	alarms.idl \
	session.idl \
	platformConfig.idl \

IDL_CPP_OUTPUT_DIR = cppsource

# Include 'idls' in the package -- since that directory is where
# our Project Makefile originates from. NOTE that 'TARGET' is parsed/resolved in the 
# master Makefile to be the individual IDL filename.
LocalJavaIDLCompilerOptions = -pkgPrefix TARGET ems.idls -td $(DEV_ROOT)/src/

LocalIDLCompilerOptions = 

IncludeDirs = \
        /usr/include \
        ${COMPILER_VERSION} \
        ${ACE_ROOT} \
	${TAO_ROOT} \

LibraryDirs = \
        /usr/lib \
        ${ACE_ROOT}/ace \
	${ACE_ROOT}/lib \

Libraries = \
        ACE \
	TAO \

Main    = 

include $(DEV_ROOT)/make/Makefile
