#ifndef CC_TOOLS_CLASSIFICATION_TOOL_ETHZ_RANDOM_FOREST_H
#define CC_TOOLS_CLASSIFICATION_TOOL_ETHZ_RANDOM_FOREST_H

#if defined (_MSC_VER) && !defined (_WIN64)
#pragma warning(disable:4244) // boost::number_distance::distance()
                              // converts 64 to 32 bits integers
#endif

#include <QString>
#include <QObject>
#include <QMap>
#include <QWidget>

#include <ccPointCloud.h>
#include "ccClassificationModel.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "../include/ethz_rforest/random-forest/node-gini.hpp"
#include "../include/ethz_rforest/random-forest/forest.hpp"

#include <fstream>
#include <sstream>

class ParametersManager;
class RGBAColorsTableType;

class ccToolsETHZRandomForest
{
public:
    static inline ccToolsETHZRandomForest* getInstance(void )
    {
        if (m_instance==0) m_instance = new ccToolsETHZRandomForest;
        return m_instance;
    };
    ccToolsETHZRandomForest();
    ~ccToolsETHZRandomForest();
    bool initializeFromCCPointCloud(ccPointCloud* ptrCloud,
                                    RGBAColorsTableType* ptrFormerCloudColors,
                                    ccClassificationModel* ptrClassificationModel,
                                    QString &strError);
    bool classify(ParametersManager* ptrParametersManager,
                  QString method,
                  QString targetFieldName,
                  QString &timeInSeconds,
                  QString& strReport,
                  //               QVector<int>& classifiedValues,
                  QString& strError);
    bool computeFeatures(ParametersManager* ptrParametersManager,
                         QStringList &featuresStringsFromTrainingFile,
                         QString &timeInSeconds,
                         QString& strReport,
                         QString& strError);
    bool train(ParametersManager *ptrParametersManager,
               QString trainFieldName,
               QString targetFieldName,
               QString &timeInSeconds,
               QString &strReport,
               //                                     QVector<int> &classifiedValues,
               QString &strError);
    bool trainingClassesSeparabiltyAnalysis(ParametersManager *ptrParametersManager,
                                            QString trainFieldName,
                                            QString &timeInSeconds,
                                            QString &strReport,
                                            //                                     QVector<int> &classifiedValues,
                                            QString &strError);

private:
//    void addRemainingPointSetPropertiesAsFeatures();
    //! Returns a default first guess for algorithms kernel size (one cloud)
    PointCoordinateType GetDefaultCloudKernelSize(ccGenericPointCloud* cloud, unsigned knn = 12);
//    PointCoordinateType GetDefaultCloudKernelSize(ccPointCloud* cloud, unsigned knn = 12);

    static ccToolsETHZRandomForest* m_instance;
    ccPointCloud* m_ptrCCPointCloud;
    RGBAColorsTableType* m_ptrFormerCloudColors;
    ccClassificationModel* m_ptrClassificationModel;
    bool m_success;
    QMap<int,int> m_newClassByOriginalClass;
    QMap<int,QString> m_classLabelByOriginalClass;
    QMap<int, int> m_originalClassByNewClass;
    QMap<int,int> m_numberOfPointsByOriginalTrainingClass;// newClass
//    std::vector<int> m_training;
    std::vector<std::vector<float> > m_labelProbabilities;

    typedef liblearning::RandomForest::RandomForest
    < liblearning::RandomForest::NodeGini
      < liblearning::RandomForest::AxisAlignedSplitter> > Forest;
    std::shared_ptr<Forest> m_ptrRandomForest;
    QStringList m_featuresStringsSelected;
    QMap<QString,QVector<double> > m_features;
    QVector<int> m_invalidPointsPositions;
//    QVector<QString> m_labels;
};

#endif
