#!/bin/bash

cd `dirname $0`
cd ..

source ./bin/env.sh

while true; do
  node client.js client.out 2>&1
  echo "Client.js exited with code $?"
  sleep 1
done
