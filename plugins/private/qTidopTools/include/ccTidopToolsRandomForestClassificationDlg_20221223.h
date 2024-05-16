#pragma once

//##########################################################################
//#                                                                        #
//#                   CLOUDCOMPARE PLUGIN: qCloudLayers                    #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#                     COPYRIGHT: WigginsTech 2022                        #
//#                                                                        #
//##########################################################################

#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>

// local
#include <ui_ccTidopToolsRandomForestClassificationDlg.h>
#include "ccClassificationModel.h"
#include "ccTidopToolsClassificationHelper.h"
#include "../include/ccClassificationToolCGAL.h"
#include "../include/ccTidopToolsETHZRandomForest.h"

//CC
#include <ccOverlayDialog.h>

#include "ccProgressDialog.h"

#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE  "Tidop Tools: Random Forest Classification"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_INITIAL_WIDTH  540
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_INITIAL_HEIGHT  700
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT  " ... "
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES                            "All classes"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES                    "All visible classes"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY            "Select only"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT               "Unselect"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS           "Change class"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE                 "Remove"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS "Recover original class"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER                "Recover removed points"
//#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_DEFAULT_NEW_FIELD                       "training"
#define TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_DEFAULT_NEW_FIELD_SUFFIX                "_tr"

class ParametersManager;
class ccClassificationToolCGAL;
class ccTidopToolsETHZRandomForest;

class ccPointCloud;
class ccMouseCircle;

class ccTidopToolsRandomForestClassificationDlg : public ccOverlayDialog, public Ui::ccTidopToolsRandomForestClassificationDlg
{
	Q_OBJECT

public:
	//! Default constructor
    explicit ccTidopToolsRandomForestClassificationDlg(QString classificationModelName,
                                                       ccMainAppInterface* app,
                                                       QWidget* parent = nullptr);

	//! Destructor
	virtual ~ccTidopToolsRandomForestClassificationDlg();

	//! inherited from ccOverlayDialog
	bool start() override;
	void stop(bool accepted) override;
	
	void setPointCloud(ccPointCloud* cloud);

private:
	void resetUI();
	void initTableView();

	void saveSettings();
	void loadSettings();

	bool eventFilter(QObject* obj, QEvent* event) override;
	void reject() override;

    void updateCbScalarFields();


private Q_SLOTS:

	//! add new asprs item
	void addClicked();

	//! delete select(ed) asprs item(s)
	void deleteClicked();

	//! draw mouse circle
	void startClicked();

	//! stop drawing mouse cirlce
	void pauseClicked();

	//! apply changes and close dialog
	void applyClicked();

	//! restore changes and close dialog
	void closeClicked();

    void newScalarFieldClicked();
    void deleteScalarFieldClicked();
    void cloneScalarFieldClicked();

	void scalarFieldIndexChanged(int index);
    void selectedPointsToolsChanged(int index);
    void selectedPointsScalarValuesChanged(int index);

	//! asprs model signals
	void codeChanged(ccClassificationModel::Item &item, int oldCode);
	void colorChanged(ccClassificationModel::Item &item);

    void setAllVisible();
    void setAllInvisible();
    void setAllLocked();
    void setAllUnlocked();

    void selectedPointsProcess();

	//! show color picker dialog
	void tableViewDoubleClicked(const QModelIndex &index);
    void tableViewClicked(const QModelIndex &index);

	//! update input and output comboboxes
    void uptdateSelectedPointsTools();

    void mouseMoved(int x, int y, Qt::MouseButtons buttons);

    void classificationTrainingScalarFieldChanged();
    void classificationTargetScalarFieldChanged();
    void classificationStepChanged();

    void selectParameters();
    void selectClassificationProcess();

    void useCGALClicked();

public slots:
    void on_process_finished();

private:
    bool initialize(QString& strError);
    bool static classify(ParametersManager* ptrParametersManager,
                         QString targetFieldName);
    bool static computeFeatures(ParametersManager* ptrParametersManager);
//    bool computeFeatures(ParametersManager* ptrParametersManager,
//                         QString& strError);
    bool static train(ParametersManager* ptrParametersManager,
                      QString trainFieldName,
                      QString targetFieldName);
    ccMainAppInterface* m_ptrApp;
    ccClassificationModel* m_ptrClassificationModel;
    ccTidopToolsClassificationHelper* m_ptrHelper;
    ccMouseCircle* m_ptrMouseCircle;
    QString m_basePath;
    ParametersManager* m_ptrParametersManager;
    static ccClassificationToolCGAL* m_ptrClassificationToolCGAL;
    static ccTidopToolsETHZRandomForest* m_ptrTidopToolsETHZRandomForest;
    QMap<QString,QVector<int> > m_previousClassificationFieldValues;
    QMap<QString,bool> updatedScalarFields;
    QFuture<bool> m_future;
    QFutureWatcher<bool> m_futureWatcher;
    QProgressDialog* m_ptrProcessesDialog;
    static QString m_classificationProcessCommand;

    static QString m_strError;
    static QString m_functionName;
    static QString m_strReport;
    static QString m_strTime;

    static bool m_useCGAL;

};

