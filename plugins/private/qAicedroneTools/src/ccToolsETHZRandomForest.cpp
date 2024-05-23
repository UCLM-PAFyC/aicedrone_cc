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

#include <QFIleInfo>
#include <QTime>
#include <QMessageBox>

//CCCoreLib
#include "ccProgressDialog.h"
#include "ccHObject.h"
#include <GeometricalAnalysisTools.h>
#include <ScalarFieldTools.h>

//qCC_db
#include <ccOctree.h>
#include <ccPointCloud.h>
#include <ccScalarField.h>
#include <GeometricalAnalysisTools.h>

#include <ccAdvancedTypes.h>

#include "../include/ccToolsETHZRandomForest.h"
#include "../include/ccToolsRandomForestClassificationDefinitions.h""
#include "libParameters/ParametersManager.h"
#include "libParameters/Parameter.h"

static QString GetDensitySFName(CCCoreLib::GeometricalAnalysisTools::Density densityType, bool approx, double densityKernelSize = 0.0)
{
    QString sfName;

    //update the name with the density type
    switch (densityType)
    {
        case CCCoreLib::GeometricalAnalysisTools::DENSITY_KNN:
            sfName = TOOLS_CC_LOCAL_KNN_DENSITY_FIELD_NAME;
            break;
        case CCCoreLib::GeometricalAnalysisTools::DENSITY_2D:
            sfName = TOOLS_CC_LOCAL_SURF_DENSITY_FIELD_NAME;
            break;
        case CCCoreLib::GeometricalAnalysisTools::DENSITY_3D:
            sfName = TOOLS_CC_LOCAL_VOL_DENSITY_FIELD_NAME;
            break;
        default:
            assert(false);
            break;
    }

    sfName += QString(" (r=%2)").arg(densityKernelSize);

    if (approx)
        sfName += " [approx]";

    return sfName;
}

ccToolsETHZRandomForest* ccToolsETHZRandomForest::m_instance = 0;

ccToolsETHZRandomForest::ccToolsETHZRandomForest()
{
    m_ptrCCPointCloud=NULL;
    m_ptrFormerCloudColors=NULL;
    m_ptrClassificationModel=NULL;
}

ccToolsETHZRandomForest::~ccToolsETHZRandomForest()
{
//    if(m_ptrEthzRFC!=NULL)
//    {
//        delete(m_ptrEthzRFC);
//        m_ptrEthzRFC = NULL;
//    }
//    if(m_ptrFeatureGenerator!=NULL)
//    {
//        delete(m_ptrFeatureGenerator);
//        m_ptrFeatureGenerator = NULL;
    //    }
}

bool ccToolsETHZRandomForest::initializeFromCCPointCloud(ccPointCloud *ptrCloud,
                                                         RGBAColorsTableType *ptrFormerCloudColors,
                                                         ccClassificationModel *ptrClassificationModel,
                                                         QString &strError)
{
    QString functionName="ccToolsETHZRandomForest::initializeFromCCPointCloud";
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
//    if(m_ptrPts!=NULL)
//    {
//        delete m_ptrPts;
//        m_ptrPts=NULL;
//    }
//    if(m_ptrFeatureGenerator!=NULL)
//    {
//        delete m_ptrFeatureGenerator;
//        m_ptrFeatureGenerator=NULL;
//    }
//    m_features.clear();
//    m_labels.clear();

//    if(m_ptrEthzRFC!=NULL)
//    {
//        delete m_ptrEthzRFC;
//        m_ptrEthzRFC=NULL;
//    }
//    m_labelProbabilities.clear();
    m_success=false;
    m_newClassByOriginalClass.clear();
    m_classLabelByOriginalClass.clear();
    m_originalClassByNewClass.clear();
    m_numberOfPointsByOriginalTrainingClass.clear();
//    m_training.clear();
    m_labelProbabilities.clear();
    m_features.clear();
    m_invalidPointsPositions.clear();
    m_ptrCCPointCloud=ptrCloud;
    m_ptrClassificationModel=ptrClassificationModel;
    m_ptrFormerCloudColors=ptrFormerCloudColors;
    return(true);
}

bool ccToolsETHZRandomForest::classify(ParametersManager *ptrParametersManager,
                                            QString method,
                                            QString targetFieldName,
                                            QString &timeInSeconds,
                                            QString &strReport,
                                            QString &strError)
{
    QString functionName="ccToolsETHZRandomForest::classify";
    strError.clear();
    timeInSeconds.clear();
    strReport.clear();
//    classifiedValues.clear();
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
//    if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0)
//    {
//        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_ONLY_UNLOCKED_CLASSES;
//    }
//    else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
//    {
//        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_ONLY_UNLOCKED_CLASSES;
//    }
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

    parameterCode=RFC_PROCESS_CLASSIFICATION_IGNORE_INVALID_POINTS_AND_CLASSIFY_AS_NOISE;
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    bool ignorePointsWithInvalidFeaturesAndClassifyAsNoise=true;
    if(strValue.compare("true",Qt::CaseInsensitive)!=0
            &&strValue.compare("false",Qt::CaseInsensitive)!=0)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
        return(false);
    }
    if(strValue.compare("false",Qt::CaseInsensitive)==0)
    {
        ignorePointsWithInvalidFeaturesAndClassifyAsNoise=false;
    }

    parameterCode=RFC_PROCESS_CLASSIFICATION_USE_TRAINING_FILE;
//    if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0)
//    {
//        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_USE_TRAINING_FILE;
//    }
//    else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
//    {
//        parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_USE_TRAINING_FILE;
//    }
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
    QStringList featuresNamesFromFile;
    unsigned cloudSize = m_ptrCCPointCloud->size();
    if(useTrainingFile)
    {
        parameterCode=RFC_PROCESS_CLASSIFICATION_TRAINING_FILE;
//        if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0)
//        {
//            parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING_TRAINING_FILE;
//        }
//        else if(method.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
//        {
//            parameterCode=RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT_TRAINING_FILE;
//        }
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
        QMap<int,int> newClassByOriginalClass_fromFile;
        QMap<int,QString> classLabelByOriginalClass_fromFile;
        QMap<int, int> originalClassByNewClass_fromFile;
        QDataStream auxiliarIn(&auxiliarFile);
        auxiliarIn>>newClassByOriginalClass_fromFile;
        auxiliarIn>>classLabelByOriginalClass_fromFile;
        auxiliarIn>>featuresNamesFromFile;
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
            iterNcBoC++;
        }
        m_newClassByOriginalClass=newClassByOriginalClass_fromFile;
        m_classLabelByOriginalClass=classLabelByOriginalClass_fromFile;
        m_originalClassByNewClass=originalClassByNewClass_fromFile;
        //        labelProbabilities_fromFile.resize (labels_fromFile.size());
        //        for (std::size_t i = 0; i < labelProbabilities_fromFile.size(); ++ i)
        //        {
        //            labelProbabilities_fromFile[i].resize (m_ptrPts->size(), -1);
        //        }
    }
    else
    {
        if(m_numberOfPointsByOriginalTrainingClass.size()!=m_originalClassByNewClass.size())
        {
            strError=functionName;
            strError+=QObject::tr("\nInvalid number of points by training class. Train before");
            return(false);
        }
    }
    if(m_newClassByOriginalClass.size()!=m_classLabelByOriginalClass.size())
    {
        strError=functionName;
        strError+=QObject::tr("\nInvalid number of points by training class. Train before");
        return(false);
    }
    m_labelProbabilities.clear();
    m_labelProbabilities.resize (m_originalClassByNewClass.size());
    for (std::size_t i = 0; i < m_labelProbabilities.size(); ++ i)
    {
        m_labelProbabilities[i].resize (cloudSize, -1);
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
            m_featuresStringsSelected=featuresNamesFromFile;
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
        for(int np=0;np<m_featuresStringsSelected.size();np++)
        {
            QString computedFeatureStr=m_featuresStringsSelected.at(np);
            if(featuresNamesFromFile.indexOf(computedFeatureStr)==-1)
            {
                strError=functionName;
                strError+=QObject::tr("\nComputed feature: %1 is not in traning file");
                return(false);
                // Cancel was clicked
            }
            if(featuresNamesFromFile.indexOf(computedFeatureStr)!=np)
            {
                strError=functionName;
                strError+=QObject::tr("\nComputed feature: %1 is in traning file in another position");
                return(false);
                // Cancel was clicked
            }
        }
        for(int np=0;np<featuresNamesFromFile.size();np++)
        {
            QString computedFeatureStr=featuresNamesFromFile.at(np);
            if(m_featuresStringsSelected.indexOf(computedFeatureStr)==-1)
            {
                strError=functionName;
                strError+=QObject::tr("\nImported feature from training file: %1 is not in computed features");
                return(false);
                // Cancel was clicked
            }
//            if(m_featuresStringsSelected.indexOf(computedFeatureStr)!=np)
//            {
//                strError=functionName;
//                strError+=QObject::tr("\nImported feature from training file: %1 is in computed features in another position");
//                return(false);
//                // Cancel was clicked
//            }
        }
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

    if(useTrainingFile)
    {
        bool reset_trees=true;
        if (m_ptrRandomForest && reset_trees)
            m_ptrRandomForest.reset();
        liblearning::RandomForest::ForestParams params;
        m_ptrRandomForest = std::make_shared<Forest> (params);
        std::ifstream fconfig (inputTrainingFileName.toStdString(), std::ios_base::binary);
        m_ptrRandomForest->read(fconfig);
    }
    QTime startTime=QTime::currentTime();
    std::vector<int> indices(cloudSize,-1);
    // classify
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        if(ignorePointsWithInvalidFeaturesAndClassifyAsNoise
                &&m_invalidPointsPositions.size()>0)
        {
            if(m_invalidPointsPositions.indexOf(i)!=-1)
            {
//                int classNoise=CC_CLASSIFICATION_MODEL_ASPRS_NOISE_CODE;
                int classNoise=m_ptrClassificationModel->getNoiseCode();
                indices[i]=classNoise;
                continue;
            }
        }
        std::vector<float> out;
        out.resize(m_classLabelByOriginalClass.size(),0.);
        std::vector<float> ft;
        ft.reserve (m_features.size());
        QMap<QString,QVector<double> >::const_iterator iterFeatures=m_features.begin();
        while(iterFeatures!=m_features.end())
        {
            ft.push_back(iterFeatures.value()[i]);
            iterFeatures++;
        }
        std::vector<float> prob (m_classLabelByOriginalClass.size());
        m_ptrRandomForest->evaluate (ft.data(), prob.data());
        for (std::size_t j = 0; j < out.size(); ++ j)
            out[j] = (std::min) (1.f, (std::max) (0.f, prob[j]));
        float val_class_best = 0.f;
        std::size_t nb_class_best=0;
        for(std::size_t k = 0; k < m_classLabelByOriginalClass.size(); ++ k)
        {
            m_labelProbabilities[k][i] = out[k];
            if(val_class_best < out[k])
            {
                val_class_best = out[k];
                nb_class_best = k;
            }
        }
        indices[i]=nb_class_best;
    }
    QTime endTime=QTime::currentTime();
//    timeInSeconds="Process time in seconds = ";
    double seconds=((double)endTime.msecsSinceStartOfDay()-(double)startTime.msecsSinceStartOfDay())/1000.;
    timeInSeconds=QString::number(seconds,'f',3);

    QMap<int,int> numberOfSuccessPointsByTrainingClass;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        if(ignorePointsWithInvalidFeaturesAndClassifyAsNoise
                &&m_invalidPointsPositions.size()>0)
        {
            if(m_invalidPointsPositions.indexOf(i)!=-1)
            {
//                ScalarType floatClassifiedValue = static_cast<ScalarType>(CC_CLASSIFICATION_MODEL_ASPRS_NOISE_CODE);
                ScalarType floatClassifiedValue = static_cast<ScalarType>(m_ptrClassificationModel->getNoiseCode());
                targetScalarField->setValue(i,floatClassifiedValue);
                continue;
            }
        }
        ScalarType targetFloatValue = targetScalarField->getValue(i);
        int targetOriginalClass=qRound(targetFloatValue);
        if(targetOriginalClass==CC_CLASSIFICATION_MODEL_REMOVED_CODE)
        {
            continue;
        }
        // consulto el item del campo escalar de entrenamiento porque es donde se indican
        // las clases a bloquear. El modelo es unico para entrenamiento y clasificacion
        ccClassificationModel::Item* ptrItem = m_ptrClassificationModel->find(targetOriginalClass);
        if(ptrItem->locked)
        {
            continue;
        }
        int newClass = indices[i];
        int classifiedValue=newClass;
        if(newClass!=-1)
        {
            if(m_originalClassByNewClass.contains(newClass))
            {
                classifiedValue=m_originalClassByNewClass[newClass];
            }
        }
        ScalarType floatClassifiedValue = static_cast<ScalarType>(classifiedValue);
        targetScalarField->setValue(i,floatClassifiedValue);
    }
    if(!useTrainingFile)
    {
        strReport+="Reliability results::\n";
        QMap<int,int>::const_iterator iter1=m_originalClassByNewClass.begin();
        while(iter1!=m_originalClassByNewClass.end())
        {
            int newClass=iter1.key();
            int originalClass=iter1.value();
            QString className=m_classLabelByOriginalClass[originalClass];
            int numberOfPoints=m_numberOfPointsByOriginalTrainingClass[originalClass];
            int numberOfSuccessPoints=numberOfSuccessPointsByTrainingClass[newClass];
            float reliabilityPercentage=100.*((float)numberOfSuccessPoints)/((float)numberOfPoints);
            strReport+= " * ";
            strReport+=className;
            strReport+=": ";
            strReport+=QString::number(reliabilityPercentage,'f',1);
            strReport+="\n";
            iter1++;
        }
    }
    return(true);
}

bool ccToolsETHZRandomForest::computeFeatures(ParametersManager *ptrParametersManager,
                                                   QStringList& featuresStringsFromTrainingFile,
                                                   QString &timeInSeconds,
                                                   QString &strReport,
                                                   QString &strError)
{
    m_features.clear();
    m_invalidPointsPositions.clear();
    QString functionName="ccToolsETHZRandomForest::computeFeatures";
    timeInSeconds.clear();
    strError.clear();
    strReport.clear();
    bool reportFeaturesValuesInDialog=false;
    if(m_ptrCCPointCloud==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nPoint cloud is NULL");
        return(false);
    }
    QString outputFilename;
    double localNeighborhoodRadius;
    bool computeRoughness=false;
    bool computeMomentOrder1=false;
    bool computeMeanCurvature=false;
    bool computeGaussianCurvature=false;
    bool computeNormalChangeRate=false;
    bool computeDensityKnn=false;
    bool computeDensity2d=false;
    bool computeDensity3d=false;
    bool computeEigenValuesSum=false;
    bool computeOmnivariance=false;
    bool computeEigenEntropy=false;
    bool computeAnisotropy=false;
    bool computePlanarity=false;
    bool computeLinearity=false;
    bool computePCA1=false;
    bool computePCA2=false;
    bool computeSurfaceVariation=false;
    bool computeSphericity=false;
    bool computeVerticality=false;
    bool computeEigenValue1=false;
    bool computeEigenValue2=false;
    bool computeEigenValue3=false;
    double roughnessDirectionFirst,roughnessDirectionSecond,roughnessDirectionThird;
    QString strValue;
    bool okToInt=false,okToDouble=false;
    bool useColor=false;
//    bool existsColors=m_ptrCCPointCloud->hasColors();
    bool existsColors=false;
    if(m_ptrFormerCloudColors) existsColors=true;
    if(featuresStringsFromTrainingFile.isEmpty())
    {
        if(!ptrParametersManager)
        {
            strError=functionName;
            strError+=QObject::tr("\nNull pointer for ParametersManager");
            return(false);
        }

        QString parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_LOCAL_NEIGHBORHOOD_RADIUS;
        Parameter* ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        localNeighborhoodRadius=strValue.toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not a double in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        if(localNeighborhoodRadius<0.001)
            localNeighborhoodRadius=GetDefaultCloudKernelSize(m_ptrCCPointCloud); // double radius

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_OUTPUT_FILE;
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

        useColor=false;
        if(existsColors)
        {
            parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_COLOR;
            ptrParameter=ptrParametersManager->getParameter(parameterCode);
            if(ptrParameter==NULL)
            {
                strError=functionName;
                strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                        .arg(parameterCode).arg(ptrParametersManager->getFileName());
                return(false);
            }
            ptrParameter->getValue(strValue);
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

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_ROUGHNESS;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeRoughness=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeRoughness=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_MOMENT_ORDER_1;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeMomentOrder1=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeMomentOrder1=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_MEAN_CURVATURE;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeMeanCurvature=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeMeanCurvature=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_GAUSSIAN_CURVATURE;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeGaussianCurvature=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeGaussianCurvature=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_NORMAL_CHANGE_RATE;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeNormalChangeRate=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeNormalChangeRate=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_DENSITY_KNN;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeDensityKnn=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeDensityKnn=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_DENSITY_2D;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeDensity2d=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeDensity2d=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_DENSITY_3D;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeDensity3d=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeDensity3d=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_VALUES_SUM;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeEigenValuesSum=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeEigenValuesSum=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_OMNIVARIANCE;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeOmnivariance=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeOmnivariance=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_ENTROPY;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeEigenEntropy=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeEigenEntropy=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_ANISOTROPY;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeAnisotropy=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeAnisotropy=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_PLANARITY;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computePlanarity=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computePlanarity=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_LINEARITY;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeLinearity=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeLinearity=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_PCA1;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computePCA1=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computePCA1=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_PCA2;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computePCA2=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computePCA2=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_SURFACE_VARIATION;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeSurfaceVariation=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeSurfaceVariation=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_SPHERICITY;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeSphericity=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeSphericity=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_VERTICALITY;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeVerticality=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeVerticality=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_VALUE_1;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeEigenValue1=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeEigenValue1=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_VALUE_2;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeEigenValue2=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeEigenValue2=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_EIGEN_VALUE_3;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        computeEigenValue3=false;
        if(strValue.compare("true",Qt::CaseInsensitive)!=0
                &&strValue.compare("false",Qt::CaseInsensitive)!=0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        if(strValue.compare("true",Qt::CaseInsensitive)==0)
        {
            computeEigenValue3=true;
        }

        parameterCode=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL_ROUGHNESS_DIRECTION;
        ptrParameter=ptrParametersManager->getParameter(parameterCode);
        if(ptrParameter==NULL)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                    .arg(parameterCode).arg(ptrParametersManager->getFileName());
            return(false);
        }
        ptrParameter->getValue(strValue);
        strValue=strValue.trimmed();
        strValue=strValue.remove("(");
        strValue=strValue.remove(")");
        QStringList strVectorComponents=strValue.split(',');
        if(strVectorComponents.size()!=3)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not a vector with three componentes, is:").arg(parameterCode).arg(strValue);
            return(false);
        }
        roughnessDirectionFirst=strVectorComponents[0].toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not double, is:").arg(parameterCode).arg(strVectorComponents[0]);
            return(false);
        }
        if(roughnessDirectionFirst>1||roughnessDirectionFirst<0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not double in range [0,1], is:").arg(parameterCode).arg(strVectorComponents[0]);
            return(false);
        }
        roughnessDirectionSecond=strVectorComponents[1].toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not double, is:").arg(parameterCode).arg(strVectorComponents[1]);
            return(false);
        }
        if(roughnessDirectionSecond>1||roughnessDirectionSecond<0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not double in range [0,1], is:").arg(parameterCode).arg(strVectorComponents[0]);
            return(false);
        }
        roughnessDirectionThird=strVectorComponents[2].toDouble(&okToDouble);
        if(!okToDouble)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not double, is:").arg(parameterCode).arg(strVectorComponents[1]);
            return(false);
        }
        if(roughnessDirectionThird>1||roughnessDirectionThird<0)
        {
            strError=functionName;
            strError+=QObject::tr("\nParameter: %1 is not double in range [0,1], is:").arg(parameterCode).arg(strVectorComponents[0]);
            return(false);
        }

        m_featuresStringsSelected.clear();
        QString parameterString="localNeighborhoodRadius="+QString::number(localNeighborhoodRadius,'f',9);
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeRoughness=";
        if(computeRoughness) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeMomentOrder1=";
        if(computeMomentOrder1) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeMeanCurvature=";
        if(computeMeanCurvature) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeGaussianCurvature=";
        if(computeGaussianCurvature) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeNormalChangeRate=";
        if(computeNormalChangeRate) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeDensityKnn=";
        if(computeDensityKnn) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeDensity2d=";
        if(computeDensity2d) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeDensity3d=";
        if(computeDensity3d) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeEigenValuesSum=";
        if(computeEigenValuesSum) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeOmnivariance=";
        if(computeOmnivariance) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeEigenEntropy=";
        if(computeEigenEntropy) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeAnisotropy=";
        if(computeAnisotropy) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computePlanarity=";
        if(computePlanarity) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeLinearity=";
        if(computeLinearity) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computePCA1=";
        if(computePCA1) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computePCA2=";
        if(computePCA2) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeSurfaceVariation=";
        if(computeSurfaceVariation) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeSphericity=";
        if(computeSphericity) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeVerticality=";
        if(computeVerticality) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeEigenValue1=";
        if(computeEigenValue1) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeEigenValue2=";
        if(computeEigenValue2) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="computeEigenValue3=";
        if(computeEigenValue3) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);

        parameterString="roughnessDirectionFirst="+QString::number(roughnessDirectionFirst,'f',9);
        m_featuresStringsSelected.append(parameterString);

        parameterString="roughnessDirectionSecond="+QString::number(roughnessDirectionSecond,'f',9);
        m_featuresStringsSelected.append(parameterString);

        parameterString="roughnessDirectionThird="+QString::number(roughnessDirectionThird,'f',9);
        m_featuresStringsSelected.append(parameterString);

//        if(existsColors&&useColor)
//        {
//            parameterString="useColor=";
//            if(useColor) parameterString+="true";
//            else parameterString+="false";
//            m_featuresStringsSelected.append(parameterString);
//        }
        // si se indica en el parametro usar colors pero la nube no tiene se queda a false
        parameterString="useColor=";
        if(useColor) parameterString+="true";
        else parameterString+="false";
        m_featuresStringsSelected.append(parameterString);
    }
    else
    {
        for(int np=0;np<featuresStringsFromTrainingFile.size();np++)
        {
            QString parameterStr=featuresStringsFromTrainingFile.at(np);
            QStringList strValues=parameterStr.split("=");
            if(strValues.size()!=2)
            {
                strError=functionName;
                strError+=QString("\nInvalid parameter from training file: %1").arg(parameterStr);
                return(false);
            }
            QString parameterName=strValues.at(0).trimmed();
            QString parameterValue=strValues.at(1).trimmed();
            bool parameterIsValid=false;
            if(parameterName.compare("localNeighborhoodRadius",Qt::CaseInsensitive)==0)
            {
                okToDouble=false;
                localNeighborhoodRadius=parameterValue.toDouble(&okToDouble);
                if(!okToDouble)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid double value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                parameterIsValid=true;
            }
            if(parameterName.compare("computeRoughness",Qt::CaseInsensitive)==0)
            {
                computeRoughness=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeRoughness=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeMomentOrder1",Qt::CaseInsensitive)==0)
            {
                computeMomentOrder1=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeMomentOrder1=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeMeanCurvature",Qt::CaseInsensitive)==0)
            {
                computeMeanCurvature=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeMeanCurvature=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeGaussianCurvature",Qt::CaseInsensitive)==0)
            {
                computeGaussianCurvature=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeGaussianCurvature=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeNormalChangeRate",Qt::CaseInsensitive)==0)
            {
                computeNormalChangeRate=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeNormalChangeRate=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeDensityKnn",Qt::CaseInsensitive)==0)
            {
                computeDensityKnn=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeDensityKnn=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeDensity2d",Qt::CaseInsensitive)==0)
            {
                computeDensity2d=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeDensity2d=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeDensity3d",Qt::CaseInsensitive)==0)
            {
                computeDensity3d=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeDensity3d=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeEigenValuesSum",Qt::CaseInsensitive)==0)
            {
                computeEigenValuesSum=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeEigenValuesSum=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeOmnivariance",Qt::CaseInsensitive)==0)
            {
                computeOmnivariance=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeOmnivariance=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeEigenEntropy",Qt::CaseInsensitive)==0)
            {
                computeEigenEntropy=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeEigenEntropy=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeAnisotropy",Qt::CaseInsensitive)==0)
            {
                computeAnisotropy=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeAnisotropy=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computePlanarity",Qt::CaseInsensitive)==0)
            {
                computePlanarity=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computePlanarity=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeLinearity",Qt::CaseInsensitive)==0)
            {
                computeLinearity=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeLinearity=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computePCA1",Qt::CaseInsensitive)==0)
            {
                computePCA1=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computePCA1=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computePCA2",Qt::CaseInsensitive)==0)
            {
                computePCA2=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computePCA2=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeSurfaceVariation",Qt::CaseInsensitive)==0)
            {
                computeSurfaceVariation=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeSurfaceVariation=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeSphericity",Qt::CaseInsensitive)==0)
            {
                computeSphericity=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeSphericity=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeVerticality",Qt::CaseInsensitive)==0)
            {
                computeVerticality=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeVerticality=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeEigenValue1",Qt::CaseInsensitive)==0)
            {
                computeEigenValue1=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeEigenValue1=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeEigenValue2",Qt::CaseInsensitive)==0)
            {
                computeEigenValue2=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeEigenValue2=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("computeEigenValue3",Qt::CaseInsensitive)==0)
            {
                computeEigenValue3=false;
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    computeEigenValue3=true;
                parameterIsValid=true;
            }
            if(parameterName.compare("roughnessDirectionFirst",Qt::CaseInsensitive)==0)
            {
                okToDouble=false;
                roughnessDirectionFirst=parameterValue.toDouble(&okToDouble);
                if(!okToDouble)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid double value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                parameterIsValid=true;
            }
            if(parameterName.compare("roughnessDirectionSecond",Qt::CaseInsensitive)==0)
            {
                okToDouble=false;
                roughnessDirectionSecond=parameterValue.toDouble(&okToDouble);
                if(!okToDouble)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid double value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                parameterIsValid=true;
            }
            if(parameterName.compare("roughnessDirectionThird",Qt::CaseInsensitive)==0)
            {
                okToDouble=false;
                roughnessDirectionThird=parameterValue.toDouble(&okToDouble);
                if(!okToDouble)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid double value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                parameterIsValid=true;
            }
            useColor=false;
            if(parameterName.compare("useColor",Qt::CaseInsensitive)==0)
            {
                if(parameterValue.compare("true",Qt::CaseInsensitive)!=0
                                          &&parameterValue.compare("false",Qt::CaseInsensitive)!=0)
                {
                    strError=functionName;
                    strError+=QString("\nInvalid boolean value in parameter from training file: %1").arg(parameterStr);
                    return(false);
                }
                if(parameterValue.compare("true",Qt::CaseInsensitive)==0)
                    useColor=true;
                parameterIsValid=true;
            }
            if(!parameterIsValid)
            {
                strError=functionName;
                strError+=QString("\nInvalid parameter from training file: %1").arg(parameterStr);
                return(false);
            }
        }
        m_featuresStringsSelected=featuresStringsFromTrainingFile;
    }

    if(m_featuresStringsSelected.size()<(RFC_PROCESS_COMPUTE_FEATURES_MINIMUM_NUMBER_OF_FEATURES+4))
    {
        strError=functionName;
        strError+=QString("\nNumber of selected features is lower than minimum: %1")
                .arg(QString::number(RFC_PROCESS_COMPUTE_FEATURES_MINIMUM_NUMBER_OF_FEATURES));
        return(false);
    }

    struct GeomCharacteristic
    {
        GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::GeomCharacteristic c, int option = 0)
            : charac(c)
            , subOption(option)
        {}

        CCCoreLib::GeometricalAnalysisTools::GeomCharacteristic charac;
        int subOption = 0;
    };
    typedef std::vector<GeomCharacteristic> GeomCharacteristicSet;
    GeomCharacteristicSet characteristics;
    {
        if(computeRoughness)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Roughness, 0));
        if(computeMomentOrder1)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::MomentOrder1, 0));
        if(computeMeanCurvature)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Curvature, CCCoreLib::Neighbourhood::MEAN_CURV));
        if(computeGaussianCurvature)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Curvature, CCCoreLib::Neighbourhood::GAUSSIAN_CURV));
        if(computeNormalChangeRate)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Curvature, CCCoreLib::Neighbourhood::NORMAL_CHANGE_RATE));
        if(computeDensityKnn)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::LocalDensity, CCCoreLib::GeometricalAnalysisTools::DENSITY_KNN));
        if(computeDensity2d)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::LocalDensity, CCCoreLib::GeometricalAnalysisTools::DENSITY_2D));
        if(computeDensity3d)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::LocalDensity, CCCoreLib::GeometricalAnalysisTools::DENSITY_3D));
        if(computeEigenValuesSum)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::EigenValuesSum));
        if(computeOmnivariance)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::Omnivariance));
        if(computeEigenEntropy)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::EigenEntropy));
        if(computeAnisotropy)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::Anisotropy));
        if(computePlanarity)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::Planarity));
        if(computeLinearity)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::Linearity));
        if(computePCA1)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::PCA1));
        if(computePCA2)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::PCA2));
        if(computeSurfaceVariation)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::SurfaceVariation));
        if(computeSphericity)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::Sphericity));
        if(computeVerticality)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::Verticality));
        if(computeEigenValue1)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::EigenValue1));
        if(computeEigenValue2)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::EigenValue2));
        if(computeEigenValue3)
            characteristics.push_back(GeomCharacteristic(CCCoreLib::GeometricalAnalysisTools::Feature, CCCoreLib::Neighbourhood::EigenValue3));
    }

    ccProgressDialog* pDlg=new ccProgressDialog(true);
    pDlg->setAutoClose(false);
    pDlg->show();
    static CCVector3 roughnessUpDir(roughnessDirectionFirst, roughnessDirectionSecond, roughnessDirectionThird);
//    const CCVector3* roughnessUpDir;
    ccPointCloud* ptrClonedPc=m_ptrCCPointCloud->cloneThis();
    PointCoordinateType radius=localNeighborhoodRadius;
    QTime startTime=QTime::currentTime();
    for (const GeomCharacteristic& g : characteristics)
    {
        CCCoreLib::GeometricalAnalysisTools::GeomCharacteristic c=g.charac;
        int subOption=g.subOption;
        QString sfName;
        switch (c)
        {
        case CCCoreLib::GeometricalAnalysisTools::Feature:
        {
            switch (subOption)
            {
            case CCCoreLib::Neighbourhood::EigenValuesSum:
                sfName = "Eigenvalues sum";
                break;
            case CCCoreLib::Neighbourhood::Omnivariance:
                sfName = "Omnivariance";
                break;
            case CCCoreLib::Neighbourhood::EigenEntropy:
                sfName = "Eigenentropy";
                break;
            case CCCoreLib::Neighbourhood::Anisotropy:
                sfName = "Anisotropy";
                break;
            case CCCoreLib::Neighbourhood::Planarity:
                sfName = "Planarity";
                break;
            case CCCoreLib::Neighbourhood::Linearity:
                sfName = "Linearity";
                break;
            case CCCoreLib::Neighbourhood::PCA1:
                sfName = "PCA1";
                break;
            case CCCoreLib::Neighbourhood::PCA2:
                sfName = "PCA2";
                break;
            case CCCoreLib::Neighbourhood::SurfaceVariation:
                sfName = "Surface variation";
                break;
            case CCCoreLib::Neighbourhood::Sphericity:
                sfName = "Sphericity";
                break;
            case CCCoreLib::Neighbourhood::Verticality:
                sfName = "Verticality";
                break;
            case CCCoreLib::Neighbourhood::EigenValue1:
                sfName = "1st eigenvalue";
                break;
            case CCCoreLib::Neighbourhood::EigenValue2:
                sfName = "2nd eigenvalue";
                break;
            case CCCoreLib::Neighbourhood::EigenValue3:
                sfName = "3rd eigenvalue";
                break;
            default:
//                assert(false);
//                ccLog::Error("Internal error: invalid sub option for Feature computation");
//                return false;
                strError=functionName;
                strError+=QString("\nInternal error: invalid sub option for Feature computation");
                delete pDlg;
                pDlg = nullptr;
                return(false);
            }

            sfName += QString(" (%1)").arg(radius);
        }
        break;

        case CCCoreLib::GeometricalAnalysisTools::Curvature:
        {
            switch (subOption)
            {
            case CCCoreLib::Neighbourhood::GAUSSIAN_CURV:
                sfName = TOOLS_CC_CURVATURE_GAUSSIAN_FIELD_NAME;
                break;
            case CCCoreLib::Neighbourhood::MEAN_CURV:
                sfName = TOOLS_CC_CURVATURE_MEAN_FIELD_NAME;
                break;
            case CCCoreLib::Neighbourhood::NORMAL_CHANGE_RATE:
                sfName = TOOLS_CC_CURVATURE_NORM_CHANGE_RATE_FIELD_NAME;
                break;
            default:
//                assert(false);
//                ccLog::Error("Internal error: invalid sub option for Curvature computation");
//                return false;
                strError=functionName;
                strError+=QString("\nInternal error: invalid sub option for Curvature computation");
                delete pDlg;
                pDlg = nullptr;
                return(false);
            }
            sfName += QString(" (%1)").arg(radius);
        }
        break;

        case CCCoreLib::GeometricalAnalysisTools::LocalDensity:
            sfName = GetDensitySFName(static_cast<CCCoreLib::GeometricalAnalysisTools::Density>(subOption), false, radius);
            break;

        case CCCoreLib::GeometricalAnalysisTools::ApproxLocalDensity:
            sfName = GetDensitySFName(static_cast<CCCoreLib::GeometricalAnalysisTools::Density>(subOption), true);
            break;

        case CCCoreLib::GeometricalAnalysisTools::Roughness:
            sfName = TOOLS_CC_ROUGHNESS_FIELD_NAME + QString(" (%1)").arg(radius);
            break;

        case CCCoreLib::GeometricalAnalysisTools::MomentOrder1:
            sfName = TOOLS_CC_MOMENT_ORDER1_FIELD_NAME + QString(" (%1)").arg(radius);
            break;

        default:
            assert(false);
            return false;
        }
        if(m_features.contains(sfName))
        {
            m_features.remove(sfName);
        }

        int sfIdx = -1;
        sfIdx = ptrClonedPc->getScalarFieldIndexByName(qPrintable(sfName));
        if (sfIdx < 0)
            sfIdx = ptrClonedPc->addScalarField(qPrintable(sfName));
        if (sfIdx >= 0)
            ptrClonedPc->setCurrentScalarField(sfIdx);
        else
        {
            strError=functionName;
            strError+=QString("\nFailed to create scalar field on cloud '%1' (not enough memory?)").arg(m_ptrCCPointCloud->getName());
            delete pDlg;
            pDlg = nullptr;
            return(false);
        }
        ccOctree::Shared octree = ptrClonedPc->getOctree();
        if (!octree)
        {
            if (pDlg)
            {
                pDlg->show();
            }
            octree = ptrClonedPc->computeOctree(pDlg);
            if (!octree)
            {
                strError=functionName;
                strError+=QString("\nCouldn't compute octree for cloud '%1'!").arg(m_ptrCCPointCloud->getName());
                delete pDlg;
                pDlg = nullptr;
                return(false);
            }
        }
        CCCoreLib::GeometricalAnalysisTools::ErrorCode result = CCCoreLib::GeometricalAnalysisTools::ComputeCharactersitic(	c,
                                                                                                                            subOption,
                                                                                                                            ptrClonedPc,
                                                                                                                            radius,
                                                                                                                            &roughnessUpDir,
                                                                                                                            pDlg,
                                                                                                                            octree.data());

        if (result == CCCoreLib::GeometricalAnalysisTools::NoError)
        {
            if (m_ptrCCPointCloud && sfIdx >= 0)
            {
                QVector<double> values(ptrClonedPc->size());
                CCCoreLib::ScalarField* sf = ptrClonedPc->getScalarField(sfIdx);
                int counter = 0;
                for (auto it = sf->begin(); it != sf->end(); ++it, ++counter)
                {
                    double value=sf->getValue(counter);
                    values[counter]=value;
                }
                m_features[sfName]=values;
                ptrClonedPc->deleteScalarField(sfIdx);
                sfIdx = -1;
            }
        }
        else
        {
            QString errorMessage;
            switch (result)
            {
            case CCCoreLib::GeometricalAnalysisTools::InvalidInput:
                errorMessage = "Internal error (invalid input)";
                break;
            case CCCoreLib::GeometricalAnalysisTools::NotEnoughPoints:
                errorMessage = "Not enough points";
                break;
            case CCCoreLib::GeometricalAnalysisTools::OctreeComputationFailed:
                errorMessage = "Failed to compute octree (not enough memory?)";
                break;
            case CCCoreLib::GeometricalAnalysisTools::ProcessFailed:
                errorMessage = "Process failed";
                break;
            case CCCoreLib::GeometricalAnalysisTools::UnhandledCharacteristic:
                errorMessage = "Internal error (unhandled characteristic)";
                break;
            case CCCoreLib::GeometricalAnalysisTools::NotEnoughMemory:
                errorMessage = "Not enough memory";
                break;
            case CCCoreLib::GeometricalAnalysisTools::ProcessCancelledByUser:
                errorMessage = "Process cancelled by user";
                break;
            default:
                assert(false);
                errorMessage = "Unknown error";
                break;
            }
            if (ptrClonedPc && sfIdx >= 0)
            {
                ptrClonedPc->deleteScalarField(sfIdx);
                sfIdx = -1;
            }
            strError=functionName;
            strError+=QString("\nFailed to apply processing to cloud '%1'").arg(ptrClonedPc->getName());
            strError+=QObject::tr("\nError:\n%1").arg(errorMessage);
            delete pDlg;
            pDlg = nullptr;
            return(false);
        }
    }
    delete(ptrClonedPc);
    delete pDlg;
    pDlg = nullptr;
    if(useColor)
    {
        if (existsColors)
        {
            QString sfNameColorR=TOOLS_CC_COLOR_RED_FIELD_NAME;
            if(m_features.contains(sfNameColorR))
            {
                m_features.remove(sfNameColorR);
            }
            QString sfNameColorG=TOOLS_CC_COLOR_GREEN_FIELD_NAME;
            if(m_features.contains(sfNameColorG))
            {
                m_features.remove(sfNameColorG);
            }
            QString sfNameColorB=TOOLS_CC_COLOR_BLUE_FIELD_NAME;
            if(m_features.contains(sfNameColorB))
            {
                m_features.remove(sfNameColorB);
            }
            unsigned pointCount = m_ptrCCPointCloud->size();
            QVector<double> redValues(pointCount);
            QVector<double> greenValues(pointCount);
            QVector<double> blueValues(pointCount);
            for (unsigned i = 0; i < pointCount; ++i)
            {
//                ccColor::Rgba color = m_ptrCCPointCloud->getPointColor(i);
                ccColor::Rgba color = m_ptrFormerCloudColors->getValue(i);
                double red=static_cast<double>(color.r) / ccColor::MAX;
                double green=static_cast<double>(color.g) / ccColor::MAX;
                double blue=static_cast<double>(color.b) / ccColor::MAX;
                redValues[i]=red;
                greenValues[i]=green;
                blueValues[i]=blue;
            }
            m_features[sfNameColorR]=redValues;
            m_features[sfNameColorG]=greenValues;
            m_features[sfNameColorB]=blueValues;
        }
        else
        {
            if(!featuresStringsFromTrainingFile.isEmpty())
            {
                strError=functionName;
                strError+=QObject::tr("\nExists colors in training data but this point cloud has no colors");
                return(false);
            }
        }
    }

    QTime endTime=QTime::currentTime();
//    timeInSeconds="Process time in seconds = ";
    double seconds=((double)endTime.msecsSinceStartOfDay()-(double)startTime.msecsSinceStartOfDay())/1000.;
    timeInSeconds=QString::number(seconds,'f',3);
    QFile* ptrFile;
    QTextStream* ptrOut;
    if(!outputFilename.isEmpty())
    {
        ptrFile= new QFile(outputFilename);
        if(!ptrFile->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening output file: %1").arg(outputFilename);
            return(false);
        }
        ptrOut=new QTextStream(ptrFile);
    }
    QMap<QString,QVector<double> >::const_iterator iterFeatures=m_features.begin();
    if(reportFeaturesValuesInDialog)
        strReport="Features report:\n";
    while(iterFeatures!=m_features.end())
    {
        QString featureName=iterFeatures.key();
        QVector<double> featureValues=iterFeatures.value();
        double mean=0.;
        double min=1000000.0;
        double max=-1000000.0;
        int cont=0;
        for(int i=0;i<featureValues.size();i++)
        {
            double value=featureValues[i];
            if(isnan(value))
            {
                if(m_invalidPointsPositions.indexOf(i)==-1)
                {
                    m_invalidPointsPositions.push_back(i);
                }
                continue;
            }
//            if(fabs(value)>10000.0)
//            {
//                continue;
//            }
            mean+=value;
            if(value<min) min=value;
            if(value>max) max=value;
            cont++;
        }
        mean/=cont;
        if(!outputFilename.isEmpty())
        {
            (*ptrOut)<<featureName<<" in "<<"[";
            (*ptrOut)<<QString::number(min,'f',9);
            (*ptrOut)<<";";
            (*ptrOut)<<QString::number(max,'f',9);
            (*ptrOut)<<"], mean = ";
            (*ptrOut)<<QString::number(mean,'f',9);
            (*ptrOut)<<", number of NaN values = "<<QString::number(featureValues.size()-cont);
            (*ptrOut)<<"\n";
        }
        if(reportFeaturesValuesInDialog)
        {
            strReport+=featureName;
            strReport+=" in ";
            strReport+="[";
            strReport+=QString::number(min,'f',9);
            strReport+=";";
            strReport+=QString::number(max,'f',9);
            strReport+="], mean = ";
            strReport+=QString::number(mean,'f',9);
            strReport+=", number of NaN values = ";
            strReport+=QString::number(featureValues.size()-cont);
            strReport+="\n";
        }
        iterFeatures++;
    }
    if(reportFeaturesValuesInDialog)
    {
        strReport+="Number of invalid points = ";
        strReport+=QString::number(m_invalidPointsPositions.size());
        strReport+="\n";
    }
    if(!outputFilename.isEmpty())
    {
        (*ptrOut)<<"Number of invalid points = "<<QString::number(m_invalidPointsPositions.size())<<"\n";
        ptrFile->close();
        delete(ptrOut);
        delete(ptrFile);
    }
    int newPointPosition=0;
    for(int i=0;i<m_ptrCCPointCloud->size();i++)
    {
        if(m_invalidPointsPositions.indexOf(i)!=-1) continue;
        newPointPosition++;
    }
    return(true);
}

bool ccToolsETHZRandomForest::train(ParametersManager *ptrParametersManager,
                                         QString trainFieldName,
                                         QString targetFieldName,
                                         QString &timeInSeconds,
                                         QString &strReport,
                                         QString &strError)
{
    QString functionName="ccToolsETHZRandomForest::train";
    strError.clear();
    timeInSeconds.clear();
    strReport.clear();
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

    parameterCode=RFC_PROCESS_TRAINING_PROCESS_IGNORE_INVALID_POINTS_AND_CLASSIFY_AS_NOISE;
    ptrParameter=ptrParametersManager->getParameter(parameterCode);
    if(ptrParameter==NULL)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 not found in file:\n%2")
                .arg(parameterCode).arg(ptrParametersManager->getFileName());
        return(false);
    }
    ptrParameter->getValue(strValue);
    bool ignorePointsWithInvalidFeaturesAndClassifyAsNoise=true;
    if(strValue.compare("true",Qt::CaseInsensitive)!=0
            &&strValue.compare("false",Qt::CaseInsensitive)!=0)
    {
        strError=functionName;
        strError+=QObject::tr("\nParameter: %1 is not true or false, is:").arg(parameterCode).arg(strValue);
        return(false);
    }
    if(strValue.compare("false",Qt::CaseInsensitive)==0)
    {
        ignorePointsWithInvalidFeaturesAndClassifyAsNoise=false;
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

    QVector<int> originalValues; // contiene todos los puntos
    QVector<int> originalValuesToClassify; // contiene las clases a clasificar para los puntos validos
    m_newClassByOriginalClass.clear();
    m_classLabelByOriginalClass.clear();
    m_numberOfPointsByOriginalTrainingClass.clear();
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
            originalValuesToClassify.push_back(RFC_PROCESS_CLASSIFICATION_NO_VALUE);
            continue;
        }
        if(ignorePointsWithInvalidFeaturesAndClassifyAsNoise
                &&m_invalidPointsPositions.size()>0)
        {
            if(m_invalidPointsPositions.indexOf(i)!=-1)
            {
                originalValuesToClassify.push_back(RFC_PROCESS_CLASSIFICATION_NO_VALUE);
                continue;
            }
        }
        originalValuesToClassify.push_back(originalClass);
        QString classLabel=ptrItem->name;
        if(!m_newClassByOriginalClass.contains(originalClass))
        {
            m_newClassByOriginalClass[originalClass]=RFC_PROCESS_CLASSIFICATION_NO_VALUE;
        }
        if(!m_classLabelByOriginalClass.contains(originalClass))
        {
            m_classLabelByOriginalClass[originalClass]=classLabel;
        }
        int newClass=m_newClassByOriginalClass[originalClass];
        if(!m_numberOfPointsByOriginalTrainingClass.contains(originalClass))
        {
            m_numberOfPointsByOriginalTrainingClass[originalClass]=0;
        }
        m_numberOfPointsByOriginalTrainingClass[originalClass]=m_numberOfPointsByOriginalTrainingClass[originalClass]+1;
    }
    QMap<int,int>::const_iterator iterNpTc=m_numberOfPointsByOriginalTrainingClass.begin();// newClass
    while(iterNpTc!=m_numberOfPointsByOriginalTrainingClass.end())
    {
        int originalClass=iterNpTc.key();
        int nop=iterNpTc.value();
        iterNpTc++;
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
    m_labelProbabilities.clear();
    while(iterClasses!=m_newClassByOriginalClass.end())
    {
        int originalClass=iterClasses.key();
        int newClass=cont;
        m_newClassByOriginalClass[originalClass]=newClass;
        m_originalClassByNewClass[newClass]=originalClass;
        QString qsClassLabel=m_classLabelByOriginalClass[originalClass];
        std::string stdClassLabel=qsClassLabel.toStdString();
        const char* charClassLabel = stdClassLabel.c_str();
        cont++;
        iterClasses++;
    }

    m_labelProbabilities.resize (m_newClassByOriginalClass.size());
    for (std::size_t i = 0; i < m_labelProbabilities.size(); ++ i)
        m_labelProbabilities[i].resize (cloudSize, -1);

    liblearning::RandomForest::ForestParams params;
    params.n_trees   = numberOfTrees;
    params.max_depth = maximumDeep;

    std::vector<int> gt;
    std::vector<float> ft;

    for(int i=0;i<originalValuesToClassify.size();i++)
    {
        int originalValue=originalValuesToClassify[i];
        if(originalValue==RFC_PROCESS_CLASSIFICATION_NO_VALUE)
            continue;
        QMap<QString,QVector<double> >::const_iterator iterFeatures=m_features.begin();
        while(iterFeatures!=m_features.end())
        {
            ft.push_back(iterFeatures.value()[i]);
            iterFeatures++;
        }
        int newClass=m_newClassByOriginalClass[originalValue];
        gt.push_back(newClass);
    }
    bool reset_trees=true;
    if (m_ptrRandomForest && reset_trees)
      m_ptrRandomForest.reset();

    if (!m_ptrRandomForest)
      m_ptrRandomForest = std::make_shared<Forest> (params);

    liblearning::DataView2D<int> label_vector (&(gt[0]), gt.size(), 1);
    liblearning::DataView2D<float> feature_vector(&(ft[0]), gt.size(), ft.size() / gt.size());
    liblearning::RandomForest::AxisAlignedRandomSplitGenerator generator;

    QTime startTime=QTime::currentTime();
    m_ptrRandomForest->train(feature_vector,
                             label_vector,
                             liblearning::DataView2D<int>(),
                             generator,
                             0,
//                             reset_trees,
                             m_classLabelByOriginalClass.size());
    std::vector<int> indices(cloudSize,-1);
    // classify
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        if(ignorePointsWithInvalidFeaturesAndClassifyAsNoise
                &&m_invalidPointsPositions.size()>0)
        {
            if(m_invalidPointsPositions.indexOf(i)!=-1)
            {
//                int classNoise=CC_CLASSIFICATION_MODEL_ASPRS_NOISE_CODE;
                int classNoise=m_ptrClassificationModel->getNoiseCode();
                indices[i]=classNoise;
                continue;
            }
        }
        std::vector<float> out;
        out.resize(m_classLabelByOriginalClass.size(),0.);
        std::vector<float> ft;
        ft.reserve (m_features.size());
        QMap<QString,QVector<double> >::const_iterator iterFeatures=m_features.begin();
        while(iterFeatures!=m_features.end())
        {
            ft.push_back(iterFeatures.value()[i]);
            iterFeatures++;
        }
        std::vector<float> prob (m_classLabelByOriginalClass.size());
        m_ptrRandomForest->evaluate (ft.data(), prob.data());
        for (std::size_t j = 0; j < out.size(); ++ j)
            out[j] = (std::min) (1.f, (std::max) (0.f, prob[j]));
        float val_class_best = 0.f;
        std::size_t nb_class_best=0;
        for(std::size_t k = 0; k < m_classLabelByOriginalClass.size(); ++ k)
        {
            m_labelProbabilities[k][i] = out[k];
            if(val_class_best < out[k])
            {
                val_class_best = out[k];
                nb_class_best = k;
            }
        }
        indices[i]=nb_class_best;
    }
    QTime endTime=QTime::currentTime();
//    timeInSeconds="Process time in seconds = ";
    double seconds=((double)endTime.msecsSinceStartOfDay()-(double)startTime.msecsSinceStartOfDay())/1000.;
    timeInSeconds=QString::number(seconds,'f',3);

    QMap<int,int> numberOfSuccessPointsByTrainingClass;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        if(ignorePointsWithInvalidFeaturesAndClassifyAsNoise
                &&m_invalidPointsPositions.size()>0)
        {
            if(m_invalidPointsPositions.indexOf(i)!=-1)
            {
//                ScalarType floatClassifiedValue = static_cast<ScalarType>(CC_CLASSIFICATION_MODEL_ASPRS_NOISE_CODE);
                ScalarType floatClassifiedValue = static_cast<ScalarType>(m_ptrClassificationModel->getNoiseCode());
                targetScalarField->setValue(i,floatClassifiedValue);
                continue;
            }
        }
        ScalarType targetFloatValue = targetScalarField->getValue(i);
        int targetOriginalClass=qRound(targetFloatValue);
        if(targetOriginalClass==CC_CLASSIFICATION_MODEL_REMOVED_CODE)
        {
            continue;
        }
        // consulto el item del campo escalar de entrenamiento porque es donde se indican
        // las clases a bloquear. El modelo es unico para entrenamiento y clasificacion
        ccClassificationModel::Item* ptrItem = m_ptrClassificationModel->find(targetOriginalClass);
        if(ptrItem->locked)
        {
            continue;
        }
        int originalTrainingClass = originalValues[i];
        int newClass = indices[i];
        if(m_newClassByOriginalClass.contains(originalTrainingClass))
        {
            int trainingNewClass=m_newClassByOriginalClass[originalTrainingClass];
            if(trainingNewClass !=-1)
            {
                if(newClass==trainingNewClass)
                {
                    if(!numberOfSuccessPointsByTrainingClass.contains(trainingNewClass))
                    {
                        numberOfSuccessPointsByTrainingClass[trainingNewClass]=0;
                    }
                    numberOfSuccessPointsByTrainingClass[trainingNewClass]=numberOfSuccessPointsByTrainingClass[trainingNewClass]+1;
                }
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
        ScalarType floatClassifiedValue = static_cast<ScalarType>(classifiedValue);
        targetScalarField->setValue(i,floatClassifiedValue);
    }
    strReport+="Reliability results::\n";
    QMap<int,int>::const_iterator iter1=m_originalClassByNewClass.begin();
    while(iter1!=m_originalClassByNewClass.end())
    {
        int newClass=iter1.key();
        int originalClass=iter1.value();
        QString className=m_classLabelByOriginalClass[originalClass];
        int numberOfPoints=m_numberOfPointsByOriginalTrainingClass[originalClass];
        int numberOfSuccessPoints=numberOfSuccessPointsByTrainingClass[newClass];
        float reliabilityPercentage=100.*((float)numberOfSuccessPoints)/((float)numberOfPoints);
        strReport+= " * ";
        strReport+=className;
        strReport+=": ";
        strReport+=QString::number(reliabilityPercentage,'f',1);
        strReport+="\n";
        iter1++;
    }
    if(!output_training_filename.isEmpty())
    {
        std::string stdOutputTrainingFileName=output_training_filename.toStdString();
        std::ofstream outputTrainingFile (stdOutputTrainingFileName,
                                          std::ios_base::binary);
        m_ptrRandomForest->write(outputTrainingFile);
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
        auxiliarOut<<m_featuresStringsSelected;
//        auxiliarOut<<m_originalClassByNewClass;
        auxiliarFile.close();
    }
    return(true);
}

bool ccToolsETHZRandomForest::trainingClassesSeparabiltyAnalysis(ParametersManager *ptrParametersManager,
                                                                 QString trainFieldName,
                                                                 QString &timeInSeconds,
                                                                 QString &strReport,
                                                                 QString &strError)
{
    QString functionName="ccToolsETHZRandomForest::trainingClassesSeparabiltyAnalysis";
    strError.clear();
    timeInSeconds.clear();
    strReport.clear();
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
    QMap<QString,QMap<QString,QVector<double> > > featuresByClassByParameter;
    QTime startTime=QTime::currentTime();
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
        if(!featuresByClassByParameter.contains(classLabel))
        {
            QMap<QString,QVector<double> > aux;
            featuresByClassByParameter[classLabel]=aux;
        }
        QMap<QString,QVector<double> >::const_iterator iterByParameter=m_features.begin();
        while(iterByParameter!=m_features.end())
        {
            QString parameterName=iterByParameter.key();
            if(i>(m_features[parameterName].size()-1))
            {
                iterByParameter++;
                continue;
            }
            double parameterValue=m_features[parameterName][i];
            if(isnan(parameterValue))
            {
                iterByParameter++;
                continue;
            }
            if(!featuresByClassByParameter[classLabel].contains(parameterName))
            {
                QVector<double> aux;
                featuresByClassByParameter[classLabel][parameterName]=aux;
            }
            featuresByClassByParameter[classLabel][parameterName].push_back(parameterValue);
            iterByParameter++;
        }
    }
    QMap<QString,double> meanValueByParameter;
    QMap<QString,double> stdValueByParameter;
    QMap<QString,double> minValueByParameter;
    QMap<QString,double> maxValueByParameter;
    if(normalizeParameters)
    {
        QMap<QString,QVector<double> >::const_iterator iterByParameter=m_features.begin();
        while(iterByParameter!=m_features.end())
        {
            QString parameterName=iterByParameter.key();
            QVector<double> values=iterByParameter.value();
            double meanValue=0.;
            double minValue=10000000.0;
            double maxValue=-10000000.0;
            int cont=0;
            for(int i=0;i<values.size();i++)
            {
                double parameterValue=values[i];
                if(isnan(parameterValue))
                {
                    continue;
                }
                meanValue+=parameterValue;
                if(parameterValue<=minValue) minValue=parameterValue;
                if(parameterValue>=maxValue) maxValue=parameterValue;
                cont++;
            }
            meanValue/=((double)cont);
            double stdValue=0.;
            if(cont<2) stdValue=-1.;
            for(int i=0;i<values.size();i++)
            {
                double parameterValue=values[i];
                if(isnan(parameterValue))
                {
                    continue;
                }
                stdValue+=pow(parameterValue-meanValue,2.0);
                cont++;
            }
            if(cont>1)
                stdValue=sqrt(stdValue/((double)(cont-1)));
            meanValueByParameter[parameterName]=meanValue;
            stdValueByParameter[parameterName]=stdValue;
            minValueByParameter[parameterName]=minValue;
            maxValueByParameter[parameterName]=maxValue;
            iterByParameter++;
        }
    }
    QMap<QString,QMap<QString,double> > meanValueByParameterByClass;
    QMap<QString,QMap<QString,double> > stdValueByParameterByClass;
    QMap<QString,QMap<QString,double> > minValueByParameterByClass;
    QMap<QString,QMap<QString,double> > maxValueByParameterByClass;
    QString outputFileHeader;
    QString outputFileAllHeader;
    QString outputFileAllHeaderWithOutStd;
    QMap<QString,QMap<QString,QVector<double> > >::const_iterator iterByClass=featuresByClassByParameter.begin();
    while(iterByClass!=featuresByClassByParameter.end())
    {
        QString className=iterByClass.key();
        className=className.remove(' ');
        if(outputFileHeader.isEmpty())
        {
            outputFileHeader+="Parameter";
        }
        if(outputFileAllHeader.isEmpty())
        {
            outputFileAllHeader+="Parameter";
            outputFileAllHeaderWithOutStd+="Parameter";
        }
        outputFileHeader+=";";
        outputFileHeader+=className;
        outputFileAllHeader+=";";
        outputFileAllHeader+=(className+"_mean");
        outputFileAllHeader+=";";
        outputFileAllHeader+=(className+"_std");
        outputFileAllHeader+=";";
        outputFileAllHeader+=(className+"_min");
        outputFileAllHeader+=";";
        outputFileAllHeader+=(className+"_max");
        outputFileAllHeaderWithOutStd+=";";
        outputFileAllHeaderWithOutStd+=(className+"_mean");
//        outputFileAllHeaderWithOutStd+=";";
//        outputFileAllHeaderWithOutStd+=(className+"_std");
        outputFileAllHeaderWithOutStd+=";";
        outputFileAllHeaderWithOutStd+=(className+"_min");
        outputFileAllHeaderWithOutStd+=";";
        outputFileAllHeaderWithOutStd+=(className+"_max");
        QMap<QString,QVector<double> >::const_iterator iterByParameter=featuresByClassByParameter[className].begin();
        QMap<QString,double> meanValueByClass;
        QMap<QString,double> stdValueByClass;
        QMap<QString,double> minValueByClass;
        QMap<QString,double> maxValueByClass;
        while(iterByParameter!=featuresByClassByParameter[className].end())
        {
            QString parameterName=iterByParameter.key();
            QVector<double> values=featuresByClassByParameter[className][parameterName];
            double meanValue=0.;
            double stdValue=0.;
            double minValue=10000000000.;
            double maxValue=-10000000000.;
            for(int i=0;i<values.size();i++)
            {
                meanValue+=values[i];
                if(values[i]<=minValue) minValue=values[i];
                if(values[i]>=maxValue) maxValue=values[i];
            }
            meanValue/=(double(values.size()));
            if(values.size()>1)
            {
                for(int i=0;i<values.size();i++)
                {
                    stdValue+=pow(meanValue-values[i],2.0);
                }
                stdValue=sqrt(stdValue/(double(values.size()-1)));
            }
            else
                stdValue=-1.;
            meanValueByClass[className]=meanValue;
            stdValueByClass[className]=stdValue;
            minValueByClass[className]=minValue;
            maxValueByClass[className]=maxValue;
            meanValueByParameterByClass[parameterName][className]=meanValueByClass[className];
            stdValueByParameterByClass[parameterName][className]=stdValueByClass[className];
            minValueByParameterByClass[parameterName][className]=minValueByClass[className];
            maxValueByParameterByClass[parameterName][className]=maxValueByClass[className];
            iterByParameter++;
        }
        iterByClass++;
    }
    QFile outputFile(outputFilename);
    if(!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nError opening output file: %1").arg(outputFilename);
        return(false);
    }
    QTextStream out(&outputFile);
    QFileInfo outputFileInfo(outputFilename);
    QString outputPath=outputFileInfo.absolutePath();
    QString outputFilenameMean=outputPath+"/"+outputFileInfo.completeBaseName()+"_mean.csv";
    QString outputFilenameStd=outputPath+"/"+outputFileInfo.completeBaseName()+"_std.csv";
    QString outputFilenameMin=outputPath+"/"+outputFileInfo.completeBaseName()+"_min.csv";
    QString outputFilenameMax=outputPath+"/"+outputFileInfo.completeBaseName()+"_max.csv";
    QFile outputFileMean(outputFilenameMean);
    QFile outputFileStd(outputFilenameStd);
    QFile outputFileMin(outputFilenameMin);
    QFile outputFileMax(outputFilenameMax);
    if(!outputFileMean.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nError opening output mean values file: %1").arg(outputFilenameMean);
        return(false);
    }
    QTextStream outMean(&outputFileMean);
    if(!outputFileStd.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nError opening output std values file: %1").arg(outputFilenameStd);
        return(false);
    }
    QTextStream outStd(&outputFileStd);
    if(!outputFileMin.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nError opening output minimum values file: %1").arg(outputFilenameMin);
        return(false);
    }
    QTextStream outMin(&outputFileMin);
    if(!outputFileMax.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        strError=functionName;
        strError+=QObject::tr("\nError opening output maximum values file: %1").arg(outputFilenameMax);
        return(false);
    }
    QTextStream outMax(&outputFileMax);
    out<<outputFileAllHeader<<"\n";
    outMean<<outputFileHeader<<"\n";
    outStd<<outputFileHeader<<"\n";
    outMin<<outputFileHeader<<"\n";
    outMax<<outputFileHeader<<"\n";
    QTextStream* ptrOutNorm,*ptrOutMeanNorm,*ptrOutStdNorm,*ptrOutMinNorm,*ptrOutMaxNorm;
    QFile* ptrOutputFileNorm,*ptrOutputFileMeanNorm,*ptrOutputFileStdNorm,*ptrOutputFileMinNorm,*ptrOutputFileMaxNorm;
    if(normalizeParameters)
    {
        QString outputFilenameNorm=outputPath+"/"+outputFileInfo.completeBaseName()+"_norm.csv";
        QString outputFilenameMeanNorm=outputPath+"/"+outputFileInfo.completeBaseName()+"_mean_norm.csv";
//        QString outputFilenameStdNorm=outputPath+"/"+outputFileInfo.completeBaseName()+"_std_norm.csv";
        QString outputFilenameMinNorm=outputPath+"/"+outputFileInfo.completeBaseName()+"_min_norm.csv";
        QString outputFilenameMaxNorm=outputPath+"/"+outputFileInfo.completeBaseName()+"_max_norm.csv";
        ptrOutputFileNorm=new QFile(outputFilenameNorm);
        if(!ptrOutputFileNorm->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening output mean values file: %1").arg(outputFilenameNorm);
            return(false);
        }
        ptrOutNorm=new QTextStream(ptrOutputFileNorm);
        ptrOutputFileMeanNorm=new QFile(outputFilenameMeanNorm);
        if(!ptrOutputFileMeanNorm->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening output mean values file: %1").arg(outputFilenameMeanNorm);
            return(false);
        }
        ptrOutMeanNorm=new QTextStream(ptrOutputFileMeanNorm);
//        ptrOutputFileStdNorm=new QFile(outputFilenameStdNorm);
//        if(!ptrOutputFileStdNorm->open(QIODevice::WriteOnly | QIODevice::Text))
//        {
//            strError=functionName;
//            strError+=QObject::tr("\nError opening output std values file: %1").arg(outputFilenameStdNorm);
//            return(false);
//        }
//        ptrOutStdNorm=new QTextStream(ptrOutputFileStdNorm);
        ptrOutputFileMinNorm=new QFile(outputFilenameMinNorm);
        if(!ptrOutputFileMinNorm->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening output minimum values file: %1").arg(outputFilenameMinNorm);
            return(false);
        }
        ptrOutMinNorm=new QTextStream(ptrOutputFileMinNorm);
        ptrOutputFileMaxNorm=new QFile(outputFilenameMaxNorm);
        if(!ptrOutputFileMaxNorm->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            strError=functionName;
            strError+=QObject::tr("\nError opening output maximum values file: %1").arg(outputFilenameMaxNorm);
            return(false);
        }
        ptrOutMaxNorm=new QTextStream(ptrOutputFileMaxNorm);
        (*ptrOutNorm)<<outputFileAllHeaderWithOutStd<<"\n";
        (*ptrOutMeanNorm)<<outputFileHeader<<"\n";
//        (*ptrOutStdNorm)<<outputFileHeader<<"\n";
        (*ptrOutMinNorm)<<outputFileHeader<<"\n";
        (*ptrOutMaxNorm)<<outputFileHeader<<"\n";
    }
    QMap<QString,QMap<QString,double> >::const_iterator iterByParameter=meanValueByParameterByClass.begin();
    while(iterByParameter!=meanValueByParameterByClass.end())
    {
        QString parameterName=iterByParameter.key();
        out<<parameterName;
        outMean<<parameterName;
        outStd<<parameterName;
        outMin<<parameterName;
        outMax<<parameterName;
        if(normalizeParameters)
        {
            (*ptrOutNorm)<<parameterName;
            (*ptrOutMeanNorm)<<parameterName;
//            (*ptrOutStdNorm)<<parameterName;
            (*ptrOutMinNorm)<<parameterName;
            (*ptrOutMaxNorm)<<parameterName;
        }
        QMap<QString,double>::const_iterator iterByClass=iterByParameter.value().begin();
        while(iterByClass!=iterByParameter.value().end())
        {
            QString className=iterByClass.key();
            double meanValue=meanValueByParameterByClass[parameterName][className];
            double stdValue=stdValueByParameterByClass[parameterName][className];
            double minValue=minValueByParameterByClass[parameterName][className];
            double maxValue=maxValueByParameterByClass[parameterName][className];
            QString strMeanValue=QString::number(meanValue,'f',15);
            QString strStdValue=QString::number(stdValue,'f',15);
            QString strMinValue=QString::number(minValue,'f',15);
            QString strMaxValue=QString::number(maxValue,'f',15);
            out<<";"<<strMeanValue<<";"<<strStdValue<<";"<<strMinValue<<";"<<strMaxValue;
            outMean<<";"<<strMeanValue;
            outStd<<";"<<strStdValue;
            outMin<<";"<<strMinValue;
            outMax<<";"<<strMaxValue;
            if(normalizeParameters)
            {
                double parameterMeanValue=meanValueByParameter[parameterName];
                double parameterStdValue=stdValueByParameter[parameterName];
                double parameterMinValue=minValueByParameter[parameterName];
                double parameterMaxValue=maxValueByParameter[parameterName];
                QString strMeanValueNorm=QString::number((meanValue-parameterMeanValue)/parameterStdValue,'f',15);
//                QString strStdValueNorm=QString::number(stdValue,'f',15);
                QString strMinValueNorm=QString::number((minValue-parameterMeanValue)/parameterStdValue,'f',15);
                QString strMaxValueNorm=QString::number((maxValue-parameterMeanValue)/parameterStdValue,'f',15);
//                (*ptrOutNorm)<<";"<<strMeanValueNorm<<";"<<strStdValueNorm<<";"<<strMinValueNorm<<";"<<strMaxValueNorm;
                (*ptrOutNorm)<<";"<<strMeanValueNorm<<";"<<strMinValueNorm<<";"<<strMaxValueNorm;
                (*ptrOutMeanNorm)<<";"<<strMeanValueNorm;
//                (*ptrOutStdNorm)<<";"<<strStdValueNorm;
                (*ptrOutMinNorm)<<";"<<strMinValueNorm;
                (*ptrOutMaxNorm)<<";"<<strMaxValueNorm;
            }
            iterByClass++;
        }
        outMean<<"\n";
        outStd<<"\n";
        outMin<<"\n";
        outMax<<"\n";
        out<<"\n";
        if(normalizeParameters)
        {
            (*ptrOutNorm)<<"\n";
            (*ptrOutMeanNorm)<<"\n";
//                (*ptrOutStdNorm)<<"\n";
            (*ptrOutMinNorm)<<"\n";
            (*ptrOutMaxNorm)<<"\n";
        }
        iterByParameter++;
    }
    outputFile.close();
    outputFileMean.close();
    outputFileStd.close();
    outputFileMin.close();
    outputFileMax.close();
    if(normalizeParameters)
    {
        ptrOutputFileNorm->close();
        ptrOutputFileMeanNorm->close();
//        ptrOutputFileStdNorm->close();
        ptrOutputFileMinNorm->close();
        ptrOutputFileMaxNorm->close();
    }
    QTime endTime=QTime::currentTime();
//    timeInSeconds="Process time in seconds = ";
    double seconds=((double)endTime.msecsSinceStartOfDay()-(double)startTime.msecsSinceStartOfDay())/1000.;
    timeInSeconds=QString::number(seconds,'f',3);
    return(true);
}

// from ccLibAlgoritms qCC
PointCoordinateType ccToolsETHZRandomForest::GetDefaultCloudKernelSize(ccGenericPointCloud *cloud, unsigned knn/*=12*/)
{
    assert(cloud);
    if (cloud && cloud->size() != 0)
    {
        //we get 1% of the cloud bounding box
        //and we divide by the number of points / 10e6 (so that the kernel for a 20 M. points cloud is half the one of a 10 M. cloud)
        ccBBox box = cloud->getOwnBB();

        //old way
        //PointCoordinateType radius = box.getDiagNorm() * static_cast<PointCoordinateType>(0.01/std::max(1.0,1.0e-7*static_cast<double>(cloud->size())));

        //new way
        CCVector3 d = box.getDiagVec();
        PointCoordinateType volume = d[0] * d[1] * d[2];
        PointCoordinateType surface = pow(volume, static_cast<PointCoordinateType>(2.0/3.0));
        PointCoordinateType surfacePerPoint = surface / cloud->size();
        return sqrt(surfacePerPoint * knn);
    }

    return -CCCoreLib::PC_ONE;
}

