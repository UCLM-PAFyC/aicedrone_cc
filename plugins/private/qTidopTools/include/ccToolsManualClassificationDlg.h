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

#pragma once

// local
#include <ui_ccToolsManualClassificationDlg.h>
#include "ccClassificationModel.h"
#include "ccToolsClassificationHelper.h"

//CC
#include <ccOverlayDialog.h>

#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE  "Tidop Tools: Manual Classification"
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_INITIAL_WIDTH  300
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_INITIAL_HEIGHT  700
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT  " ... "
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES                            "All classes"
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES                    "All visible classes"
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY            "Select only"
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT               "Unselect"
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS           "Change class"
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE                 "Remove"
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS "Recover original class"
#define TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER                "Recover removed points"

class ccPointCloud;
class ccMouseCircle;

class ccToolsManualClassificationDlg : public ccOverlayDialog, public Ui::ccToolsManualClassificationDlg
{
	Q_OBJECT

public:
	//! Default constructor
    explicit ccToolsManualClassificationDlg(QString classificationModelName,
                                                 ccMainAppInterface* app,
                                                 QWidget* parent = nullptr);

	//! Destructor
	virtual ~ccToolsManualClassificationDlg();

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
    void editStartClicked();

	//! stop drawing mouse cirlce
    void editPauseClicked();

	//! apply changes and close dialog
	void applyClicked();

    //! apply changes and close dialog
    void editApplyClicked();

	//! restore changes and close dialog
	void closeClicked();

    void editCircleClicked();
    void editPolygonClicked();
    void editRectangleClicked();

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

    inline void addPointToPolyline(int x, int y) { return addPointToPolylineExt(x, y, false); }
    void addPointToPolylineExt(int x, int y, bool allowClicksOutside);
    void closePolyLine(int x = 0, int y = 0); //arguments for compatibility with ccGlWindow::rightButtonClicked signal
    void closeRectangle();
    void updatePolyLine(int x, int y, Qt::MouseButtons buttons);
    void run();
    void stopRunning();

private:
    ccMainAppInterface* m_ptrApp;
    ccClassificationModel* m_ptrClassificationModel;
    ccToolsClassificationHelper* m_ptrHelper;
    ccMouseCircle* m_ptrMouseCircle;

    enum PointsEditionProcessStates
    {
        POLYLINE		= 1,
        RECTANGLE		= 2,
        //...			= 4,
        //...			= 8,
        //...			= 16,
        PAUSED			= 32,
        STARTED			= 64,
        RUNNING			= 128,
    };
    unsigned m_pointsEditionState;
    ccPolyline* m_segmentationPoly;
    ccPointCloud* m_polyVertices;
    bool m_segmentationPolyIsAdded;
};

