#!/bin/bash
echo "pwd:"
pwd
cat condor_exec.exe
echo "Contents:"
ls -lrth
echo "CMSSW_BASE:"
echo $CMSSW_BASE
echo "CMSSW src contents:"
ls -lrth $CMSSW_BASE/src
echo "asdf" >> testout.txt
