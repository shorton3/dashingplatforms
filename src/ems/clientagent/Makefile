Source = \
	alarmsI.cpp \
	platformConfigI.cpp \
	sessionI.cpp \
	ClientAgent.cpp \

LocalCppOptions =

IncludeDirs = \
        /usr/include \
        ${COMPILER_VERSION} \
        ${ACE_ROOT} \
	${TAO_ROOT} \
	${TAO_ROOT}/orbsvcs \

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
	TAO \
	TAO_CosLifeCycle \
	TAO_CosNaming \
	TAO_IORInterceptor \
	TAO_IORTable \
	TAO_Messaging \
	TAO_ObjRefTemplate \
	TAO_PortableServer \
	TAO_Svc_Utils \
	TAO_Valuetype \
	TAO_Utils \
	emsidlscppsource \
	platformutilities \
	platformdatamgr \
	platformlogger \
	platformthreadmgr \
	platformmsgmgr \
	platformmessages \
	platformopm \

Main    = ClientAgent

include $(DEV_ROOT)/make/Makefile
