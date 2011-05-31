#!/bin/bash

valgrind ./clipsync.exe $1 &> memcheck.log
