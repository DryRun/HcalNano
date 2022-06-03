#!/bin/bash
ls -lrth
pwd
mkdir work
mv test_cfg.py work
cd work
INPUTFILES=(/store/group/dpg_hcal/comm_hcal/Splashes2022/splashes_350968_FEVT.root)
cmsRun test_cfg.py inputFiles=${INPUTFILES[$1]} outputFile=hcalnano.subjob$1.root None 2>&1
xrdcp -p -f *root root://eoscms.cern.ch///store/user/dryu/HCAL/PFG/HcalNano/ 
ls -lrth
pwd
ls -lrth $_CONDOR_SCRATCH_DIR
