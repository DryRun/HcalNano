#ifndef QIE11DigiTable_h
#define QIE11DigiTable_h

#include <vector>
#include <map>

#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalGenericDetId.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "CondFormats/HcalObjects/interface/HcalElectronicsMap.h"
#include "CondFormats/HcalObjects/interface/HcalChannelQuality.h"
#include "DQM/HcalCommon/interface/Utilities.h"

/*
class QIE11Digi {
public:
    HcalDetId did;

    int* ieta_;
    int* iphi_;
    int* subdet_;
    int* depth_;
    int* rawID_;
    bool* linkError_;
    bool* capidError_;
    int* flags_;
    int* soi_;
    bool* valid_;

    int* nTS_;
    std::vector<int>* adc_;
    std::vector<double>* fc_;
    std::vector<double>* pedestalfc_;
    std::vector<int>* tdc_;
    std::vector<int>* capID_;

    inline QIE11Digi(QIE11DigiTable &table, unsigned int index) {
        this.nTS_ = table.nTS_;
        this.did_ = table.dids_[index];

        this.ieta_ = &(table.ietas_[index]);
        this.iphi_ = &(table.iphis_[index]);
        this.subdet_ = &(table.subdets_[index]);
        this.depth_ = &(table.depths_[index]);
        this.rawID_ = &(table.rawIds_[index]);
        this.linkError_ = &(table.linkErrors_[index]);
        this.capidError_ = &(table.capidErrors_[index]);
        this.flags_ = &(table.flags_[index]);
        this.soi_ = &(table.sois_[index]);
        this.valid_ = &(table.valids_[index]);

        this.adc_ = &(table.adcs_[index]);
        this.fc_ = &(table.fcs_[index]);
        this.pedestalfc_ = &(table.pedestalfcs_[index]);
        this.tdc_ = &(table.tdcs_[index]);
        this.capID_ = &(table.capIDs_[index]);

    }

    ~QIE11Digi();

    inline void reset() {
        this.valid_ = false;
        this.ieta_ = 0;
        this.iphi_ = 0;
        this.subdet_ = 0;
        this.depth_ = 0;
        this.rawID_ = 0;
        this.linkError_ = false;
        this.capidError_ = false
        this.flags_ = 0;
        this.soi_ = 0;
        this.adc_.clear();
        this.fc_.clear();
        this.pedestalfc_.clear();
        this.tdc_.clear();
        this.capID_.clear();
    }
}
*/

class QIE11DigiTable {
public:
    std::vector<HcalDetId> dids_;
    std::map<HcalDetId, unsigned int> did_indexmap_; // Use std::map for efficient lookup, rather than std::find

    std::vector<int> ietas_;
    std::vector<int> iphis_;
    std::vector<int> subdets_;
    std::vector<int> depths_;
    std::vector<int> rawIds_;
    std::vector<bool> linkErrors_;
    std::vector<bool> capidErrors_;
    std::vector<int> flags_;
    std::vector<int> sois_;
    std::vector<bool> valids_;

    unsigned int nTS_;
    std::vector<std::vector<int>> adcs_;
    std::vector<std::vector<float>> fcs_;
    std::vector<std::vector<float>> pedestalfcs_;
    std::vector<std::vector<int>> tdcs_;
    std::vector<std::vector<int>> capids_;

    QIE11DigiTable(std::vector<HcalDetId>& _dids, unsigned int _nTS);
    void add(const QIE11DataFrame* digi, const edm::ESHandle<HcalDbService>& dbService);
    void reset();

};

typedef QIE11DigiTable HBDigiTable;
typedef QIE11DigiTable HEDigiTable;

#endif
