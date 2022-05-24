import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *

hcalChannelInfoTable = cms.EDProducer("hcalnano::HcalChannelInfoTableProducer")
