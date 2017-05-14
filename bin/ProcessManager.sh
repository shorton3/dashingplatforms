#!/bin/bash

# NOTE: Process Manager needs to be started using:
./ProcessManager -f ProcessManager.conf

# If the ProcessManager.conf is kept as its original name 'svc.conf', then some of the other applications, such as the TAO Naming_Service will attempt to read it.
