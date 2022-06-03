#!/bin/bash
start=`date +%s`
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
