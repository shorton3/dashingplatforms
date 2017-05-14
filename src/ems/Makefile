# Cpp Section
Packages = \
	ems/idls \
	ems/idls/cppsource \
	ems/clientagent \
	ems/cliclient \

IncludeDirs = \
	/usr/include \
	${COMPILER_VERSION} \
	${ACE_ROOT} \
	
LibraryDirs = \
	/usr/lib \
        ${ACE_ROOT}/ace \

Libraries = \
	ACE \

# Java Section
JavaPackages = \
	ems/idls/platformConfig \
        ems/idls/session \
	ems/guiclient \
                                                                                                        
JavaLibraries = \
        emsidl.jar \
	${JACORB_HOME}/lib/jacorb.jar \
                                                                                                        
JavaMainClass = ems.guiclient.GuiClient
                                                                                                        
RunParameters =
                                                                                                        
JavadocWindowTitle = 'EMS IDL Interface Library and Client'
JavadocDocTitle    = 'EMS Subsystem'
JavadocHeader      = 'EMS Subsystem'
JavadocFooter      = '<font size="-1">Copyright &copy; 2014</font>'

include $(DEV_ROOT)/make/Makefile
