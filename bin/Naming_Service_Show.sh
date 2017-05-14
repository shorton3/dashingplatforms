#!/bin/bash

# Display the IORs for the Naming Service and Registered Interface Objects
$TAO_ROOT/utils/nslist/nslist --ior -ORBDefaultInitRef 'corbaloc:iiop:127.0.0.1:1050'

# Display the protocols and Endpoints
$TAO_ROOT/utils/nslist/nslist -ORBDefaultInitRef 'corbaloc:iiop:127.0.0.1:1050'
