#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>

// local
#include <ui_ccToolsRandomForestClassificationDlg.h>
#include "ccClassificationModel.h"
#include "ccToolsClassificationHelper.h"
#include "../include/ccClassificationToolCGAL.h"
#include "../include/ccToolsETHZRandomForest.h"

//CC
#include <ccOverlayDialog.h>

#include "ccProgressDialog.h"

#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE  "AICEDRONE: Random Forest Classification"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_INITIAL_WIDTH  540
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_INITIAL_HEIGHT  700
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT  " ... "
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES                            "All classes"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES                    "All visible classes"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY            "Select only"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT               "Unselect"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS           "Change class"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE                 "Remove"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS "Recover original class"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER                "Recover removed points"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_DEFAULT_NEW_FIELD                       "training"
#define TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_DEFAULT_NEW_FIELD_SUFFIX                "_tr"

class ParametersManager;
class ccClassificationToolCGAL;
class ccToolsETHZRandomForest;

class ccPointCloud;
class ccMouseCircle;

class ccToolsRandomForestClassificationDlg : public ccOverlayDialog, public Ui::ccToolsRandomForestClassificationDlg
{
	Q_OBJECT

public:
	//! Default constructor
    explicit ccToolsRandomForestClassificationDlg(QString classificationModelName,
                                                       ccMainAppInterface* app,
                                                       QWidget* parent = nullptr);

	//! Destructor
	virtual ~ccToolsRandomForestClassificationDlg();

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

    inline void addPointToPolyline(int x, int y) { return addPointToPolylineExt(x, y, false); }
    void addPointToPolylineExt(int x, int y, bool allowClicksOutside);
    void closePolyLine(int x = 0, int y = 0); //arguments for compatibility with ccGlWindow::rightButtonClicked signal
    void closeRectangle();
    void updatePolyLine(int x, int y, Qt::MouseButtons buttons);
    void run();
    void stopRunning();

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
    bool static trainingClassesSeparabiltyAnalysis(ParametersManager* ptrParametersManager,
                                                   QString trainFieldName);
    ccMainAppInterface* m_ptrApp;
    ccClassificationModel* m_ptrClassificationModel;
    ccToolsClassificationHelper* m_ptrHelper;
    ccMouseCircle* m_ptrMouseCircle;
    QString m_basePath;
    ParametersManager* m_ptrParametersManager;
    static ccClassificationToolCGAL* m_ptrClassificationToolCGAL;
    static ccToolsETHZRandomForest* m_ptrToolsETHZRandomForest;
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

