from PhysicsTools.NanoAOD.common_cff import Var,CandVars

nanoMetadata = cms.EDProducer("UniqueStringProducer",
    strings = cms.PSet(
        tag = cms.string("untagged"),
    )
)
