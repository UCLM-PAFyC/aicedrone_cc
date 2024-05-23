//##########################################################################
//#                                                                        #
//#                     AICEDRONE PLUGIN: qAicedrone                       #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 or later of the License.      #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#      COPYRIGHT: TIDOP-USAL / PAFYC-UCLM                                #
//#                                                                        #
//##########################################################################

#ifndef CC_TIDOP_TOOLS_CLASSIFICATION_TOOL_CGAL_H
#define CC_TIDOP_TOOLS_CLASSIFICATION_TOOL_CGAL_H

#if defined (_MSC_VER) && !defined (_WIN64)
#pragma warning(disable:4244) // boost::number_distance::distance()
                              // converts 64 to 32 bits integers
#endif

#include <QString>
#include <QObject>
#include <QMap>

#include <ccPointCloud.h>
#include "ccClassificationModel.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Classification.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/Point_set_3/IO.h>

#include <CGAL/Real_timer.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

class ParametersManager;

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Point_set_3<Point> Point_set;
typedef Kernel::Iso_cuboid_3 Iso_cuboid_3;

typedef Point_set::Point_map Pmap;
typedef Point_set::Property_map<int> Imap;
typedef Point_set::Property_map<unsigned char> UCmap;
typedef Point_set::Property_map<float> Fmap;
typedef Point_set::Property_map<double> Dmap;

namespace Classification = CGAL::Classification;

typedef Classification::Label_handle                                            Label_handle;
typedef Classification::Feature_handle                                          Feature_handle;
typedef Classification::Label_set                                               Label_set;
typedef Classification::Feature_set                                             Feature_set;

typedef Classification::Point_set_feature_generator<Kernel, Point_set, Pmap>    Feature_generator;

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel_FT;
typedef Kernel_FT::FT FT;
//typedef std::array<unsigned char, 3> Color;
//typedef Point_set::Property_map<Color> Color_map;
typedef Point_set::Property_map<FT> FT_map;
typedef CGAL::Parallel_if_available_tag Concurrency_tag;

//normals
//typedef Kernel::Vector_3 Vector;
//typedef std::pair<Point, Vector> PointVectorPair;
//typedef Point_set::Vector_map Vmap;

class RGBAColorsTableType;

class ccClassificationToolCGAL
{
public:
    static inline ccClassificationToolCGAL* getInstance(void )
    {
        if (m_instance==0) m_instance = new ccClassificationToolCGAL;
        return m_instance;
    };
    ccClassificationToolCGAL();
    ~ccClassificationToolCGAL();
    bool initializeFromCCPointCloud(ccPointCloud* ptrCloud,
                                    RGBAColorsTableType *ptrFormerCloudColors,
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
                         QStringList &featuresNamesFromTrainingFile,
                         QString &timeInSeconds,
                         QString& strReport,
                         QString& strError);
    bool train(ParametersManager* ptrParametersManager,
               QString trainFieldName,
               QString targetFieldName,
               QString &timeInSeconds,
               QString& strReport,
//               QVector<int>& classifiedValues,
               QString& strError);
    bool trainingClassesSeparabiltyAnalysis(ParametersManager* ptrParametersManager,
                                            QString trainFieldName,
                                            QString &timeInSeconds,
                                            QString& strReport,
                             //               QVector<int>& classifiedValues,
                                            QString& strError);
    template <typename Type>
    bool try_adding_simple_feature (const std::string& name);

private:
    void addRemainingPointSetPropertiesAsFeatures();

    static ccClassificationToolCGAL* m_instance;
    ccPointCloud* m_ptrCCPointCloud;
    RGBAColorsTableType* m_ptrFormerCloudColors;
    ccClassificationModel* m_ptrClassificationModel;
    Point_set* m_ptrPts;
    Point_set::Property_map<CGAL::IO::Color> m_color;
    Feature_generator* m_ptrFeatureGenerator;
    Feature_set m_features;
    Label_set m_labels;
    Classification::ETHZ::Random_forest_classifier* m_ptrEthzRFC;
    std::vector<std::vector<float> > m_labelProbabilities;
//    Point_set::Property_map<int> m_training;
//    Point_set::Property_map<int> m_classif;
    bool m_success;
    QMap<int,int> m_newClassByOriginalClass;
    QMap<int,QString> m_classLabelByOriginalClass;
    QMap<int, int> m_originalClassByNewClass;
    QMap<int,int> m_numberOfPointsByTrainingClass;
    std::vector<int> m_training;
    QStringList m_featuresNamesSelected;
    int m_numberOfScalesEthzRFC;
    float m_voxelSizeEthzRFC;
    bool m_useColorsEthzRFC;
};


#endif
