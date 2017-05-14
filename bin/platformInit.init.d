#!/bin/bash
#
# platformInit     Init.d script that handles startup and shutdown of the
#                  platform Process Manager. This is the application's 
#                  entry point into running under the OS
#
# chkconfig: 3 95 5
# description: platformInit is the script used to start and stop the 
#              platform's Process Manager, which starts all of the platform
#              and application child processes.
# processname: ProcessManager
# config: /etc/platformInit.conf
# pidfile: /var/run/platformInit.pid

# Source function library.
. /etc/init.d/functions

RETVAL=0

if [ -f /etc/platformInit.conf ];then
	. /etc/platformInit.conf
else
	echo -ne "platformInit configuration file missing!"
	exit 0
fi

PROCESS_NAME=ProcessManager
SERVICE_NAME=platformInit
# Since we handle our own daemonization, no need to pass "-b" option to ProcessManager
PROCESS_OPTIONS="-d -f ${DEV_ROOT}/bin/ProcessManager.conf"
# Need to provide a location to redirect ProcessManager's stdout before logger takes over
PROCESSMGR_STDOUT="/tmp/platformprocessmgr.out"

start() {
	echo -ne "Starting $SERVICE_NAME: "
	if [ `ps -e | grep -i "${PROCESS_NAME}" | wc -l ` -gt 0 ]
	then
	   echo -ne "${PROCESS_NAME} is Already Running\n"
	   exit 0
	fi
	cd $"${DEV_ROOT}/bin"
        # Don't need to prefix this command with 'daemon' from /etc/init.d/functions since
        # ProcessManager handles its own daemonization (redirect pre-redirected stdout to a file)
	echo -ne "\n\nStarting ${SERVICE_NAME} (`date`)\n\n" >> ${PROCESSMGR_STDOUT} 2>> ${PROCESSMGR_STDOUT}
	${DEV_ROOT}/bin/${PROCESS_NAME} ${PROCESS_OPTIONS} >> ${PROCESSMGR_STDOUT} 2>> ${PROCESSMGR_STDOUT}
	RETVAL=$?
	# Write the lock and pid files if the startup was successful
	if [ $RETVAL -eq 0 ]
	then
	   echo_success && touch /var/lock/subsys/${SERVICE_NAME} && echo `pidofproc "${PROCESS_NAME}"` > /var/run/${SERVICE_NAME}.pid
	fi
	return ${RETVAL}
}


stop() {
	echo -ne "Stopping ${SERVICE_NAME}: "
        # Send SIGTERM (15) to the Process Manager
	echo -ne "\n\nStopping ${SERVICE_NAME} (`date`)\n\n" >> ${PROCESSMGR_STDOUT} 2>> ${PROCESSMGR_STDOUT}
	killproc ${SERVICE_NAME} "-15"
	RETVAL=$?
        # Erase the lock file (leave the pid file) if the shutdown was successful
	if [ $RETVAL -eq 0 ]
	then
	   rm -f /var/lock/subsys/${SERVICE_NAME}
	fi
	return ${RETVAL}
}


case "$1" in
  start)
	start
	;;
  stop)
	stop
	;;
  status)
	status ${PROCESS_NAME}
	;;
  restart|reload)
	stop
        sleep 5
	start
	RETVAL=$?
	;;
  *)
	echo $"Usage: $0 {start|stop|status|reload|restart}"
	exit 1
esac

exit $?
