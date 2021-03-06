# This file gets deployed as /etc/platformInit.conf in the finished
# product.

#
# NOTE: The following need to be adjusted to match the development OS
#

# Loadbuild Instructions: After sourcing this file in your development
# shell (. ./setEnv.sh), you need to change directory into each of the
# dev/src directories and issue the make command with the specified
# build tag used to identify the load (do NOT use spaces in the tag
# name):
#   make BuildTag=040105_BUILD
#
# Note that 'imake' rpm package must be installed to support builds,
#   namely, the dependency tree
#
# For Postgresql:
# UnixODBC Libraries and the ODBC Driver for Postgresql have been
#   installed in: /usr/local/lib
#   If you ever happen to want to link against installed libraries
#   in a given directory, LIBDIR, you must either use libtool, and
#   specify the full pathname of the library, or use the `-LLIBDIR'
#   flag during linking and do at least one of the following:
#    - add LIBDIR to the `LD_LIBRARY_PATH' environment variable
#      during execution
#    - add LIBDIR to the `LD_RUN_PATH' environment variable
#      during linking
#    - use the `-Wl,--rpath -Wl,LIBDIR' linker flag
#    - have your system administrator add LIBDIR to `/etc/ld.so.conf' 

export LOCAL_NEID=100000001
export IS_UNIX=true
export DEV_ROOT=~/dashingplatforms
export ACE_ROOT=/opt/ACE_wrappers
export TAO_ROOT=$ACE_ROOT/TAO
#export COMPILER_VERSION=/usr/include/c++/3.2.3/
export COMPILER_VERSION=/usr/include/c++/4.4.7/
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64/
export JACORB_HOME=/opt/JacORB/
# For postgres.org distribution, all libraries are in /usr/lib. So, we keep this placeholder
#export DB_ROOT=/usr/local/pgsql
export DB_ROOT=/usr/pgsql-9.1/
# Here we needed to download CentOS pre-built RPMs for pgadmin3, wxBase, wxGTK-2
export DBADMIN_ROOT=/usr/bin/pgadmin3
export LD_LIBRARY_PATH=$DEV_ROOT/lib:/usr/local/lib:$DB_ROOT/lib:.
export PATH=$DEV_ROOT/bin/:$DBADMIN_ROOT/bin:$DB_ROOT/bin:$JAVA_HOME/bin:$PATH
