#!/bin/bash
start=`date +%s`
source /cvmfs/cms.cern.ch/cmsset_default.sh
mkdir -p CMSSW_12_3_0
tar -xzf HCALNano_CMSSW_12_3_0.tar.gz -C CMSSW_12_3_0
ls -lrth CMSSW_12_3_0
cd CMSSW_12_3_0/src
scramv1 b ProjectRename
eval `scramv1 runtime -sh`
cd -
ls -lrth
export X509_USER_PROXY=$(readlink -e $_CONDOR_SCRATCH_DIR/x509up)
echo $X509_USER_PROXY
ls -lrth $X509_USER_PROXY
voms-proxy-info -all
source runOnCondor.sh $1
echo "Job done."

echo "Contents of current directory:
"ls -lrth

echo "Contents of top directory:
"ls -lrth $_CONDOR_SCRATCH_DIR
end=`date +%s`
runtime=$((end-start))
echo "Run time $runtime"
