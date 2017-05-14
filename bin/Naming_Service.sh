#!/bin/bash

# Start the TAO Naming_Service using port 1050
# NOTE: Specifying '-p' to cause the Naming Service to persist IORs causes TAO to memory map shared memory, and on Linux, the base address for this is 0x80000000
# NOTE: Add the -ORBDaemon option later to cause the Naming_Service to daemonize

echo "Attempting to call the TAO Naming_Service and install it into the background..."

$TAO_ROOT/orbsvcs/Naming_Service/Naming_Service -o /tmp/Naming_Service.ior -f /tmp/Naming_Service.persistDB -p /var/run/Naming_Service -ORBListenEndpoints 'iiop://127.0.0.1:1050' -ORBDottedDecimalAddresses 1 &

# Needed for later migration to an init.d script:
#touch /var/lock/subsys/Naming_Service
