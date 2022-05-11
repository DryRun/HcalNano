import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *

hcalDigiTable = cms.EDProducer("HcalDigiTableProducer",
  tagQIE11  = cms.untracked.InputTag("hcalDigis"),
  tagQIE10  = cms.untracked.InputTag("hcalDigis"),
  #taguMNio  = cms.untracked.InputTag("hcalDigis"),
  #StoreLaser = cms.untracked.bool(False),
  #chargeSkim = cms.untracked.double(0)
)
