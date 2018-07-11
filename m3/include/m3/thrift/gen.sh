#!/bin/bash

VERSION=$(thrift --version)
if [ "$VERSION" != "Thrift version 0.11.0" ]; then
    echo "Thrift version is not 0.11.0, exiting."
    exit 1
fi

thrift --gen cpp --out . m3.thrift

# Update include paths.
ls -1 *.h *.cpp | grep -v operator.cpp | xargs -I {} sed -i "" 's|#include "|#include "m3/thrift/|g' {}
