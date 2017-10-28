#!/bin/sh
#
# Put this file in /usr/local/etc/rc.d/curtains-client.sh

admin_home=/var/services/homes/admin

case "$1" in
stop)
  echo "Stop curtains-client..."
  ;;
start)
  su admin -c "admin_home/ArduinoCurtainsControl/mq/client/bin/start.sh"
  ;;
status)
  ps | grep client.js | grep -q -v grep
  return $?
  ;;
*)
  echo "usage: $0 { start | stop | status}" >&2
        exit 1
        ;;
esac
