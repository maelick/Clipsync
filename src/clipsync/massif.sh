#!/bin/bash

valgrind --tool=massif --time-unit=B ./clipsync.exe $1 &> massif.log
