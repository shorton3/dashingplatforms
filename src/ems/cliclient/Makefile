Source = \
	CliClient.cpp \
	CliMenuController.cpp \
	CliMenuAlarms.cpp \
	CliMenuBaseActions.cpp \
	CliMenuPlatformConfig.cpp \
	CliMenuStateActions.cpp \
	sessionI.cpp \

LocalCppOptions =

IncludeDirs = \
        ${COMPILER_VERSION} \
        /usr/include \
        ${ACE_ROOT} \
	${TAO_ROOT} \
	${TAO_ROOT}/orbsvcs \

LibraryDirs = \
        /usr/lib \
        ${ACE_ROOT}/ace \
	${ACE_ROOT}/lib \

Libraries = \
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
	TAO_Utils \
	TAO_Valuetype \
	emsidlscppsource \
	platformutilities \
	platformlogger \
	platformthreadmgr \
	platformmsgmgr \
	platformopm \

Main    = CliClient

include $(DEV_ROOT)/make/Makefile
