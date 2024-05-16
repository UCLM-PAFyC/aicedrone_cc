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

// local
#include <ui_ccTidopToolsManualClassificationDlg.h>
#include "ccClassificationModel.h"
#include "ccTidopToolsClassificationHelper.h"

//CC
#include <ccOverlayDialog.h>

#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE  "Tidop Tools: Manual Classification"
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_INITIAL_WIDTH  300
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_INITIAL_HEIGHT  700
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT  " ... "
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES                            "All classes"
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES                    "All visible classes"
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY            "Select only"
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT               "Unselect"
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS           "Change class"
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE                 "Remove"
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS "Recover original class"
#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER                "Recover removed points"
//#define TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_

class ccPointCloud;
class ccMouseCircle;

class ccTidopToolsManualClassificationDlg : public ccOverlayDialog, public Ui::ccTidopToolsManualClassificationDlg
{
	Q_OBJECT

public:
	//! Default constructor
    explicit ccTidopToolsManualClassificationDlg(QString classificationModelName,
                                                 ccMainAppInterface* app,
                                                 QWidget* parent = nullptr);

	//! Destructor
	virtual ~ccTidopToolsManualClassificationDlg();

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

private:
    ccMainAppInterface* m_ptrApp;
    ccClassificationModel* m_ptrClassificationModel;
    ccTidopToolsClassificationHelper* m_ptrHelper;
    ccMouseCircle* m_ptrMouseCircle;
};

