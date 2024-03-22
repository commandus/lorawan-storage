#!/bin/sh
# Create PID file
touch /var/run/lorawan-storage.pid
# Copy systemd service file
cp lorawan-storage.service /etc/systemd/system/
exit 0
