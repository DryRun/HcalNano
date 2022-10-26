### Disclaimer
I'm merging HcalNano into github.com/cms-sw/cmssw, so this repository will soon be obsolete. 

# HcalNano
An EDM analyer for analyzing HCAL digis and RecHits. This supersedes https://github.com/HCALPFG/HcalTupleMaker, replacing the custom output module with CMSSW NanoAODOutputModule. 

Setup instructions:
```
scram project -n "CMSSW_12_3_0_HcalNano" CMSSW_12_3_0
cd CMSSW_12_3_0_HcalNano/src
cmsenv
git cms-init

## If you want ZSTD or LZ4 compression
# git cms-rebase-topic DryRun:CMSSW_12_3_0-nanocompression

git clone git@github.com:HCALPFG/HcalNano.git HCALPFG/HcalNano
git clone git@github.com:HCALPFG/HcalNanoDF.git DataFormats/HcalNano

## Optional, if you want to study or modify NanoAOD in some way:
git cms-addpkg PhysicsTools/NanoAOD
git cms-addpkg DataFormats/NanoAOD

scram b -j8
```

Example:
```
cd $CMSSW_BASE/src/HCALPFG/HcalNano/test
cmsRun test_cfg.py inputFiles=root://eoscms.cern.ch//store/group/dpg_hcal/comm_hcal/Splashes2022/splashes_350968_FEVT.root outputFile=hcalnano_test.root nThreads=4 compressionAlgorithm=ZLIB compressionLevel=5
```

