#!/usr/bin/env bash
# Autotools bootstrap script
#
aclocal
autoheader
automake --foreign --add-missing
autoreconf --force --install
