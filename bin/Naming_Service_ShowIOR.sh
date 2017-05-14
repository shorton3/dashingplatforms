#!/bin/bash

# Display the IORs for the Naming Service and pipe them into the catior utility for decoding.
# Since I haven't figured out how to redirect this through a pipe, send it to a /tmp/ior.txt file to read
$TAO_ROOT/utils/nslist/nslist --ior -ORBDefaultInitRef 'corbaloc:iiop:127.0.0.1:1050' 2>&1 | cut -d '<' -f2 > /tmp/platformior.out
$TAO_ROOT/utils/catior/catior -f /tmp/platformior.out
