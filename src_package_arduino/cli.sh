#!/usr/bin/env bash
# Simple script for switching clang between boards and different compile_commands on CLI

PARAMETER=$1
BOARD=$2

if [ "$ENV" == "default" ]; then
  BOARD="uno_r4_wifi"
fi

init() {
  echo "Initializing PlatformIO CLI..."
  if [ -z "$ENV" ]; then
    pio init --ide vim && pio run -t compiledb -e uno_r4_wifi
  else
    pio init --ide vim && pio run -t compiledb -e "$ENV"
  fi
}

compiledb() {
  echo "Generating compile commands database..."
  pio run -t compiledb
}

convert() {
  echo -n "Converting compile_commands.json to alternate format.."
  python3 ../tools/convert.py
  echo "...done"
}

usage() {
  echo "Usage: $0 [init|compiledb]"
}

if [ "$PARAMETER" == "init" ]; then
  init
elif [ "$PARAMETER" == "compiledb" ]; then
  compiledb
elif [ "$PARAMETER" == "convert" ]; then
  convert
else
  usage
  exit 1
fi
