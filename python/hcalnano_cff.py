from PhysicsTools.NanoAOD.common_cff import Var,CandVars

nanoMetadata = cms.EDProducer("UniqueStringProducer",
    strings = cms.PSet(
        tag = cms.string("untagged"),
    )
)

nanogenSequence = cms.Sequence(
    nanoMetadata+
    hcalDigiTable+
    hcalRecHitTable

    cms.Sequence(particleLevelTask)+
    genJetTable+
    patJetPartonsNano+
    genJetFlavourAssociation+
    genJetFlavourTable+
    genJetAK8Table+
    genJetAK8FlavourAssociation+
    genJetAK8FlavourTable+
    cms.Sequence(genTauTask)+
    genTable+
    cms.Sequence(genParticleTablesTask)+
    cms.Sequence(genVertexTablesTask)+
    tautagger+
    rivetProducerHTXS+
    cms.Sequence(particleLevelTablesTask)+
    metMCTable+
    genWeightsTable
)