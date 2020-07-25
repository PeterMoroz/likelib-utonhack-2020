#!/bin/bash
rm -rf _build_Debug
mkdir _build_Debug
cd _build_Debug
export CC=gcc-9
export CXX=g++-9
cmake -DCMAKE_BUILD_TYPE=Debug ../
