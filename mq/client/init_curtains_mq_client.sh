#!/bin/bash -e

cd `dirname $0`

INIT_FILE="$(readlink -f $0)"
PROJECT_DIR=`dirname "$INIT_FILE"`
SCRIPT="node client.js"
RUNAS=admin
#RUNAS=agornostal

PIDFILE=$PROJECT_DIR/client.pid
LOGFILE=$PROJECT_DIR/client.log

start() {
  if [ -f $PIDFILE ] && kill -0 $(cat $PIDFILE); then
    echo 'Service already running' >&2
    return 1
  fi
  echo 'Starting service...' >&2
  cd $PROJECT_DIR
  local CMD="$SCRIPT &>> \"$LOGFILE\" & echo \$! > $PIDFILE"
  su -c "$CMD" -s /bin/bash $RUNAS
  echo 'Service started' >&2
}

stop() {
  if [ ! -f "$PIDFILE" ] || ! kill -0 $(cat "$PIDFILE"); then
    echo 'Service not running' >&2
    return 1
  fi
  echo 'Stopping service…' >&2
  kill -15 $(cat "$PIDFILE") && rm -f "$PIDFILE"
  echo 'Service stopped' >&2
}

status() {
  if [ -f $PIDFILE ] && kill -0 $(cat $PIDFILE); then
    echo "Service already running. PID $(cat $PIDFILE)" >&2
  else
    echo "Service not running" >&2
  fi
}

case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  restart)
    stop; start
  status)
    status
    ;;
    ;;
  *)
    echo "Usage: $0 {start|stop|status}"
esac
