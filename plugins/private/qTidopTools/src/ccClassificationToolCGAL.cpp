//##########################################################################
//#                                                                        #
//#                     TIDOPTOOLS PLUGIN: qTidopTools                     #
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

#include <QFIleInfo>
#include <QMessageBox>

#include "../include/ccClassificationToolCGAL.h"
#include "../include/ccToolsRandomForestClassificationDefinitions.h""
#include "libParameters/ParametersManager.h"
#include "libParameters/Parameter.h"

#include <ccAdvancedTypes.h>

//#include <CGAL/compute_average_spacing.h>
//#include <CGAL/pca_estimate_normals.h>
//#include <CGAL/jet_estimate_normals.h>

ccClassificationToolCGAL* ccClassificationToolCGAL::m_instance = 0;

ccClassificationToolCGAL::ccClassificationToolCGAL()
{
    m_ptrPts=NULL;
    m_ptrFeatureGenerator=NULL;
    m_ptrEthzRFC=NULL;
    m_ptrCCPointCloud=NULL;
    m_ptrFormerCloudColors=NULL;
    m_ptrClassificationModel=NULL;
}

ccClassificationToolCGAL::~ccClassificationToolCGAL()
{
    if(m_ptrEthzRFC!=NULL)
    {
        delete(m_ptrEthzRFC);
        m_ptrEthzRFC = NULL;
    }
    if(m_ptrFeatureGenerator!=NULL)
    {
        delete(m_ptrFeatureGenerator);
        m_ptrFeatureGenerator = NULL;
    }
}

void ccClassificationToolCGAL::addRemainingPointSetPropertiesAsFeatures()
{
    /*
  const std::vector<std::string>& prop = m_points->point_set()->base().properties();

  for (std::size_t i = 0; i < prop.size(); ++ i)
  {
    if (prop[i] == "index" ||
        prop[i] == "point" ||
        prop[i] == "normal" ||
        prop[i] == "echo" ||
        prop[i] == "number_of_returns" ||
        prop[i] == "training" ||
        prop[i] == "label" ||
        prop[i] == "classification" ||
        prop[i] == "scan_direction_flag" ||
        prop[i] == "real_color" ||
        prop[i] == "shape" ||
        prop[i] == "red" || prop[i] == "green" || prop[i] == "blue" ||
        prop[i] == "r" || prop[i] == "g" || prop[i] == "b")
      continue;

    if (try_adding_simple_feature<boost::int8_t>(prop[i]))
      continue;
    if (try_adding_simple_feature<boost::uint8_t>(prop[i]))
      continue;
    if (try_adding_simple_feature<boost::int16_t>(prop[i]))
      continue;
    if (try_adding_simple_feature<boost::uint16_t>(prop[i]))
      continue;
    if (try_adding_simple_feature<boost::int32_t>(prop[i]))
      continue;
    if (try_adding_simple_feature<boost::uint32_t>(prop[i]))
      continue;
    if (try_adding_simple_feature<float>(prop[i]))
      continue;
    if (try_adding_simple_feature<double>(prop[i]))
      continue;
  }
  */
}

bool ccClassificationToolCGAL::initializeFromCCPointCloud(ccPointCloud *ptrCloud,
                                                          RGBAColorsTableType *ptrFormerCloudColors,
                                                          ccClassificationModel *ptrClassificationModel,
                                                          QString& strError)
{
    QString functionName="ccClassificationToolCGAL::initializeFromCCPointCloud";
    if(m_ptrCCPointCloud!=NULL)
    {
        int cloudId=ptrCloud->getUniqueID();
        int currentId=m_ptrCCPointCloud->getUniqueID();
        if(cloudId==currentId)
        {
            return(true);
        }
    }
//    int cloudIdForDisplay=ptrCloud->getUniqueIDForDisplay();
    strError.clear();
    unsigned pointCount = ptrCloud->size();
    if(m_ptrPts!=NULL)
    {
        delete m_ptrPts;
        m_ptrPts=NULL;
    }
    if(m_ptrFeatureGenerator!=NULL)
    {
        delete m_ptrFeatureGenerator;
        m_ptrFeatureGenerator=NULL;
    }
    m_features.clear();
    m_labels.clear();

    if(m_ptrEthzRFC!=NULL)
    {
        delete m_ptrEthzRFC;
        m_ptrEthzRFC=NULL;
    }
    m_labelProbabilities.clear();
    m_success=false;
    m_newClassByOriginalClass.clear();
    m_classLabelByOriginalClass.clear();
    m_originalClassByNewClass.clear();
    m_numberOfPointsByTrainingClass.clear();
    m_training.clear();

    m_ptrPts=new Point_set();
    try
    {
        m_ptrPts->reserve(pointCount);
    }
    catch (const std::bad_alloc&)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot enough memory");
        return false;
    };

    for (unsigned i = 0; i < pointCount; ++i)
    {
//        ccColor::Rgba color = ptrCloud->getPointColor(i);
        const CCVector3* ptrP3D = ptrCloud->getPoint(i);
        float x=ptrP3D->x;
        float y=ptrP3D->y;
        float z=ptrP3D->z;
        Point_set::iterator it_dhl = m_ptrPts->insert (Point (x, y, z));
    }
//    if(ptrCloud->hasColors())
    m_ptrFormerCloudColors=ptrFormerCloudColors;
    if(m_ptrFormerCloudColors)
    {
        m_color.reset();
        m_color = m_ptrPts->add_property_map<CGAL::IO::Color>("real_color").first;
        Point_set::const_iterator it = m_ptrPts->begin();
        for (unsigned i = 0; i < pointCount; ++i)
        {
//            ccColor::Rgba color = ptrCloud->getPointColor(i);
            ccColor::Rgba color = m_ptrFormerCloudColors->getValue(i);
            double red=static_cast<double>(color.r) / ccColor::MAX;
            double green=static_cast<double>(color.g) / ccColor::MAX;
            double blue=static_cast<double>(color.b) / ccColor::MAX;
            m_color[*it] = CGAL::IO::Color((unsigned char)(255 * red),
                                               (unsigned char)(255 * green),
                                               (unsigned char)(255 * blue));
            it++;
        }
    }

    m_ptrCCPointCloud=ptrCloud;
    m_ptrClassificationModel=ptrClassificationModel;
    return(true);
}

bool ccClassificationToolCGAL::classify(ParametersManager *ptrParametersManager,
                                        QString method,
                                        QString targetFieldName,
                                        QString &timeInSeconds,
                                        QString &strReport,
                                        QString &strError)
{
    QString functionName="ccClassificationToolCGAL::classify";
    strError.clear();
    timeInSeconds.clear();
    strReport.clear();
//    classifiedValues.clear();
    if(m_ptrPts==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nCGAL point cloud is not initialized");
        return(false);
    }
    if(m_ptrCCPointCloud==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nCloudCompare point cloud is not initialized");
        return(false);
    }
    if(m_ptrClassificationModel==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nClassification model is not initialized");
        return(false);
    }
    if(m_features.size()==0)
    {
        strError=functionName;
        strError+=QObject::tr("\nFeatures are not computed");
        return(false);
    }

    if(!ptrParametersManager)
    {
        strError=functionName;
        strError+=QObject::tr("\nNull pointer for ParametersManager");
        return(false);
    }

    QString strValue;
    bool okToInt=false,okToDouble=false;
    int intValue;
    double dblValue;

    QString parameterCode=RFC_PROCESS_CLASSIFICATION_ONLY_UNLOCKED_CLASSES;
    if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0)
    {
        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_ONLY_UNLOCKED_CLASSES;
    }
    else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
    {
        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_ONLY_UNLOCKED_CLASSES;
    }
    Parameter* ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    bool processOnlyUnlockedClasses=true;
    if(strValue.compare("true",Qt::CaseInsensitive)!=0
            &&strValue.compare("false",Qt::CaseInsensitive)!=0)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
        return(false);
    }
    if(strValue.compare("false",Qt::CaseInsensitive)==0)
    {
        processOnlyUnlockedClasses=false;
    }

    parameterCode=RFC_PROCESS_CLASSIFICATION_USE_TRAINING_FILE;
    if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0)
    {
        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_USE_TRAINING_FILE;
    }
    else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
    {
        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_USE_TRAINING_FILE;
    }
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    bool useTrainingFile=false;
    if(strValue.compare("true",Qt::CaseInsensitive)!=0
            &&strValue.compare("false",Qt::CaseInsensitive)!=0)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
        return(false);
    }
    if(strValue.compare("true",Qt::CaseInsensitive)==0)
    {
        useTrainingFile=true;
    }
    QString inputTrainingFileName;
    QString inputTrainingAuxiliarFileName;
    Label_set labels_fromFile;
    Classification::ETHZ::Random_forest_classifier* ptrEthzRFC_fromFile=NULL;
    QMap<int,int> newClassByOriginalClass_fromFile;
    QMap<int,QString> classLabelByOriginalClass_fromFile;
    QMap<int, int> originalClassByNewClass_fromFile;
    QStringList featuresNamesFromFile;
    if(useTrainingFile)
    {
        parameterCode=RFC_PROCESS_CLASSIFICATION_TRAINING_FILE;
        if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0)
        {
            parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_TRAINING_FILE;
        }
        else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
        {
            parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_TRAINING_FILE;
        }
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        inputTrainingFileName=strValue.trimmed();
    //    std::vector<std::vector<float> > labelProbabilities_fromFile;
        if(inputTrainingFileName.isEmpty())
        {
            strError=functionName;
            strError+=QObject::tr("\nTraining file not selected");
            return(false);
        }
        if(!QFile::exists(inputTrainingFileName))
        {
            strError=functionName;
            strError+=QObject::tr("\nNot exists input training file:\n%1")
                    .arg(inputTrainingFileName);
            return(false);
        }
        inputTrainingAuxiliarFileName=inputTrainingFileName+RFC_PROCESS_TRAINING_AUXLIAR_SAVE_FILE_SUFFIX;
        if(!QFile::exists(inputTrainingAuxiliarFileName))
        {
            strError=functionName;
            strError+=QObject::tr("\nNot exists input training auxiliar file:\n%1")
                    .arg(inputTrainingAuxiliarFileName);
            return(false);
        }
        QFile auxiliarFile(inputTrainingAuxiliarFileName);
        if(!auxiliarFile.open(QIODevice::ReadOnly))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening auxiliar training file:\n%1")
                    .arg(inputTrainingAuxiliarFileName);
            return(false);
        }
        QDataStream auxiliarIn(&auxiliarFile);
        auxiliarIn>>newClassByOriginalClass_fromFile;
        auxiliarIn>>classLabelByOriginalClass_fromFile;
        auxiliarIn>>featuresNamesFromFile;
        auxiliarIn>>m_numberOfScalesEthzRFC;
        auxiliarIn>>m_voxelSizeEthzRFC;
        auxiliarIn>>m_useColorsEthzRFC;
        auxiliarFile.close();
        QMap<int,int>::const_iterator iterNcBoC=newClassByOriginalClass_fromFile.begin();
        while(iterNcBoC!=newClassByOriginalClass_fromFile.end())
        {
            int originalClass=iterNcBoC.key();
            int newClass=iterNcBoC.value();
            originalClassByNewClass_fromFile[newClass]=originalClass;
            if(!classLabelByOriginalClass_fromFile.contains(originalClass))
            {
                strError=functionName;
                strError+=QObject::tr("\nIn auxiliar training file:\n%1")
                        .arg(inputTrainingAuxiliarFileName);
                strError+=QObject::tr("\nclass label not found for original classs: %1")
                        .arg(QString::number(originalClass));
                return(false);
            }
            QString qsClassLabel=classLabelByOriginalClass_fromFile[originalClass];
            std::string stdClassLabel=qsClassLabel.toStdString();
            const char* charClassLabel = stdClassLabel.c_str();
            Label_handle classLabel = labels_fromFile.add (charClassLabel);
            iterNcBoC++;
        }
        //        labelProbabilities_fromFile.resize (labels_fromFile.size());
        //        for (std::size_t i = 0; i < labelProbabilities_fromFile.size(); ++ i)
        //        {
        //            labelProbabilities_fromFile[i].resize (m_ptrPts->size(), -1);
        //        }
    }
    else
    {
        if(m_labels.size()==0)
        {
            strError=functionName;
            strError+=QObject::tr("\nLabels are not computed. Train before");
            return(false);
        }
        if(m_newClassByOriginalClass.size()!=m_labels.size())
        {
            strError=functionName;
            strError+=QObject::tr("\nInvalid new class by original class. Train before");
            return(false);
        }
        if(m_classLabelByOriginalClass.size()!=m_labels.size())
        {
            strError=functionName;
            strError+=QObject::tr("\nInvalid class label by original class. Train before");
            return(false);
        }
        if(m_originalClassByNewClass.size()!=m_labels.size())
        {
            strError=functionName;
            strError+=QObject::tr("\nInvalid original class by new class. Train before");
            return(false);
        }
        if(m_numberOfPointsByTrainingClass.size()!=m_labels.size())
        {
            strError=functionName;
            strError+=QObject::tr("\nInvalid number of points by training class. Train before");
            return(false);
        }
        m_labelProbabilities.clear();
        m_labelProbabilities.resize (m_labels.size());
        for (std::size_t i = 0; i < m_labelProbabilities.size(); ++ i)
        {
            m_labelProbabilities[i].resize (m_ptrPts->size(), -1);
        }
    }

    int numberOfSubdivisions=16;
    int regularizationWeight=0.5;
    if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
    {
        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_NUMBER_OF_SUBDIVISIONS;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        intValue=strValue.trimmed().toInt(&okToInt);
        if(!okToInt)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not an integer in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        numberOfSubdivisions=intValue;

        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_REGULARIZATION_WEIGHT;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        dblValue=strValue.trimmed().toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not a double in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        regularizationWeight=dblValue;
    }

    int targetFieldIndex=-1;
    unsigned sfCount = m_ptrCCPointCloud->getNumberOfScalarFields();
    QStringList scalarFields;
    if (m_ptrCCPointCloud->hasScalarFields())
    {
        for (unsigned i = 0; i < sfCount; ++i)
        {
            QString scalarFieldName=QString(m_ptrCCPointCloud->getScalarFieldName(i)).trimmed();
            if(scalarFieldName.compare(targetFieldName,Qt::CaseInsensitive)==0)
            {
                targetFieldIndex=(int)i;
                break;
            }
        }
    }
    else
    {
        strError=functionName;
        strError+=QObject::tr("\nThere are no scalar fields");
        return(false);
    }
    if(targetFieldIndex==-1)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists field: %1").arg(targetFieldName);
        return(false);
    }
    CCCoreLib::ScalarField* targetScalarField = m_ptrCCPointCloud->getScalarField(targetFieldIndex);
    if (!targetScalarField)
    {
        strError=functionName;
        strError+=QObject::tr("\nError getting field: %1").arg(targetFieldName);
        return(false);
    }

    bool needComputeFeatures=false;
    if(m_features.size()==0
            &&!useTrainingFile)
    {
        strError=functionName;
        strError+=QObject::tr("\nFeatures are not computed");
        return(false);
    }
    QString auxStrError;
    if(m_features.size()==0
            &&useTrainingFile)
    {
        QMessageBox msgBox;
        msgBox.setText("Features have not been computed");
        msgBox.setInformativeText("Do you want to compute features presents in input training file?");
        msgBox.setStandardButtons(QMessageBox::Apply | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Apply);
        int ret = msgBox.exec();
        switch (ret)
        {
        case QMessageBox::Apply:
            // Save was clicked
            if(!this->computeFeatures(ptrParametersManager,
                                      featuresNamesFromFile,
                                      timeInSeconds,
                                      strReport,
                                      auxStrError))
            {
                strError=functionName;
                strError+=QObject::tr("\nError computing features:\n%1").arg(auxStrError);
                return(false);
            }
            m_featuresNamesSelected=featuresNamesFromFile;
            break;
        case QMessageBox::Cancel:
            strError=functionName;
            strError+=QObject::tr("\nFeatures are not computed");
            return(false);
            // Cancel was clicked
            break;
        default:
            strError=functionName;
            strError+=QObject::tr("\nFeatures are not computed");
            return(false);
            // should never be reached
            break;
        }
    }
    if(m_features.size()>0
            &&useTrainingFile)
    {
        for(int np=0;np<m_featuresNamesSelected.size();np++)
        {
            QString computedFeatureStr=m_featuresNamesSelected.at(np);
            if(featuresNamesFromFile.indexOf(computedFeatureStr)==-1)
            {
                strError=functionName;
                strError+=QObject::tr("\nComputed feature: %1 is not in traning file")
                        .arg(computedFeatureStr);
                return(false);
                // Cancel was clicked
            }
            if(featuresNamesFromFile.indexOf(computedFeatureStr)!=np)
            {
                strError=functionName;
                strError+=QObject::tr("\nComputed feature: %1 is in traning file in another position")
                        .arg(computedFeatureStr);
                return(false);
                // Cancel was clicked
            }
        }
        for(int np=0;np<featuresNamesFromFile.size();np++)
        {
            QString computedFeatureStr=featuresNamesFromFile.at(np);
            if(m_featuresNamesSelected.indexOf(computedFeatureStr)==-1)
            {
                strError=functionName;
                strError+=QObject::tr("\nImported feature from training file: %1 is not in computed features")
                        .arg(computedFeatureStr);
                return(false);
                // Cancel was clicked
            }
            if(m_featuresNamesSelected.indexOf(computedFeatureStr)!=np)
            {
                strError=functionName;
                strError+=QObject::tr("\nImported feature from training file: %1 is in computed features in another position")
                        .arg(computedFeatureStr);
                return(false);
                // Cancel was clicked
            }
        }
    }

    std::vector<int> indices (m_ptrPts->size(), -1);
    CGAL::Real_timer t;
    t.start();
    if(useTrainingFile)
    {
        ptrEthzRFC_fromFile=new Classification::ETHZ::Random_forest_classifier(labels_fromFile, m_features);
        std::string stdTrainingFileName=inputTrainingFileName.toStdString();
        std::ifstream trainingFile (stdTrainingFileName, std::ios_base::in | std::ios_base::binary);
        ptrEthzRFC_fromFile->load_configuration (trainingFile);
        if(method.compare(RFC_PROCESS_CLASSIFICATION,Qt::CaseInsensitive)==0)
        {
            CGAL::Classification::classify<Concurrency_tag> (*(m_ptrPts),
                                                             labels_fromFile,
                                                             *ptrEthzRFC_fromFile,
                                                             indices);
        }
        else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0)
        {
            CGAL::Classification::classify_with_local_smoothing<Concurrency_tag> (*(m_ptrPts),
                                                                                  m_ptrPts->point_map(),
                                                                                  labels_fromFile,
                                                                                  *ptrEthzRFC_fromFile,
                 m_ptrFeatureGenerator->neighborhood().sphere_neighbor_query(m_ptrFeatureGenerator->radius_neighbors()),
                                                                                  indices);
        }
        else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
        {
            CGAL::Classification::classify_with_graphcut<Concurrency_tag> (*(m_ptrPts),
                                                                           m_ptrPts->point_map(),
                                                                           labels_fromFile,
                                                                           *ptrEthzRFC_fromFile,
                                                                           m_ptrFeatureGenerator->neighborhood().k_neighbor_query(12),
                                                                           regularizationWeight,
                                                                           numberOfSubdivisions,
                                                                           indices);
        }
        t.stop();
        timeInSeconds=QString::number(t.time(),'f',2);
        int counter=0;
        for (Point_set::const_iterator it = m_ptrPts->begin();
             it != m_ptrPts->end(); ++ it)
        {
            ScalarType targetFloatValue = targetScalarField->getValue(counter);
            int targetOriginalClass=qRound(targetFloatValue);
            if(targetOriginalClass==CC_CLASSIFICATION_MODEL_REMOVED_CODE)
            {
                counter++;
                continue;
            }
            // consulto el item del campo escalar de entrenamiento porque es donde se indican
            // las clases a bloquear. El modelo es unico para entrenamiento y clasificacion
            ccClassificationModel::Item* ptrItem = m_ptrClassificationModel->find(targetOriginalClass);
            if(ptrItem->locked)
            {
                counter++;
                continue;
            }
    //        if(targetOriginalClass==CC_CLASSIFICATION_MODEL_ASPRS_SELECTED_CODE)
    //        {
    //            counter++;
    //            continue;
    //        }

    //        m_classif[*it] = indices[*it];
//            int trainingClass = m_training[*it];
            int newClass = indices[*it];
//            if(trainingClass !=-1)
//            {
//                if(newClass==trainingClass)
//                {
//                    if(!numberOfSuccessPointsByTrainingClass.contains(trainingClass))
//                    {
//                        numberOfSuccessPointsByTrainingClass[trainingClass]=0;
//                    }
//                    numberOfSuccessPointsByTrainingClass[trainingClass]=numberOfSuccessPointsByTrainingClass[trainingClass]+1;
//                }
//            }
            int classifiedValue=newClass;
            if(newClass!=-1)
            {
                if(originalClassByNewClass_fromFile.contains(newClass))
                {
                    classifiedValue=originalClassByNewClass_fromFile[newClass];
                }
            }
    //        classifiedValues[counter]=classifiedValue;
            ScalarType floatClassifiedValue = static_cast<ScalarType>(classifiedValue);
            targetScalarField->setValue(counter,floatClassifiedValue);
            counter++;
        }
        delete(ptrEthzRFC_fromFile);
        ptrEthzRFC_fromFile=NULL;
    }
    else
    {
        if(method.compare(RFC_PROCESS_CLASSIFICATION,Qt::CaseInsensitive)==0)
        {
            CGAL::Classification::classify<Concurrency_tag> (*(m_ptrPts),
                                                             m_labels,
                                                             *m_ptrEthzRFC,
                                                             indices,
                                                             m_labelProbabilities);
        }
        else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0)
        {
            CGAL::Classification::classify_with_local_smoothing<Concurrency_tag> (*(m_ptrPts),
                                                                                  m_ptrPts->point_map(),
                                                                                  m_labels,
                                                                                  *m_ptrEthzRFC,
                 m_ptrFeatureGenerator->neighborhood().sphere_neighbor_query(m_ptrFeatureGenerator->radius_neighbors()),
                                                                                  indices);
        }
        else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
        {
            CGAL::Classification::classify_with_graphcut<Concurrency_tag> (*(m_ptrPts),
                                                                           m_ptrPts->point_map(),
                                                                           m_labels,
                                                                           *m_ptrEthzRFC,
                                                                           m_ptrFeatureGenerator->neighborhood().k_neighbor_query(12),
                                                                           regularizationWeight,
                                                                           numberOfSubdivisions,
                                                                           indices);
        }
        t.stop();
        timeInSeconds=QString::number(t.time(),'f',2);
        QMap<int,int> numberOfSuccessPointsByTrainingClass;
        int counter=0;
        for (Point_set::const_iterator it = m_ptrPts->begin();
             it != m_ptrPts->end(); ++ it)
        {
            ScalarType targetFloatValue = targetScalarField->getValue(counter);
            int targetOriginalClass=qRound(targetFloatValue);
            if(targetOriginalClass==CC_CLASSIFICATION_MODEL_REMOVED_CODE)
            {
                counter++;
                continue;
            }
            // consulto el item del campo escalar de entrenamiento porque es donde se indican
            // las clases a bloquear. El modelo es unico para entrenamiento y clasificacion
            ccClassificationModel::Item* ptrItem = m_ptrClassificationModel->find(targetOriginalClass);
            if(ptrItem->locked)
            {
                counter++;
                continue;
            }
    //        if(targetOriginalClass==CC_CLASSIFICATION_MODEL_ASPRS_SELECTED_CODE)
    //        {
    //            counter++;
    //            continue;
    //        }

    //        m_classif[*it] = indices[*it];
            int trainingClass = m_training[*it];
            int newClass = indices[*it];
            if(trainingClass !=-1)
            {
                if(newClass==trainingClass)
                {
                    if(!numberOfSuccessPointsByTrainingClass.contains(trainingClass))
                    {
                        numberOfSuccessPointsByTrainingClass[trainingClass]=0;
                    }
                    numberOfSuccessPointsByTrainingClass[trainingClass]=numberOfSuccessPointsByTrainingClass[trainingClass]+1;
                }
            }
            int classifiedValue=newClass;
            if(newClass!=-1)
            {
                if(m_originalClassByNewClass.contains(newClass))
                {
                    classifiedValue=m_originalClassByNewClass[newClass];
                }
            }
    //        classifiedValues[counter]=classifiedValue;
            ScalarType floatClassifiedValue = static_cast<ScalarType>(classifiedValue);
            targetScalarField->setValue(counter,floatClassifiedValue);
            counter++;
        }
        strReport+="Reliability results::\n";
        QMap<int,int>::const_iterator iter1=m_originalClassByNewClass.begin();
        while(iter1!=m_originalClassByNewClass.end())
        {
            int newClass=iter1.key();
            int originalClass=iter1.value();
            QString className=m_classLabelByOriginalClass[originalClass];
            int numberOfPoints=m_numberOfPointsByTrainingClass[newClass];
            int numberOfSuccessPoints=numberOfSuccessPointsByTrainingClass[newClass];
            float reliabilityPercentage=100.*numberOfSuccessPoints/numberOfPoints;
            strReport+= " * ";
            strReport+=className;
            strReport+=": ";
            strReport+=QString::number(reliabilityPercentage,'f',1);
            iter1++;
        }

        std::vector<int> ground_truth(m_ptrPts->size(), -1);
        for (Point_set::const_iterator it = m_ptrPts->begin();
             it != m_ptrPts->end(); ++ it)
          {
    //        m_classif[*it] = indices[*it];
            ground_truth[*it] = m_training[*it];
          }

        QString strReportCGAL="Precision, recall, F1 scores and IoU:\n";
        CGAL::Classification::Evaluation eval (m_labels, ground_truth, indices);
        for (std::size_t i = 0; i < m_labels.size(); ++ i)
        {
            strReportCGAL+= " * ";
            strReportCGAL+=QString::fromStdString(m_labels[i]->name());
            strReportCGAL+=": ";
            strReportCGAL+=QString::number(eval.precision(m_labels[i]),'f',2);
            strReportCGAL+=" ; ";
            strReportCGAL+=QString::number(eval.recall(m_labels[i]),'f',2);
            strReportCGAL+=" ; ";
            strReportCGAL+=QString::number(eval.f1_score(m_labels[i]),'f',2);
            strReportCGAL+=" ; ";
            strReportCGAL+=QString::number(eval.intersection_over_union(m_labels[i]),'f',2);
            strReportCGAL+="\n";
        }
        strReportCGAL+="Accuracy = ";
        strReportCGAL+=QString::number(eval.accuracy(),'f',2);
        strReportCGAL+="\n";
        strReportCGAL+="Mean F1 score = ";
        strReportCGAL+=QString::number(eval.mean_f1_score(),'f',2);
        strReportCGAL+="\n";
        strReportCGAL+="Mean IoU = ";
        strReportCGAL+=QString::number(eval.mean_intersection_over_union(),'f',2);
        strReportCGAL+="\n";
    }
    return(true);
}

bool ccClassificationToolCGAL::computeFeatures(ParametersManager *ptrParametersManager,
                                               QStringList &featuresNamesFromTrainingFile,
                                               QString& timeInSeconds,
                                               QString &strReport,
                                               QString &strError)
{
    QString functionName="ccClassificationToolCGAL::computeFeatures";
    timeInSeconds.clear();
    strError.clear();
    strReport.clear();
    if(m_ptrPts==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nCGAL point cloud is not initialized");
        return(false);
    }
    if(m_ptrFeatureGenerator!=NULL)
    {
        delete m_ptrFeatureGenerator;
    }
    m_features.clear();
    m_featuresNamesSelected.clear();
    m_labels.clear();
    if(!ptrParametersManager)
    {
        strError=functionName;
        strError+=QObject::tr("\nNull pointer for ParametersManager");
        return(false);
    }
    QString strValue;
    bool okToInt=false,okToDouble=false;
    int numberOfScales=5; // default value
    double voxelSize=-1.f; // automatico
    QString parameterCode;
    Parameter* ptrParameter=NULL;
    bool useColor=false;
    if(featuresNamesFromTrainingFile.isEmpty())
    {
        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NUMBER_OF_SCALES;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        numberOfScales=strValue.toInt(&okToInt);
        if(!okToInt)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not an integer in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_VOXEL_SIZE;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        voxelSize=strValue.toDouble(&okToDouble);
        if(!okToInt)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not a double in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        if(fabs(voxelSize)<0.01)
        {
            voxelSize=-1.f;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_COLOR;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        useColor=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            useColor=true;
        }
    }
    else
    {
        numberOfScales=m_numberOfScalesEthzRFC;
        voxelSize=m_voxelSizeEthzRFC;
        useColor=m_useColorsEthzRFC;
    }

    QString outputFilename;
    parameterCode=RFC_PROCESS_COMPUTE_FEATURES_OUTPUT_FILE;
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    outputFilename=strValue.trimmed();
    if(!outputFilename.isEmpty())
    {
        if(QFile::exists(outputFilename))
        {
            if(!QFile::remove(outputFilename))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing existing training file:\n%1")
                        .arg(outputFilename);
                return(false);
            }
        }
    }

//    std::cout << "Generating features" << std::endl;
    CGAL::Real_timer t;
    t.start();
    m_ptrFeatureGenerator= new Feature_generator((*m_ptrPts),
                                 m_ptrPts->point_map(),
                                 numberOfScales,
                                 voxelSize);
#ifdef CGAL_LINKED_WITH_TBB
  m_features.begin_parallel_additions();
#endif

//    bool colors=m_ptrCCPointCloud->hasColors();
    bool colors=false;
    if(m_ptrFormerCloudColors) colors=true;

    m_ptrFeatureGenerator->generate_point_based_features(m_features);

//    // normals
//    CGAL::pca_estimate_normals<Concurrency_tag>
//        (points, nb_neighbors,
//         CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointVectorPair>())
//                          .normal_map(CGAL::Second_of_pair_property_map<PointVectorPair>()));

//    const int nb_neighbors = 18; // K-nearest neighbors = 3 rings
//    (*m_ptrPts).add_normal_map();
//    CGAL::jet_estimate_normals<CGAL::Parallel_if_available_tag> ((*m_ptrPts), nb_neighbors);

////    // normals
////    const int nb_neighbors = 18; // K-nearest neighbors = 3 rings
////    // First compute a spacing using the K parameter
////    double spacing
////        = CGAL::compute_average_spacing<Concurrency_tag>
////          ((*m_ptrPts), nb_neighbors,
////           CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointVectorPair>()));
////    // Then, estimate normals with a fixed radius
////    CGAL::pca_estimate_normals<Concurrency_tag>
////        ((*m_ptrPts),
////         0, // when using a neighborhood radius, K=0 means no limit on the number of neighbors returns
////         CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointVectorPair>())
////                          .normal_map(CGAL::Second_of_pair_property_map<PointVectorPair>())
////                          .neighbor_radius(2. * spacing)); // use 2*spacing as neighborhood radius
//    Vmap normal_map = (*m_ptrPts).normal_map();
//    bool normals=true;
//    if (normals)
//        m_ptrFeatureGenerator->generate_normal_based_features (m_features, normal_map);


    if(useColor)
    {
        if (colors)
        {
            m_ptrFeatureGenerator->generate_color_based_features (m_features, m_color);
        }
        else
        {
            if(!featuresNamesFromTrainingFile.isEmpty())
            {
                strError=functionName;
                strError+=QObject::tr("\nExists colors in training data but this point cloud has no colors");
                return(false);
            }
        }
    }
    //  if (echo)
    //    m_ptrFeatureGenerator->generate_echo_based_features (m_features, echo_map);

#ifdef CGAL_LINKED_WITH_TBB
    m_features.end_parallel_additions();
#endif
    //    m_features.begin_parallel_additions();
    //    generator.generate_point_based_features (m_features);
    //    m_features.end_parallel_additions();
    t.stop();
    timeInSeconds=QString::number(t.time(),'f',2);
    addRemainingPointSetPropertiesAsFeatures();
    if (m_ptrEthzRFC != NULL)
    {
        delete m_ptrEthzRFC;
        m_ptrEthzRFC = NULL;
    }
    if(!outputFilename.isEmpty())
    {
        QFile file(outputFilename);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening output file: %1").arg(outputFilename);
            return(false);
        }
        QTextStream strOut(&file);
        for (std::size_t i = 0; i < m_features.size(); ++ i)
        {
            float vmin = (std::numeric_limits<float>::max)();
            float vmax = -(std::numeric_limits<float>::max)();
            float vmean = 0.f;
            std::size_t nb = 0;

            for (Point_set::const_iterator it = m_ptrPts->begin();
                 it != m_ptrPts->end(); ++ it)
            {
                float v = m_features[i]->value(std::size_t(it - m_ptrPts->begin()));
                vmin = (std::min) (vmin, v);
                vmax = (std::max) (vmax, v);
                vmean += v;
                ++ nb;
            }
            strOut<<QString::fromStdString(m_features[i]->name());
            strOut<< " in [ " << QString::number(vmin,'f',9) << " ; ";
            strOut<< QString::number(vmax,'f',9);
            strOut<< " ], mean = " << QString::number(vmean / nb,'f',9);
            strOut<<"\n";
        }
        file.close();
    }
    for (std::size_t i = 0; i < m_features.size(); ++ i)
    {
        m_featuresNamesSelected.append(QString::fromStdString(m_features[i]->name()));//incluye el scale
    }
    m_numberOfScalesEthzRFC=numberOfScales;
    m_voxelSizeEthzRFC=voxelSize;
    m_useColorsEthzRFC=useColor;
    return(true);
}

bool ccClassificationToolCGAL::train(ParametersManager *ptrParametersManager,
                                     QString trainFieldName,
                                     QString targetFieldName,
                                     QString &timeInSeconds,
                                     QString &strReport,
//                                     QVector<int> &classifiedValues,
                                     QString &strError)
{
    QString functionName="ccClassificationToolCGAL::train";
    strError.clear();
    timeInSeconds.clear();
    strReport.clear();
//    classifiedValues.clear();
    if(m_ptrPts==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nCGAL point cloud is not initialized");
        return(false);
    }
    if(m_ptrCCPointCloud==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nCloudCompare point cloud is not initialized");
        return(false);
    }
    if(m_ptrClassificationModel==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nClassification model is not initialized");
        return(false);
    }
    if(m_features.size()==0)
    {
        strError=functionName;
        strError+=QObject::tr("\nFeatures are not computed");
        return(false);
    }
    if(!ptrParametersManager)
    {
        strError=functionName;
        strError+=QObject::tr("\nNull pointer for ParametersManager");
        return(false);
    }

    QString strValue;
    bool okToInt=false,okToDouble=false;
    int intValue;
    double dblValue;

    QString parameterCode=RFC_PROCESS_TRAINING_PROCESS_ONLY_UNLOCKED_CLASSES;
    Parameter* ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    bool processOnlyUnlockedClasses=true;
    if(strValue.compare("true",Qt::CaseInsensitive)!=0
            &&strValue.compare("false",Qt::CaseInsensitive)!=0)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
        return(false);
    }
    if(strValue.compare("false",Qt::CaseInsensitive)==0)
    {
        processOnlyUnlockedClasses=false;
    }

    parameterCode=RFC_PROCESS_TRAINING_PROCESS_NUMBER_OF_TREES;
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    int numberOfTrees=25;
    intValue=strValue.trimmed().toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not an integer in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    numberOfTrees=intValue;

    parameterCode=RFC_PROCESS_TRAINING_PROCESS_MAXIMUM_DEEP;
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    int maximumDeep=20;
    intValue=strValue.trimmed().toInt(&okToInt);
    if(!okToInt)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not an integer in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    maximumDeep=intValue;

    QString output_training_filename;
    parameterCode=RFC_PROCESS_TRAINING_PROCESS_OUTPUT_TRAINING_FILE;
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    output_training_filename=strValue.trimmed();
    QString output_auxiliar_training_filename;
    if(!output_training_filename.isEmpty())
    {
        if(QFile::exists(output_training_filename))
        {
            if(!QFile::remove(output_training_filename))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing existing training file:\n%1")
                        .arg(output_training_filename);
                return(false);
            }
        }
//        QFileInfo output_training_file_info(output_training_filename);
        output_auxiliar_training_filename=output_training_filename+RFC_PROCESS_TRAINING_AUXLIAR_SAVE_FILE_SUFFIX;
        if(QFile::exists(output_auxiliar_training_filename))
        {
            if(!QFile::remove(output_auxiliar_training_filename))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing existing auxiliar training file:\n%1")
                        .arg(output_auxiliar_training_filename);
                return(false);
            }
        }
    }

    int trainFieldIndex=-1;
    int targetFieldIndex=-1;
    unsigned sfCount = m_ptrCCPointCloud->getNumberOfScalarFields();
    QStringList scalarFields;
    if (m_ptrCCPointCloud->hasScalarFields())
    {
        for (unsigned i = 0; i < sfCount; ++i)
        {
            QString scalarFieldName=QString(m_ptrCCPointCloud->getScalarFieldName(i)).trimmed();
            if(scalarFieldName.compare(trainFieldName,Qt::CaseInsensitive)==0)
            {
                trainFieldIndex=(int)i;
                if(targetFieldIndex!=-1) break;
            }
            if(scalarFieldName.compare(targetFieldName,Qt::CaseInsensitive)==0)
            {
                targetFieldIndex=(int)i;
                if(trainFieldIndex!=-1) break;
            }
        }
    }
    else
    {
        strError=functionName;
        strError+=QObject::tr("\nThere are no scalar fields");
        return(false);
    }
    if(trainFieldIndex==-1)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists field: %1").arg(trainFieldName);
        return(false);
    }
    if(targetFieldIndex==-1)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists field: %1").arg(targetFieldName);
        return(false);
    }
    std::string stdTrainFieldName=trainFieldName.toStdString();
    if(m_ptrPts->has_property_map<int>(stdTrainFieldName))
    {
        Imap iInputClassificationMap;
        bool success=false;
        boost::tie (iInputClassificationMap, success) =  m_ptrPts->property_map<int>(stdTrainFieldName);
        if(!success)
        {
            strError=functionName;
            strError+=QObject::tr("\nError in CGAL points container getting propierty: %1")
                    .arg(trainFieldName);
            return(false);
        }
        if(!m_ptrPts->remove_property_map<int>(iInputClassificationMap))
        {
            strError=functionName;
            strError+=QObject::tr("\nError in CGAL points container removing classification input field: %1")
                    .arg(QString::fromStdString(stdTrainFieldName));
            return(false);
        }
    }
    CCCoreLib::ScalarField* trainScalarField = m_ptrCCPointCloud->getScalarField(trainFieldIndex);
    if (!trainScalarField)
    {
        strError=functionName;
        strError+=QObject::tr("\nError getting field: %1").arg(trainFieldName);
        return(false);
    }
    CCCoreLib::ScalarField* targetScalarField = m_ptrCCPointCloud->getScalarField(targetFieldIndex);
    if (!targetScalarField)
    {
        strError=functionName;
        strError+=QObject::tr("\nError getting field: %1").arg(targetFieldName);
        return(false);
    }

    QVector<int> originalValues;
    m_newClassByOriginalClass.clear();
    m_classLabelByOriginalClass.clear();
//    classLabelByOriginalClass[2]="ground";
//    classLabelByOriginalClass[5]="vegetation";
//    classLabelByOriginalClass[6]="roof";
////    classLabelByOriginalClass[0]="ground";
////    classLabelByOriginalClass[1]="vegetation";
////    classLabelByOriginalClass[2]="roof";
    unsigned cloudSize = m_ptrCCPointCloud->size();
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        ScalarType floatValue = trainScalarField->getValue(i);
        int originalClass=qRound(floatValue);
//        const auto& color =ptrCloud->getPointColor(i);
        ccClassificationModel::Item* ptrItem = m_ptrClassificationModel->find(originalClass);
        originalValues.push_back(originalClass);
        if(!ptrItem->train)
        {
            continue;
        }
        QString classLabel=ptrItem->name;
        if(!m_newClassByOriginalClass.contains(originalClass))
        {
            m_newClassByOriginalClass[originalClass]=RFC_PROCESS_CLASSIFICATION_NO_VALUE;
        }
        if(!m_classLabelByOriginalClass.contains(originalClass))
        {
            m_classLabelByOriginalClass[originalClass]=classLabel;
        }
    }
    if(m_newClassByOriginalClass.size()==0)
    {
        strError=functionName;
        strError+=QObject::tr("\nSelect at least one class to train");
        return(false);
    }
    QMap<int,int>::Iterator iterClasses=m_newClassByOriginalClass.begin();
    int cont=0;
    m_originalClassByNewClass.clear();
    m_labels.clear();
    m_labelProbabilities.clear();
    while(iterClasses!=m_newClassByOriginalClass.end())
    {
        int originalClass=iterClasses.key();
//        if(originalClass==-1)
//        {
//            iterClasses++;
//        }
//        if(originalClass>-1&&cont==-1) cont=0; // si no hay puntos unclassified empiezo a contar en 0
        int newClass=cont;
        m_newClassByOriginalClass[originalClass]=newClass;
        m_originalClassByNewClass[newClass]=originalClass;
        QString qsClassLabel=m_classLabelByOriginalClass[originalClass];
        std::string stdClassLabel=qsClassLabel.toStdString();
        const char* charClassLabel = stdClassLabel.c_str();
        Label_handle classLabel = m_labels.add (charClassLabel);
        cont++;
        iterClasses++;
    }
    m_labelProbabilities.resize (m_labels.size());
    for (std::size_t i = 0; i < m_labelProbabilities.size(); ++ i)
        m_labelProbabilities[i].resize (m_ptrPts->size(), -1);

    Imap iOutputClassificationMap;
    bool success=false;
//    std::string outputClassificationFieldName=RFC_PROCESS_CLASSIFICATION_FIELD_NAME;
//    boost::tie (iOutputClassificationMap, success) = m_ptrPts->add_property_map<int> (outputClassificationFieldName);
    boost::tie (iOutputClassificationMap, success) = m_ptrPts->add_property_map<int> (stdTrainFieldName);
    if(!success)
    {
        strError=functionName;
        strError+=QObject::tr("\nError appening output classification field name: %1")
                .arg(QString::fromStdString(stdTrainFieldName));
        return(false);
    }
    int pos=0;
    for (Point_set::const_iterator it = m_ptrPts->begin(); it != m_ptrPts->end(); ++ it)
//    for (unsigned i = 0; i < cloudSize; ++i)
    {
        int originalClass=originalValues[pos];
//        int originalClass=originalValues[i];
        int newClass=originalClass;
        if(m_newClassByOriginalClass.contains(originalClass))
        {
            newClass=m_newClassByOriginalClass[originalClass];
        }
        else
        {
            newClass=RFC_PROCESS_CLASSIFICATION_NO_VALUE;
        }
        iOutputClassificationMap[*it]=newClass;
        pos++;
//        iOutputClassificationMap[i]=newClass;
    }
    m_training.clear();
    m_training.resize(m_ptrPts->size(), -1);
    std::vector<int> indices (m_ptrPts->size(), -1);

    std::vector<std::size_t> nb_label (m_labels.size(), 0);
    std::size_t nb_total = 0;
    m_numberOfPointsByTrainingClass.clear();
    for (Point_set::const_iterator it = m_ptrPts->begin();
         it != m_ptrPts->end(); ++ it)
    {
        m_training[*it] = iOutputClassificationMap[*it];
        if (m_training[*it] != -1)
        {
            nb_label[std::size_t(m_training[*it])] ++;
            ++ nb_total;
            if(!m_numberOfPointsByTrainingClass.contains(m_training[*it]))
            {
                m_numberOfPointsByTrainingClass[m_training[*it]]=0;
            }
            m_numberOfPointsByTrainingClass[m_training[*it]]=m_numberOfPointsByTrainingClass[m_training[*it]]+1;
        }
    }
    strReport.clear();
    strReport=QString::number(nb_total);
    strReport+=" point(s) used for training (";
    strReport+=QString::number(100. * (nb_total / double(m_ptrPts->size())),'f',3);
    strReport+="% of the total):\n";
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
    {
        strReport+= " * ";
        strReport+=QString::fromStdString( m_labels[i]->name());
        strReport+=": ";
        strReport+=QString::number(nb_label[i]);
        strReport+=" point(s)\n";
    }
    m_ptrEthzRFC=new Classification::ETHZ::Random_forest_classifier(m_labels, m_features);
    std::size_t num_trees = numberOfTrees;
    std::size_t max_depth = maximumDeep;
    CGAL::Real_timer t;
    t.start();
    m_ptrEthzRFC->train<Concurrency_tag>(m_training,
                                         true,//reset_trees
                                         num_trees,
                                         max_depth);
    CGAL::Classification::classify<Concurrency_tag> (*(m_ptrPts),
                                                     m_labels,
                                                     *m_ptrEthzRFC,
                                                     indices,
                                                     m_labelProbabilities);

    t.stop();
    timeInSeconds=QString::number(t.time(),'f',2);
//    classifiedValues.resize(m_ptrPts->size());
    QMap<int,int> numberOfSuccessPointsByTrainingClass;
    int counter=0;
    for (Point_set::const_iterator it = m_ptrPts->begin();
         it != m_ptrPts->end(); ++ it)
    {
        ScalarType targetFloatValue = targetScalarField->getValue(counter);
        int targetOriginalClass=qRound(targetFloatValue);
        if(targetOriginalClass==CC_CLASSIFICATION_MODEL_REMOVED_CODE)
        {
            counter++;
            continue;
        }
        // consulto el item del campo escalar de entrenamiento porque es donde se indican
        // las clases a bloquear. El modelo es unico para entrenamiento y clasificacion
        ccClassificationModel::Item* ptrItem = m_ptrClassificationModel->find(targetOriginalClass);
        if(ptrItem->locked)
        {
            counter++;
            continue;
        }
//        if(targetOriginalClass==CC_CLASSIFICATION_MODEL_ASPRS_SELECTED_CODE)
//        {
//            counter++;
//            continue;
//        }

//        m_classif[*it] = indices[*it];
        int trainingClass = m_training[*it];
        int newClass = indices[*it];
        if(trainingClass !=-1)
        {
            if(newClass==trainingClass)
            {
                if(!numberOfSuccessPointsByTrainingClass.contains(trainingClass))
                {
                    numberOfSuccessPointsByTrainingClass[trainingClass]=0;
                }
                numberOfSuccessPointsByTrainingClass[trainingClass]=numberOfSuccessPointsByTrainingClass[trainingClass]+1;
            }
        }
        int classifiedValue=newClass;
        if(newClass!=-1)
        {
            if(m_originalClassByNewClass.contains(newClass))
            {
                classifiedValue=m_originalClassByNewClass[newClass];
            }
        }
//        classifiedValues[counter]=classifiedValue;
        ScalarType floatClassifiedValue = static_cast<ScalarType>(classifiedValue);
        targetScalarField->setValue(counter,floatClassifiedValue);
        counter++;
    }
    strReport+="Reliability results::\n";
    QMap<int,int>::const_iterator iter1=m_originalClassByNewClass.begin();
    while(iter1!=m_originalClassByNewClass.end())
    {
        int newClass=iter1.key();
        int originalClass=iter1.value();
        QString className=m_classLabelByOriginalClass[originalClass];
        int numberOfPoints=m_numberOfPointsByTrainingClass[newClass];
        int numberOfSuccessPoints=numberOfSuccessPointsByTrainingClass[newClass];
        float reliabilityPercentage=100.*numberOfSuccessPoints/numberOfPoints;
        strReport+= " * ";
        strReport+=className;
        strReport+=": ";
        strReport+=QString::number(reliabilityPercentage,'f',1);
        iter1++;
    }
    if(!output_training_filename.isEmpty())
    {
        std::string stdOutputTrainingFileName=output_training_filename.toStdString();
        std::ofstream outputTrainingFile (stdOutputTrainingFileName,
                                          std::ios_base::binary);
        m_ptrEthzRFC->save_configuration(outputTrainingFile);
        outputTrainingFile.close();
        QFile auxiliarFile(output_auxiliar_training_filename);
        if(!auxiliarFile.open(QIODevice::WriteOnly))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening auxiliar training file:\n%1")
                    .arg(output_auxiliar_training_filename);
            return(false);
        }
        QDataStream auxiliarOut(&auxiliarFile);
        auxiliarOut<<m_newClassByOriginalClass;
        auxiliarOut<<m_classLabelByOriginalClass;
        auxiliarOut<<m_featuresNamesSelected;
        auxiliarOut<<m_numberOfScalesEthzRFC;
        auxiliarOut<<m_voxelSizeEthzRFC;
        auxiliarOut<<m_useColorsEthzRFC;
        auxiliarFile.close();
    }
    return(true);
}

bool ccClassificationToolCGAL::trainingClassesSeparabiltyAnalysis(ParametersManager *ptrParametersManager,
                                                                  QString trainFieldName,
                                                                  QString &timeInSeconds,
                                                                  QString &strReport,
                                                                  QString &strError)
{
    QString functionName="ccClassificationToolCGAL::trainingClassesSeparabiltyAnalysis";
    strError.clear();
    timeInSeconds.clear();
    strReport.clear();
//    classifiedValues.clear();
    if(m_ptrPts==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nCGAL point cloud is not initialized");
        return(false);
    }
    if(m_ptrCCPointCloud==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nCloudCompare point cloud is not initialized");
        return(false);
    }
    if(m_ptrClassificationModel==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nClassification model is not initialized");
        return(false);
    }
    if(m_features.size()==0)
    {
        strError=functionName;
        strError+=QObject::tr("\nFeatures are not computed");
        return(false);
    }
    if(!ptrParametersManager)
    {
        strError=functionName;
        strError+=QObject::tr("\nNull pointer for ParametersManager");
        return(false);
    }

    QString strValue;
    bool okToInt=false,okToDouble=false;
    int intValue;
    double dblValue;

    QString parameterCode=RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS_ONLY_UNLOCKED_CLASSES;
    Parameter* ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    bool processOnlyUnlockedClasses=true;
    if(strValue.compare("true",Qt::CaseInsensitive)!=0
            &&strValue.compare("false",Qt::CaseInsensitive)!=0)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
        return(false);
    }
    if(strValue.compare("false",Qt::CaseInsensitive)==0)
    {
        processOnlyUnlockedClasses=false;
    }

    parameterCode=RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS_STANDARIZE_PARAMETERS;
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    bool normalizeParameters=true;
    if(strValue.compare("true",Qt::CaseInsensitive)!=0
            &&strValue.compare("false",Qt::CaseInsensitive)!=0)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
        return(false);
    }
    if(strValue.compare("false",Qt::CaseInsensitive)==0)
    {
        normalizeParameters=false;
    }

//    QString outputParametersFilename;
//    parameterCode=RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS_PARAMETERS_OUTPUT_FILE;
//    ptrParameter=ptrParametersManager->getParameter(parameterCode);
//    if(ptrParameter==NULL)
//    {
//        strError=functionName;
//        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
//                .arg(parameterCode).arg(ptrParametersManager->getFileName());
//        return(false);
//    }
//    ptrParameter->getValue(strValue);
//    outputParametersFilename=strValue.trimmed();
//    if(!outputParametersFilename.isEmpty())
//    {
//        if(QFile::exists(outputParametersFilename))
//        {
//            if(!QFile::remove(outputParametersFilename))
//            {
//                strError=functionName;
//                strError+=QObject::tr("\nError removing existing file:\n%1")
//                        .arg(outputParametersFilename);
//                return(false);
//            }
//        }
//    }

    QString outputFilename;
    parameterCode=RFC_PROCESS_TRAINING_CLASS_SEPARABILITY_ANALYSIS_OUTPUT_FILE;
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    outputFilename=strValue.trimmed();
    if(!outputFilename.isEmpty())
    {
        if(QFile::exists(outputFilename))
        {
            if(!QFile::remove(outputFilename))
            {
                strError=functionName;
                strError+=QObject::tr("\nError removing existing file:\n%1")
                        .arg(outputFilename);
                return(false);
            }
        }
    }

    int trainFieldIndex=-1;
    unsigned sfCount = m_ptrCCPointCloud->getNumberOfScalarFields();
    QStringList scalarFields;
    if (m_ptrCCPointCloud->hasScalarFields())
    {
        for (unsigned i = 0; i < sfCount; ++i)
        {
            QString scalarFieldName=QString(m_ptrCCPointCloud->getScalarFieldName(i)).trimmed();
            if(scalarFieldName.compare(trainFieldName,Qt::CaseInsensitive)==0)
            {
                trainFieldIndex=(int)i;
                break;
            }
        }
    }
    else
    {
        strError=functionName;
        strError+=QObject::tr("\nThere are no scalar fields");
        return(false);
    }
    if(trainFieldIndex==-1)
    {
        strError=functionName;
        strError+=QObject::tr("\nNot exists field: %1").arg(trainFieldName);
        return(false);
    }

    CCCoreLib::ScalarField* trainScalarField = m_ptrCCPointCloud->getScalarField(trainFieldIndex);
    if (!trainScalarField)
    {
        strError=functionName;
        strError+=QObject::tr("\nError getting field: %1").arg(trainFieldName);
        return(false);
    }
    unsigned cloudSize = m_ptrCCPointCloud->size();
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        ScalarType floatValue = trainScalarField->getValue(i);
        int originalClass=qRound(floatValue);
//        const auto& color =ptrCloud->getPointColor(i);
        ccClassificationModel::Item* ptrItem = m_ptrClassificationModel->find(originalClass);
        if(!ptrItem->train)
        {
            continue;
        }
        QString classLabel=ptrItem->name;


    }


    return(true);
}

template <typename Type>
bool ccClassificationToolCGAL::try_adding_simple_feature (const std::string& name)
{
    typedef typename Point_set::template Property_map<Type> Pmap;
    bool okay = false;
    Pmap pmap;
    boost::tie (pmap, okay) = m_ptrPts->template property_map<Type>(name.c_str());
    if (okay)
    {
        std::cerr << "Adding property<" << CGAL::demangle(typeid(Type).name()) << ">("
                  << name << ") as feature" << std::endl;
        m_features.template add<CGAL::Classification::Feature::Simple_feature <Point_set, Pmap> >
                (*(m_ptrPts), pmap, name.c_str());
    }
    return okay;
}
