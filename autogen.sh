#!/bin/sh

mkdir -p build-aux || exit $?
autoreconf -vfi
