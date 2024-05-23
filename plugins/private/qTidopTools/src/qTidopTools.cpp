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

#include "../include/qTidopTools.h"
#include "../include/ccToolsManualClassificationDlg.h"
#include "../include/ccToolsRandomForestClassificationDlg.h"

//local
// #include "qCanupoClassifDialog.h"
// #include "qCanupoTrainingDialog.h"
// #include "qCanupo2DViewDialog.h"
// #include "qCanupoTools.h"
#include "qToolsAboutDlg.h"
// #include "qCanupoCommands.h"

//CCCoreLib
#include <CloudSamplingTools.h>

//qCC_db
#include <ccProgressDialog.h>
#include <ccPointCloud.h>


qTidopToolsPlugin::qTidopToolsPlugin(QObject* parent/*=nullptr*/)
	: QObject(parent)
    , ccStdPluginInterface( ":/CC/plugin/qTidopTools/info.json" )
    , m_ptrAboutAction(nullptr)
    , m_ptrManualClassificationAction(nullptr)
    , m_ptrRandomForestClassificationAction(nullptr)
    , m_ptrAboutDlg( nullptr )
    , m_ptrManualClassificationDlg( nullptr )
    , m_ptrRandomForestClassificationDlg( nullptr )
    , m_accepted(false)
{
}

void qTidopToolsPlugin::onNewSelection(const ccHObject::Container& selectedEntities)
{
    if (m_ptrAboutAction)
    {
        m_ptrAboutAction->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
    }

    if (m_ptrManualClassificationAction)
	{
        m_ptrManualClassificationAction->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
	}

    if (m_ptrRandomForestClassificationAction)
	{
        m_ptrRandomForestClassificationAction->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
    }
	m_selectedEntities = selectedEntities;
}

QList<QAction*> qTidopToolsPlugin::getActions()
{
	QList<QAction*> group;

    if (!m_ptrAboutAction)
    {
        m_ptrAboutAction = new QAction("Tidop Tools: settings/about", this);
        m_ptrAboutAction->setToolTip("Tidop Tools: settings/about");
        m_ptrAboutAction->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qTidopTools/images/tidop.png")));
        connect(m_ptrAboutAction, &QAction::triggered, this, &qTidopToolsPlugin::doAboutAction);
    }
    group.push_back(m_ptrAboutAction);

    if (!m_ptrManualClassificationAction)
	{
        m_ptrManualClassificationAction = new QAction("Manual Classification", this);
        m_ptrManualClassificationAction->setToolTip("Manual Point Cloud Classificationedition ");
        m_ptrManualClassificationAction->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qTidopTools/images/manualClassification.png")));
        connect(m_ptrManualClassificationAction, &QAction::triggered, this, &qTidopToolsPlugin::doManualClassificationAction);
	}
    group.push_back(m_ptrManualClassificationAction);

    if (!m_ptrRandomForestClassificationAction)
	{
        m_ptrRandomForestClassificationAction = new QAction("Random Forest Algorithm Classification", this);
        m_ptrRandomForestClassificationAction->setToolTip("Random Forest Algorithm Classification");
        m_ptrRandomForestClassificationAction->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qTidopTools/images/randomForestClasiffication.png")));
        connect(m_ptrRandomForestClassificationAction, &QAction::triggered, this, &qTidopToolsPlugin::doRandomForestClassificationAction);
	}
    group.push_back(m_ptrRandomForestClassificationAction);

	return group;
}

void qTidopToolsPlugin::doAboutAction()
{
    if (!m_app)
    {
        assert(false);
        return;
    }

    if(!m_ptrAboutDlg)
    {
        m_ptrAboutDlg = new ToolsAboutDlg(m_app ? m_app->getMainWindow() : 0);
    }
    bool accepted = m_ptrAboutDlg->exec();
    if(!m_accepted && accepted)
        m_accepted = accepted;
    //disclaimer accepted?

}

void qTidopToolsPlugin::doManualClassificationAction()
{
	if (!m_app)
	{
		assert(false);
		return;
	}

	//disclaimer accepted?
    if(!m_accepted)
    {
        doAboutAction();
        if(!m_accepted)
            return;
    }

    // check selection
    const ccHObject::Container& selectedEntities = m_app->getSelectedEntities();
    if (!m_app->haveOneSelection() || !selectedEntities.front()->isA(CC_TYPES::POINT_CLOUD))
    {
        m_app->dispToConsole("Select only one point cloud!", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }

    // get first selected cloud
    ccPointCloud* cloud = static_cast<ccPointCloud*>(selectedEntities.front());

    if (!cloud->hasScalarFields())
    {
//        ccLog::Error("Cloud has no scalar field");
//        return;
        QString defaultClassificationName=CC_CLASSIFICATION_MODEL_DEFAULT_CLASSIFICATION_FIELD_NAME;
        int sfIdx = cloud->addScalarField(qPrintable(defaultClassificationName));
        if (sfIdx < 0)
        {
            QString msg="Cloud has no scalar field";
            msg+="\nAn error occurred addin default classification field! (see console)";
            ccLog::Error(msg);
//            QMessageBox::information(this,
//                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
//                                      msg);
             return;
        }
        QString classificationModelName=m_ptrAboutDlg->getClassificationModel();
        ScalarType noClassifiedSfValue;
        if(classificationModelName.compare(CC_CLASSIFICATION_MODEL_ASPRS_NAME,Qt::CaseInsensitive)==0)
        {
            noClassifiedSfValue=static_cast<ScalarType>(float(CC_CLASSIFICATION_MODEL_ASPRS_NOT_CLASSIFIED_CODE));
        }
        else// for all other models
        {
            noClassifiedSfValue=static_cast<ScalarType>(float(CC_CLASSIFICATION_MODEL_NO_ASPRS_NOT_CLASSIFIED_CODE));
        }
        int scalarFieldIndex=0;
        CCCoreLib::ScalarField* sf = cloud->getScalarField(scalarFieldIndex);
        int counter = 0;
        for (auto it = sf->begin(); it != sf->end(); ++it, ++counter)
        {
            sf->setValue(counter,noClassifiedSfValue);
        }
    }

    // set colors schema to RGB
    m_app->updateUI();

    if (!m_ptrManualClassificationDlg)
    {
        QString classificationModelName=m_ptrAboutDlg->getClassificationModel();
        m_ptrManualClassificationDlg = new ccToolsManualClassificationDlg(classificationModelName,
                                                                               m_app,
                                                                               m_app->getMainWindow());
        m_app->registerOverlayDialog(m_ptrManualClassificationDlg, Qt::TopRightCorner);
    }

    //we disable all other windows
    m_app->disableAllBut(m_app->getActiveGLWindow());

    m_ptrManualClassificationDlg->linkWith(m_app->getActiveGLWindow());
    m_ptrManualClassificationDlg->setPointCloud(cloud);

    if (m_ptrManualClassificationDlg->start())
    {
        m_ptrManualClassificationDlg->resize(TOOLS_MANUAL_CLASSIFICATION_DIALOG_INITIAL_WIDTH,
                                          TOOLS_MANUAL_CLASSIFICATION_DIALOG_INITIAL_HEIGHT);
        m_app->updateOverlayDialogsPlacement();
    }

}

void qTidopToolsPlugin::doRandomForestClassificationAction()
{
    if (!m_app)
    {
        assert(false);
        return;
    }

    //disclaimer accepted
    if(!m_accepted)
    {
        doAboutAction();
        if(!m_accepted)
            return;
    }

    // check selection
    const ccHObject::Container& selectedEntities = m_app->getSelectedEntities();
    if (!m_app->haveOneSelection() || !selectedEntities.front()->isA(CC_TYPES::POINT_CLOUD))
    {
        m_app->dispToConsole("Select only one point cloud!", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        return;
    }

    // get first selected cloud
    ccPointCloud* cloud = static_cast<ccPointCloud*>(selectedEntities.front());

    if (!cloud->hasScalarFields())
    {
        QString defaultClassificationName=CC_CLASSIFICATION_MODEL_DEFAULT_CLASSIFICATION_FIELD_NAME;
        int sfIdx = cloud->addScalarField(qPrintable(defaultClassificationName));
        if (sfIdx < 0)
        {
            QString msg="Cloud has no scalar field";
            msg+="\nAn error occurred addin default classification field! (see console)";
            ccLog::Error(msg);
//            QMessageBox::information(this,
//                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
//                                      msg);
             return;
        }
        QString classificationModelName=m_ptrAboutDlg->getClassificationModel();
        ScalarType noClassifiedSfValue;
        if(classificationModelName.compare(CC_CLASSIFICATION_MODEL_ASPRS_NAME,Qt::CaseInsensitive)==0)
        {
            noClassifiedSfValue=static_cast<ScalarType>(float(CC_CLASSIFICATION_MODEL_ASPRS_NOT_CLASSIFIED_CODE));
        }
        else// for all other models
        {
            noClassifiedSfValue=static_cast<ScalarType>(float(CC_CLASSIFICATION_MODEL_NO_ASPRS_NOT_CLASSIFIED_CODE));
        }
        int scalarFieldIndex=0;
        CCCoreLib::ScalarField* sf = cloud->getScalarField(scalarFieldIndex);
        int counter = 0;
        for (auto it = sf->begin(); it != sf->end(); ++it, ++counter)
        {
            sf->setValue(counter,noClassifiedSfValue);
        }
//        QString defaultClassificationName=CC_CLASSIFICATION_MODEL_DEFAULT_CLASSIFICATION_FIELD_NAME;
//        int sfIdx = cloud->addScalarField(qPrintable(defaultClassificationName));
//        if (sfIdx < 0)
//        {
//            QString msg="Cloud has no scalar field";
//            msg+="\nAn error occurred addin default classification field! (see console)";
//            ccLog::Error(msg);
////            QMessageBox::information(this,
////                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
////                                      msg);
//             return;
//        }
//        QString classificationModelName=m_ptrAboutDlg->getClassificationModel();
//        if(classificationModelName.compare(CC_CLASSIFICATION_MODEL_ARCHDATASET_NAME,Qt::CaseInsensitive)==0)
//        {
//            ScalarType noClassifiedSfValue=static_cast<ScalarType>(float(CC_CLASSIFICATION_MODEL_ARCHDATASET_NOT_CLASSIFIED_CODE));
//            int scalarFieldIndex=0;
//            CCCoreLib::ScalarField* sf = cloud->getScalarField(scalarFieldIndex);
//            int counter = 0;
//            for (auto it = sf->begin(); it != sf->end(); ++it, ++counter)
//            {
//                sf->setValue(counter,noClassifiedSfValue);
//            }
//        }
    }

    // set colors schema to RGB
    m_app->updateUI();

    if (!m_ptrRandomForestClassificationDlg)
    {
        QString classificationModelName=m_ptrAboutDlg->getClassificationModel();
        m_ptrRandomForestClassificationDlg = new ccToolsRandomForestClassificationDlg(classificationModelName,
                                                                                           m_app,
                                                                                           m_app->getMainWindow());
        m_app->registerOverlayDialog(m_ptrRandomForestClassificationDlg, Qt::TopRightCorner);
    }

    //we disable all other windows
    m_app->disableAllBut(m_app->getActiveGLWindow());

    m_ptrRandomForestClassificationDlg->linkWith(m_app->getActiveGLWindow());
    m_ptrRandomForestClassificationDlg->setPointCloud(cloud);

    if (m_ptrRandomForestClassificationDlg->start())
    {
        m_ptrRandomForestClassificationDlg->resize(TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_INITIAL_WIDTH,
                                          TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_INITIAL_HEIGHT);
        m_app->updateOverlayDialogsPlacement();
    }

}


// void qCanupoPlugin::registerCommands(ccCommandLineInterface* cmd)
// {
	// if (!cmd)
	// {
		// assert(false);
		// return;
	// }
	// cmd->registerCommand(ccCommandLineInterface::Command::Shared(new CommandCanupoClassif));
// }
