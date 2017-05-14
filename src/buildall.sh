. ../setEnv.sh
# NOTE: this convenience script does not build IDLs (see loadbuild.txt)
# ALSO: Note that make returns non-zero upon compilation failure, so abort
#
# ISSUE: Checking the below return value does not work currently, since the
#  return value is that of the 'tee' command!! Need to fix. Could either redirect
#  without seeing, or we could use 'tail -f -pid' to see and die after each make.
#
# Build the platform package
#
cd $DEV_ROOT/src/platform
make 2>&1 | tee /tmp/platform.build
RETVAL=$?
if [ $RETVAL -ne 0 ]
then
  exit
fi
#
# Build the ems package
#
cd $DEV_ROOT/src/ems
make 2>&1 | tee -a /tmp/platform.build
RETVAL=$?
if [ $RETVAL -ne 0 ]
then
  exit
fi
#
# Build the unittest package
#
cd $DEV_ROOT/src/unittest
make 2>&1 | tee -a /tmp/platform.build
RETVAL=$?
if [ $RETVAL -ne 0 ]
then
  exit
fi
