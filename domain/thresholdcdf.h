#ifndef THRESHOLDCDF_H
#define THRESHOLDCDF_H

#include "valuepairs.h"

typedef ValuePairs<double,double> DoublesPairs;

class ThresholdCDF : public DoublesPairs
{
public:
    ThresholdCDF( QString path );

    // ProjectComponent interface
public:
    QIcon getIcon();
    void save(QTextStream *txt_stream);

    // File interface
public:
    bool canHaveMetaData(){ return false; }
    QString getFileType(){ return "THRESHOLDCDF"; }
    void updateMetaDataFile(){}
    virtual bool isEditable(){ return true; }
    bool isDataFile(){ return false; }
};

#endif // THRESHOLDCDF_H
