import FWCore.ParameterSet.Config as cms
from  PhysicsTools.NanoAOD.common_cff import *

# IMPORTANT: This variable has to end in "Table"! 
#     Otherwise, the NanoAODOutputModule will ignore it by default 
#     (e.g., the `keep nanoaodFlatTable_*Table_*_*"` bit in 
#     process.NanoAODEDMEventContent.outputCommands, 
#     see cmssw source code for details on this arcane syntax)
hcalEventTable= cms.EDProducer("hcalnano::HcalEventTableProducer")
