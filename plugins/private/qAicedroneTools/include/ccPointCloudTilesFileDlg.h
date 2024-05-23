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

// local
#include <ui_ccPointCloudTilesFileDlg.h>

//CC
#include <ccOverlayDialog.h>
#include <CCGeom.h>
#include <CCGeom.h>
#include <ccColorTypes.h>
#include <ccGenericGLDisplay.h>
#include <ccPointCloud.h>

#define POINT_CLOUD_TILES_FILE_DIALOG_TITLE "AICEDRONE: Point Cloud Tiles File"
#define POINT_CLOUD_TILES_FILE_DIALOG_INITIAL_WIDTH 560
#define POINT_CLOUD_TILES_FILE_DIALOG_INITIAL_HEIGHT 700
#define POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT " ... "
#define POINT_CLOUD_TILES_FILE_DIALOG_DEFAULT_CRS "EPSG:25830"
#define POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT "Ellipsoid"
#define POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX "EPSG:"
#define POINT_CLOUD_TILES_FILE_DIALOG_DEFAULT_VERTICAL_CRS "EPSG:5782"
#define POINT_CLOUD_TILES_FILE_DIALOG_PLUGINS_FOLDER "/plugins"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_DEFAULT_TEMP_FOLDER "temp"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_DEFAULT_OUTPUT_FOLDER "output"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_FILENAME "QAICEDRONETOOLS_PLUGIN.ini"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH "last_path"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_TEMP_PATH "project_management_temporal_path"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_OUTPUT_PATH "project_management_output_path"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LASTOOLS_PATH "lastools_path"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_PROJECTS "projects"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_MODEL_DATABASES "model_databases"
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS "@#&"
#define POINT_CLOUD_TILES_FILE_ROIS_LAYER_NAME "Regions of interest"
#define POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE                   100000000.
#define POINTCLOUDFILE_MIN_Z_DECREMENT_STEP 0.01
#define POINTCLOUDFILE_ORTHOMOSAIC_ZMIN_OFFSET -2.0
#define POINTCLOUDFILE_ROIS_ZMIN_OFFSET -1.0
#define POINTCLOUDFILE_TILES_ZMIN_OFFSET -0.5
#define POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_ORTHOMOSAIC "orthomosaic"
#define POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_CLASSES_TITLE      "... Select Visible Classes ..."
#define POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_OBJECTS_TITLE      "... Select Visible Objects ..."
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_ENABLE    "enable object"
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_DISABLE    "disable object"
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_REMOVE    "remove object"
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_CHECK    "check object"
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_UNCHECK    "uncheck object"
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_SET_CANDIDATE    "set candidate"
//#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_SET_CANDIDATE    "set candidate"
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESSING_TOOLS_RUN_BUTTON_PROCESS_LIST_TEXT "Run Process List"
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESSING_TOOLS_RUN_BUTTON_PROCESS_TEXT  "Run Process"
#define POINT_CLOUD_TILES_FILE_DIALOG_PROCESSING_TOOLS_PROCESS_LIST_EDITION_DIALOG_TITLE "Edit Process List"

class ccHObject;
class QSettings;
class ccMainAppInterface;
class QComboCheckBox;

namespace PCFile {
class PointCloudFileManager;
}

namespace libCRS{
class CRSTools;
}

class MultipleFileSelectorDialog;

namespace AicedroneModelDb{
class ModelObject;
class ModelObjectClass;
class ModelSpatialiteDb;
}

class ccPointCloudTilesFileDlg : public ccOverlayDialog, public Ui::ccPointCloudTilesFileDlg
{
	Q_OBJECT

public:
	//! Default constructor
    explicit ccPointCloudTilesFileDlg(ccMainAppInterface* app,
	                                  QWidget* parent = nullptr);

	//! Destructor
	virtual ~ccPointCloudTilesFileDlg();

	//! inherited from ccOverlayDialog
	bool start() override;
    void stop(bool accepted) override;

private:
	void initialize();
    void projectCloud(const ccGLCameraParameters& camera);
    void project(ccGLCameraParameters camera, unsigned start, unsigned end);
    void reject() override;
    void resetUI();

private Q_SLOTS:
    void acceptClicked();
    void addProject();
	void closeClicked();
    void closeProject();
    void createProject();
    void loadOrthomosaic();
    void onHorizontalCRSsComboBoxCurrentIndexChanged(int index);
    void onVerticalCRSsComboBoxCurrentIndexChanged(int index);
    void onAddPCFsHorizontalCRSsComboBoxCurrentIndexChanged(int index);
    void onModelDbsComboBoxCurrentIndexChanged(int index);
    void onPpToolsIPCFsHorizontalCRSsComboBoxCurrentIndexChanged(int index);
    void onProjectsComboBoxCurrentIndexChanged(int index);
    void openProject();
    void ppToolsAddProcessToList();
    void ppToolsTabWidgetChanged(int index);
    void ppToolsProcessListEdition();
    void ppToolsRunProcessList();
    void ppToolsSelectLastoolsCommandParameters();
    void ppToolsSelectInternalCommandParameters();
    void removeProject();
    void ppToolsSelectInternalCommand(int index);
    void ppToolsSelectLastoolsCommand(int index);
    void ppToolsSelectLastoolsPath();
    void ppToolsSelectOutputFile();
    void ppToolsSelectOutputPath();
    void ppToolsSelectPCFiles();
    void ppToolsSelectSuffix();
    void ppToolsSelectPrefix();
    void selectOrthomosaic();
    void selectOutputPath();
    void selectPCFs();
    void selectProjectParameters();
    void selectProjectPath();
    void selectROIs();
    void selectTempPath();
    void showObjectNames(int value);
    void editStartClicked();
    void editPauseClicked();
    void loadSelectedTiles();
    void unloadTiles();
    void unloadTilesAll();
    void editPolygonClicked();
    void editRectangleClicked();
    inline void addPointToPolyline(int x, int y) { return addPointToPolylineExt(x, y, false); }
    void addPointToPolylineExt(int x, int y, bool allowClicksOutside);
    void closePolyLine(int x = 0, int y = 0); //arguments for compatibility with ccGlWindow::rightButtonClicked signal
    void closeRectangle();
    void updatePolyLine(int x, int y, Qt::MouseButtons buttons);
    void run();
    void stopRunning();
    void onVisibleModelClassesComboCheckBox(int row,
                                            bool isChecked,
                                            QString text,
                                            QString data);
    void onVisibleModelObjectsComboCheckBox(int row,
                                            bool isChecked,
                                            QString text,
                                            QString data);
    void addModelDb();
    void openModelDb();
    void closeModelDb();
    void onModelDbObjectCandidateComboBoxCurrentIndexChanged(int index);
    void onModelDbObjectComboBoxCurrentIndexChanged(int index);
    void onProcessModelDbObjectComboBoxCurrentIndexChanged(int index);
    void onToolBoxCurrentIndexChanged(int index);
    void removeModelDb();
    void pointCloudClicked();
    void processModelDbObject();
    void modelDbClicked();
    void visibleModelObjectsAll();
    void visibleModelObjectsNone();

private:
    bool addCube(int dbId,
                 QString& name,
                 QVector<QVector<double> > &vertices,
                 QString& strError);
    void addPointCloudFiles();
    void addModelDbFile(QString fileName);
    void addProjectPath(QString projectPath);
    void checkModelDbObject();
    void enableModelDbObject();
    void disableModelDbObject();
    void getMinimumCoordinates();
    void loadROIsLayer();
    void loadTilesLayer();
    void removeModelDbObject();
    void setCandidateModelDbObject();
    void setVerticalCRS(int horizontalCRSEpsgCode);
    void setAddPCFsVerticalCRS(int horizontalCRSEpsgCode);
    void setPpToolsIPFCsVerticalCRS(int horizontalCRSEpsgCode);
//    void setSelectedCandidateForPreviousModel();
    void uncheckModelDbObject();
    ccMainAppInterface* mPtrApp;
    PCFile::PointCloudFileManager* mPtrPointCloudFileManager;
    libCRS::CRSTools* mPtrCrsTools;
    QString mBasePath;
    QString mLastPath;
    bool mIsInitialized;
    QStringList mROIsShapefiles;
    MultipleFileSelectorDialog* mPtrROIsMultipleFileSelectorDialog;
    QStringList mPCFiles;
    MultipleFileSelectorDialog* mPtrPCFsMultipleFileSelectorDialog;
    QSettings *mPtrSettings;
    QString mTempPath;
    QString mOutputPath;
    QString mLastoolsPath;
    QStringList mProjects;
    QStringList mModelDbs;
    ccHObject* mPtrCcROIsContainer;
    ccHObject* mPtrCcTilesContainer;
    ccHObject* mPtrCcTilesCenterPointsContainer;
    ccHObject* mPtrCcOrthomosaicContainer;
    QMap<int, QMap<int, QMap<int,ccHObject*> > > mPtrCcPointsContainerByTileByFileId; //[fileId][tileX][tileY]
    CCVector3d* mPtrPshift;
    double mMinX,mMinY,mMinZ;
    double mMinZDecrement;
    QString mProjectName;
    // selection
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
    ccPolyline* m_selectionTilesPoly;
    ccPointCloud* m_selectionTilesPolyVertices;
    ccGLCameraParameters m_cameraParameters;
    int mHorizontalCrsEpsgCode;
    int mVerticalCrsEpsgCode; // -1 ellipsoid
    bool mSelectionTilesPolyIsAdded;

    QComboCheckBox* mVisibleModelClassesComboCheckBox;
    QComboCheckBox* mVisibleModelObjectsComboCheckBox;
//    QVector<int> mPointsPGDbId; // -1 si no pertenece a ninguno, para consultar para cualquier punto
    QMap<int,AicedroneModelDb::ModelObjectClass*> mPtrModelObjectClassesByDbId;
    QMap<int,AicedroneModelDb::ModelObject*> mPtrModelObjectsByDbId;
    QMap<int,ccHObject*> mPtrCcCubeContainerByCubeDbId;
    AicedroneModelDb::ModelSpatialiteDb* mPtrMsDb;
    QMap<int,QString> mModelNameByModelObjectDbId;
    QMap<QString,QVector<int> > mModelObjectsDbIdByModelName;
    QMap<QString,int> mSelectedModelObjectDbIdByModelName;
    QMap<QString,int> mSelectedInComboModelObjectDbIdByModelName;
    QString mPreviousModelName;

    QStringList mPpToolsPCFiles;
    MultipleFileSelectorDialog* mPtrPpToolsPCFsMultipleFileSelectorDialog;
    QStringList mProcessList;
};
