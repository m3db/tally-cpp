#!/bin/bash

# Thrift version should be 0.11.0.
thrift --gen cpp --out . m3.thrift

# Update include paths.
ls -1 *.h *.cpp | grep -v operator.cpp | xargs -I {} sed -i "" 's|#include "|#include "m3/thrift/|g' {}
