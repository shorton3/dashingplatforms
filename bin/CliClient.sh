#!/bin/bash

# -h (use IP addresses instead of hostnames in IORs)
# -d (enable Orb debug logs)
# -l <debug level> (Max is 10)
# -f <log filename> (Default is stdout)
# -e <Endpoint> (Use the endpoint URL for which interfaces to listen for connections on)
# -i <Initial Reference> (URL for initial resolution of the Name Service)
# -r <Default Init Ref> (URL for default initial resolution of the Name Service)

echo "Starting the CliClient..."

./CliClient -h -i "NameService=corbaloc:iiop:127.0.0.1:1050/NameService" -e "iiop://127.0.0.1:1090" -d -l 10 -f "/tmp/platformcliclient.out"
