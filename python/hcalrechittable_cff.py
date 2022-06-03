import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *

hcalRecHitTable = cms.EDProducer("hcalnano::HcalRecHitTableProducer",
  tagHBHE  = cms.untracked.InputTag("hbhereco"),
  tagHF    = cms.untracked.InputTag("hfreco"),
  tagHO    = cms.untracked.InputTag("horeco"),
  tagPreHF = cms.untracked.InputTag(""),
  hfPreRecHitsAvailable = cms.untracked.bool(False),
)
