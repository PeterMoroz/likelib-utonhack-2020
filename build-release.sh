#!/bin/bash
rm -rf _build_Release
mkdir _build_Release
cd _build_Release
export CC=gcc-9
export CXX=g++-9
cmake -DCMAKE_BUILD_TYPE=Release ../
