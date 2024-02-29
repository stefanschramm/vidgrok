#!/bin/sh

set -e

if [ ! -d build ] ; then
  meson setup build
else
  echo "Using existing build directory."
fi
meson compile -C build

