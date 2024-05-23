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

#include "../include/qAicedroneTools.h"
#include "../include/ccToolsManualClassificationDlg.h"
#include "../include/ccToolsRandomForestClassificationDlg.h"
#include "../include/ccPointCloudTilesFileDlg.h"

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


qAicedroneToolsPlugin::qAicedroneToolsPlugin(QObject* parent/*=nullptr*/)
	: QObject(parent)
    , ccStdPluginInterface( ":/CC/plugin/qAicedroneTools/info.json" )
    , m_ptrAboutAction(nullptr)
    , m_ptrManualClassificationAction(nullptr)
    , m_ptrRandomForestClassificationAction(nullptr)
    , m_ptrPointCloudTilesFileAction(nullptr)
    , m_ptrAboutDlg( nullptr )
    , m_ptrManualClassificationDlg( nullptr )
    , m_ptrRandomForestClassificationDlg( nullptr )
    , m_ptrPointCloudTilesFileDlg( nullptr)
    , m_accepted(false)
{
}

void qAicedroneToolsPlugin::onNewSelection(const ccHObject::Container& selectedEntities)
{
//    if (m_ptrAboutAction)
//    {
//        m_ptrAboutAction->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
//    }

    if (m_ptrManualClassificationAction)
	{
        m_ptrManualClassificationAction->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
	}

    if (m_ptrRandomForestClassificationAction)
	{
        m_ptrRandomForestClassificationAction->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
    }

//    if (m_ptrPointCloudTilesFileAction)
//    {
//        m_ptrPointCloudTilesFileAction->setEnabled(selectedEntities.size() == 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
//    }
    m_selectedEntities = selectedEntities;
}

QList<QAction*> qAicedroneToolsPlugin::getActions()
{
	QList<QAction*> group;

    if (!m_ptrAboutAction)
    {
        m_ptrAboutAction = new QAction("AICEDRONE: Settings", this);
        m_ptrAboutAction->setToolTip("AICEDRONE: Settings");
        m_ptrAboutAction->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qAicedroneTools/images/rover_icon.png")));
        connect(m_ptrAboutAction, &QAction::triggered, this, &qAicedroneToolsPlugin::doAboutAction);
    }
    group.push_back(m_ptrAboutAction);

    if (!m_ptrPointCloudTilesFileAction)
    {
        m_ptrPointCloudTilesFileAction = new QAction("AICEDRONE: Point Cloud Tiles File", this);
        m_ptrPointCloudTilesFileAction->setToolTip("AICEDRONE: Point Cloud Tiles File");
        m_ptrPointCloudTilesFileAction->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qAicedroneTools/images/tiles.png")));
        connect(m_ptrPointCloudTilesFileAction, &QAction::triggered, this, &qAicedroneToolsPlugin::doPointCloudTilesFileAction);
    }
    group.push_back(m_ptrPointCloudTilesFileAction);

    if (!m_ptrManualClassificationAction)
	{
        m_ptrManualClassificationAction = new QAction("AICEDRONE:Manual Classification", this);
        m_ptrManualClassificationAction->setToolTip("AICEDRONE:Manual Point Cloud Classificationedition ");
        m_ptrManualClassificationAction->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qAicedroneTools/images/manualClassification.png")));
        connect(m_ptrManualClassificationAction, &QAction::triggered, this, &qAicedroneToolsPlugin::doManualClassificationAction);
	}
    group.push_back(m_ptrManualClassificationAction);

    if (!m_ptrRandomForestClassificationAction)
	{
        m_ptrRandomForestClassificationAction = new QAction("AICEDRONE:Random Forest Algorithm Classification", this);
        m_ptrRandomForestClassificationAction->setToolTip("AICEDRONE:Random Forest Algorithm Classification");
        m_ptrRandomForestClassificationAction->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qAicedroneTools/images/randomForestClasiffication.png")));
        connect(m_ptrRandomForestClassificationAction, &QAction::triggered, this, &qAicedroneToolsPlugin::doRandomForestClassificationAction);
	}
    group.push_back(m_ptrRandomForestClassificationAction);

	return group;
}

void qAicedroneToolsPlugin::doAboutAction()
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

void qAicedroneToolsPlugin::doManualClassificationAction()
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

//    if (!cloud->hasScalarFields())
//    {
//        ccLog::Error("Cloud has no scalar field");
//        return;
//    }
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

void qAicedroneToolsPlugin::doRandomForestClassificationAction()
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

void qAicedroneToolsPlugin::doPointCloudTilesFileAction()
{
    if (!m_app)
    {
        assert(false);
        return;
    }

    //disclaimer accepted?
//    if(!m_accepted)
//    {
//        doAboutAction();
//        if(!m_accepted)
//            return;
//    }

//     check selection
//    const ccHObject::Container& selectedEntities = m_app->getSelectedEntities();
//    if (!m_app->haveOneSelection() || !selectedEntities.front()->isA(CC_TYPES::POINT_CLOUD))
//    {
//        m_app->dispToConsole("Select only one point cloud!", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
//        return;
//    }

    // get first selected cloud
//    ccPointCloud* cloud = static_cast<ccPointCloud*>(selectedEntities.front());

//    if (!cloud->hasScalarFields())
//    {
//        ccLog::Error("Cloud has no scalar field");
//        return;
//    }

//    // set colors schema to RGB
    m_app->updateUI();

    if (!m_ptrPointCloudTilesFileDlg)
    {
        m_ptrPointCloudTilesFileDlg = new ccPointCloudTilesFileDlg(m_app,
                                                                   m_app->getMainWindow());
        m_app->registerOverlayDialog(m_ptrPointCloudTilesFileDlg, Qt::TopRightCorner);
    }

    //we disable all other windows
    m_app->disableAllBut(m_app->getActiveGLWindow());

    m_ptrPointCloudTilesFileDlg->linkWith(m_app->getActiveGLWindow());
//    m_ptrManualClassificationDlg->setPointCloud(cloud);

    if (m_ptrPointCloudTilesFileDlg->start())
    {
        m_ptrPointCloudTilesFileDlg->resize(POINT_CLOUD_TILES_FILE_DIALOG_INITIAL_WIDTH,
                                          POINT_CLOUD_TILES_FILE_DIALOG_INITIAL_HEIGHT);
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
