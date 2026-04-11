#!/bin/bash
cmake --build cmake-build-docker --target format
git add -U
