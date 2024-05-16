#include <qToolsDefinitions.h>
#include "../include/ccPointCloudTilesFileDlg.h"

//QT
#include <QColorDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
// #include <QSortFilterProxyModel>
#include <QWidget>
#include <QMap>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>

//CC
#include <ccMainAppInterface.h>
#include <ccGLWindow.h>
#include <ccPointCloud.h>
#include <ccPolyline.h>
#include <ccMesh.h>
#include <ccPlane.h>
#include <ManualSegmentationTools.h>

//qCC_io
#include <ccShiftAndScaleCloudDlg.h>
#include <FileIOFilter.h>

// PAFYC
#include <libPointCloudFileManager/PointCloudFileManager.h>
//#include <libPointCloudFileManager/PointCloudFile.h>
#include <libCRS/CRSTools.h>
#include <libWidgets/MultipleFileSelectorDialog.h>
#include <libWidgets/ProcessListEditionDialog.h>
#include <libWidgets/QComboCheckBox.h>
#include <libAicedroneModelDb/ModelDbDefinitions.h>
#include <libAicedroneModelDb/ModelSpatialiteDb.h>
#include <libAicedroneModelDb/ModelObjectClass.h>
#include <libAicedroneModelDb/ModelObject.h>
#include <libAicedroneModelDb/ParametricGeometry.h>


#include <gdal_priv.h>
#include <ogrsf_frmts.h>

#ifndef CC_RESERVED_IDS_HEADER
#define CC_RESERVED_IDS_HEADER

//! Unique IDs reserved by CloudCompare for special entities (display elements, etc.)
/** They should all remain below ccUniqueIDGenerator::MinUniqueID (256)
**/
enum class ReservedIDs : unsigned
{
    CLIPPING_BOX = 1,
    INTERACTIVE_SEGMENTATION_TOOL_POLYLINE = 2,
    INTERACTIVE_SEGMENTATION_TOOL_POLYLINE_VERTICES = 3,
    TRACE_POLYLINE_TOOL_POLYLINE_TIP = 4,
    TRACE_POLYLINE_TOOL_POLYLINE_TIP_VERTICES = 5,
    TRACE_POLYLINE_TOOL_POLYLINE = 6,
    TRACE_POLYLINE_TOOL_POLYLINE_VERTICES = 7,
};

#endif //CC_RESERVED_IDS_HEADER

ccPointCloudTilesFileDlg::ccPointCloudTilesFileDlg(ccMainAppInterface* app,
                                                   QWidget* parent)
	: ccOverlayDialog(parent)
	, Ui::ccPointCloudTilesFileDlg()
    , mPtrSettings(nullptr)
    , mPtrPointCloudFileManager(nullptr)
    , mPtrMsDb(nullptr)
    , mIsInitialized(false)
    , mPtrROIsMultipleFileSelectorDialog(nullptr)
    , mPtrPCFsMultipleFileSelectorDialog(nullptr)
    , mPtrCcROIsContainer(nullptr)
    , mPtrCcTilesContainer(nullptr)
    , mPtrCcTilesCenterPointsContainer(nullptr)
    , mPtrPshift(nullptr)
    , mPtrCcOrthomosaicContainer(nullptr)
    , mMinX(POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE)
    , mMinY(POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE)
    , mMinZ(POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE)
    , m_pointsEditionState(0)
    , m_selectionTilesPoly(nullptr)
    , m_selectionTilesPolyVertices(nullptr)
    , mSelectionTilesPolyIsAdded(false)
    , mPtrPpToolsPCFsMultipleFileSelectorDialog(nullptr)
    , mPtrApp(app)
{
	setupUi(this);

    setWindowTitle(QString(POINT_CLOUD_TILES_FILE_DIALOG_TITLE));

	// allow resize and move window
	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint/* | Qt::WindowTitleHint*/);
	
	initialize();
}

ccPointCloudTilesFileDlg::~ccPointCloudTilesFileDlg()
{
    closeProject();
    if(mPtrPointCloudFileManager)
    {
        delete(mPtrPointCloudFileManager);
        mPtrPointCloudFileManager = nullptr;
    }
    if(mPtrMsDb)
    {
        delete(mPtrMsDb);
        mPtrMsDb = nullptr;
    }
    if (m_selectionTilesPoly)
        delete m_selectionTilesPoly;
    m_selectionTilesPoly = nullptr;
    if (m_selectionTilesPolyVertices)
        delete m_selectionTilesPolyVertices;
    m_selectionTilesPolyVertices = nullptr;
}
	
void ccPointCloudTilesFileDlg::acceptClicked()
{
//    saveSettings();
    stop(true);
}

void ccPointCloudTilesFileDlg::addProject()
{
    QString functionName="ccPointCloudTilesFileDlg::addProject";
    QString selectedPath=mLastPath;
    selectedPath = QFileDialog::getExistingDirectory(this,
                                                     QObject::tr("Select Project path to add"),
                                                     mLastPath,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (!selectedPath.isEmpty())
    {
        if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
        {
            mLastPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
            mPtrSettings->sync();
        }
        addProjectPath(selectedPath);
    }
}
	
void ccPointCloudTilesFileDlg::closeClicked()
{
    stop(true);
}

void ccPointCloudTilesFileDlg::closeProject()
{
    QString functionName="ccPointCloudTilesFileDlg::closeProject";
    QString strAuxError;
    QString projectPath=projectsComboBox->currentText();
    toolBox->setItemEnabled(1,false);
    if(projectPath.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
//        QString title=functionName;
//        QString msg=QObject::tr("Select project before");
//        QMessageBox::information(this,title,msg);
        return;
    }
//    if(!mPtrPointCloudFileManager->openPointCloudFile(projectPath,strAuxError))
//    {
//        QString title=functionName;
//        QString msg=QObject::tr("Opening project path:\n%1\nError:\n%2")
//                .arg(projectPath).arg(strAuxError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
    projectsComboBox->setEnabled(true);
    projectsComboBox->setCurrentIndex(0);
//    addProjectPushButton->setEnabled(false);
//    openProjectPushButton->setEnabled(false);
//    closeProjectPushButton->setEnabled(true);
//    removeProjectPushButton->setEnabled(false);
//    projectManagementTabWidget->setTabEnabled(0,false);
//    projectManagementTabWidget->setTabEnabled(1,true);
//    projectManagementTabWidget->setTabEnabled(2,true);
//    projectManagementTabWidget->setCurrentIndex(1);
    bool neededRedraw=false;
    if(mPtrCcROIsContainer)
    {
        bool autoRemove=true;
        mPtrApp->removeFromDB(mPtrCcROIsContainer, autoRemove);
//        delete(mPtrCcProjectContainer);
        mPtrCcROIsContainer=nullptr;
        neededRedraw=true;
    }
    if(mPtrCcTilesContainer)
    {
        bool autoRemove=true;
        mPtrApp->removeFromDB(mPtrCcTilesContainer, autoRemove);
//        delete(mPtrCcProjectContainer);
        mPtrCcTilesContainer=nullptr;
        if(!neededRedraw)
            neededRedraw=true;
    }
    if(mPtrCcTilesCenterPointsContainer)
    {
        bool autoRemove=true;
        mPtrApp->removeFromDB(mPtrCcTilesCenterPointsContainer, autoRemove);
//        delete(mPtrCcProjectContainer);
        mPtrCcTilesCenterPointsContainer=nullptr;
        if(!neededRedraw)
            neededRedraw=true;
    }
    QMap<int, QMap<int, QMap<int,ccHObject*> > >::iterator iterFileId=mPtrCcPointsContainerByTileByFileId.begin();
    while(iterFileId!=mPtrCcPointsContainerByTileByFileId.end())
    {
        int fileId=iterFileId.key();
        QMap<int, QMap<int,ccHObject*> >::iterator iterTileX=mPtrCcPointsContainerByTileByFileId[fileId].begin();
        while(iterTileX!=mPtrCcPointsContainerByTileByFileId[fileId].end())
        {
            int tileX=iterTileX.key();
            QMap<int,ccHObject*>::iterator iterTileY=mPtrCcPointsContainerByTileByFileId[fileId][tileX].begin();
            while(iterTileY!=mPtrCcPointsContainerByTileByFileId[fileId][tileX].end())
            {
                int tileY=iterTileY.key();
                ccHObject* tileContainer=iterTileY.value();
                bool autoRemove=true;
                mPtrApp->removeFromDB(tileContainer, autoRemove);
                mPtrCcPointsContainerByTileByFileId[fileId][tileX][tileY]=nullptr;
                if(!neededRedraw)
                    neededRedraw=true;
                iterTileY++;
            }
            iterTileX++;
        }
        iterFileId++;
    }
    mPtrCcPointsContainerByTileByFileId.clear();
    if(neededRedraw)
    {
        mPtrApp->redrawAll();
    }
}

void ccPointCloudTilesFileDlg::createProject()
{
    QString functionName="ccPointCloudTilesFileDlg::createProject";
    QString strAuxError;
    QString projectType=projectTypeComboBox->currentText();
    if(projectType.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select project type before");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString strGridSize=gridSizeComboBox->currentText();
    if(strGridSize.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select grid size before");
        QMessageBox::information(this,title,msg);
        return;
    }
    double gridSize=strGridSize.toDouble();
    QString horizontalCrsId=horizontalCRSsComboBox->currentText();
    if(horizontalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select horizontal CRS before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int horizontalCrsEpsgCode;
    if(!mPtrCrsTools->getCrsEpsgCode(horizontalCrsId,horizontalCrsEpsgCode,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting EPSG code for horizontal CRS: %1\nError:\n%2")
                .arg(horizontalCrsId).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString verticalCrsId=verticalCRSsComboBox->currentText();
    if(verticalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select vertical CRS before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int verticalCrsEpsgCode=-1;
    if(verticalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT,Qt::CaseInsensitive)!=0)
    {
        QString strVerticalCrsEpsgCode=verticalCrsId.remove(POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX);
        verticalCrsEpsgCode=strVerticalCrsEpsgCode.toInt();
    }
    QString projectPath=projectPathLineEdit->text();
    if(projectPath.isEmpty())
    {
        QString title=functionName;
        QString msg=QObject::tr("Select project path before");
        QMessageBox::information(this,title,msg);
        return;
    }
    if(mProjects.indexOf(projectPath)!=-1)
    {
        QString title=functionName;
        QString msg=QObject::tr("Before you must remove existing project for path:\n%1").arg(projectPath);
        QMessageBox::information(this,title,msg);
        return;
    }
    if (QDir(projectPath).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() != 0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Selected project path must be empty:\n%1").arg(projectPath);
        QMessageBox::information(this,title,msg);
        return;
    }
    QVector<QString> roisShapefiles=mROIsShapefiles.toVector();
    if(!mPtrPointCloudFileManager->createPointCloudFile(projectPath,projectType,gridSize,
                                                        horizontalCrsEpsgCode,verticalCrsEpsgCode,
                                                        roisShapefiles,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Creating project:\n%1\nError:\n%2")
                .arg(projectPath).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    addProjectPath(projectPath);
}

void ccPointCloudTilesFileDlg::loadOrthomosaic()
{
    QString functionName="ccPointCloudTilesFileDlg::loadOrthomosaic";
    QString filename=orthomosaicLineEdit->text();
    if(filename.isEmpty())
    {
        QString title=functionName;
        QString msg=QObject::tr("Select orthomosaic file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    if(!QFile::exists(filename))
    {
        QString title=functionName;
        QString msg=QObject::tr("Not exists orthomosaic:\n%1").arg(filename);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(mPtrCcOrthomosaicContainer)
    {
        bool unload=false;
        unload = (	QMessageBox::question(this,
                                    functionName,
                                    QObject::tr("Unload current orthomosaic and load:\n%1").arg(filename),
                                    QMessageBox::Yes,
                                    QMessageBox::No) == QMessageBox::Yes );
        if(unload)
        {
            bool autoRemove=true;
            mPtrApp->removeFromDB(mPtrCcOrthomosaicContainer, autoRemove);
            //        delete(mPtrCcProjectContainer);
            mPtrCcOrthomosaicContainer=nullptr;
        }
        else return;
    }
    getMinimumCoordinates();
    if(fabs(mMinZ-POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE)<1)
    {
        QString title=functionName;
        QString msg=QObject::tr("Load point cloud files before");
        QMessageBox::information(this,title,msg);
        return;
    }
    bool loadCoordinatesTransEnabled = false;
    FileIOFilter::LoadParameters parameters;
    {
        parameters.alwaysDisplayLoadDialog = true;
        parameters.shiftHandlingMode = ccGlobalShiftManager::DIALOG_IF_NECESSARY;
        parameters._coordinatesShift = mPtrPshift;
        parameters._coordinatesShiftEnabled = &loadCoordinatesTransEnabled;
        parameters.parentWidget = this;
    }
    mPtrCcOrthomosaicContainer=new ccHObject();
    FileIOFilter::ResetSesionCounter();
    unsigned sessionCounter = FileIOFilter::IncreaseSesionCounter();
    parameters.sessionStart = (sessionCounter == 1);
    //global shift
    bool preserveCoordinateShift = true;
    //    double minFc=mPtrPshift->x;
    //    double minSc=mPtrPshift->y;
    //    double minTc=mPtrPshift->z;
    //    CCVector3d Pmin(minFc,minSc,minTc);
    GDALAllRegister();
    GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(qUtf8Printable(filename), GA_ReadOnly ));
    if( poDataset == nullptr )
    {
        delete mPtrCcOrthomosaicContainer;
        mPtrCcOrthomosaicContainer=nullptr;
        QString title=functionName;
        QString msg=QObject::tr("Error opening orthomosaic:\n%1").arg(filename);
        QMessageBox::information(this,title,msg);
        return;
    }
    ccLog::Print(QString("Orthomosaic file: '%1'").arg(filename));
    ccLog::Print(	"Orthomosaic driver: %s/%s",
                    poDataset->GetDriver()->GetDescription(),
                    poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
    int rasterCount = poDataset->GetRasterCount();
    int rasterX     = poDataset->GetRasterXSize();
    int rasterY     = poDataset->GetRasterYSize();
    if (poDataset->GetProjectionRef() != nullptr)
    {
        ccLog::Print("Orthomosaic projection is '%s'", poDataset->GetProjectionRef());
    }
    // See https://gdal.org/user/raster_data_model.html
    // Xgeo = adfGeoTransform(0) + Xpixel * adfGeoTransform(1) + Yline * adfGeoTransform(2)
    // Ygeo = adfGeoTransform(3) + Xpixel * adfGeoTransform(4) + Yline * adfGeoTransform(5)
    double adfGeoTransform[6] = {	0.0, 1.0, 0.0,
                                    0.0, 0.0, 1.0 };
    if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
    {
        ccLog::Print( "Orthomosaic origin = (%.6f,%.6f)", adfGeoTransform[0], adfGeoTransform[3] );
        ccLog::Print( "Orthomosaic pixel Size = (%.6f,%.6f)", adfGeoTransform[1], adfGeoTransform[5] );
    }
    if (adfGeoTransform[1] == 0 || adfGeoTransform[5] == 0)
    {
        ccLog::Warning("Invalid pixel size! Forcing it to (1,1)");
        adfGeoTransform[1] = adfGeoTransform[5] = 1.0;
    }
    const char* aeraOrPoint = poDataset->GetMetadataItem("AREA_OR_POINT");
    ccLog::Print(QString("Pixel Type = ") + (aeraOrPoint ? aeraOrPoint : "AREA")); // Area by default
    //first check if the raster actually has 'color' bands
    int colorBands = 0;
    {
        for (int i = 1; i <= rasterCount; ++i)
        {
            GDALRasterBand* poBand = poDataset->GetRasterBand(i);
            GDALColorInterp colorInterp = poBand->GetColorInterpretation();
            switch (colorInterp)
            {
            case GCI_RedBand:
            case GCI_GreenBand:
            case GCI_BlueBand:
            case GCI_AlphaBand:
                ++colorBands;
                break;
            default:
                break;
            }
        }
    }
    bool loadAsTexturedQuad = true;
    ccPointCloud* pc = new ccPointCloud();
    CCVector3d origin(adfGeoTransform[0], adfGeoTransform[3], 0.0);
    CCVector3d Pshift(0, 0, 0);
    //check for 'big' coordinates
    {
        bool preserveCoordinateShift = true;
        if (FileIOFilter::HandleGlobalShift(origin, Pshift, preserveCoordinateShift, parameters))
        {
            if (pc && preserveCoordinateShift)
            {
                pc->setGlobalShift(Pshift);
            }
            ccLog::Warning("[Orthomosaic::loadFile] Raster has been recentered! Translation: (%.2f ; %.2f ; %.2f)",
                           Pshift.x, Pshift.y, Pshift.z);
        }
    }
    mPtrPshift->x=Pshift.x;
    mPtrPshift->y=Pshift.y;
    mPtrPshift->z=Pshift.z;
    //create blank raster 'grid'
    ccMesh* quad = 0;
    QImage quadTexture;
    quad = new ccMesh(pc);
    quad->addChild(pc);
    pc->setName("vertices");
    pc->setEnabled(false);
    //reserve memory
    quadTexture = QImage(rasterX, rasterY, QImage::Format_RGB32);
    if (!pc->reserve(4) || !quad->reserve(2) || quadTexture.size() != QSize(rasterX, rasterY))
    {
        delete pc;
        delete quad;
        GDALClose(poDataset);
        delete mPtrCcOrthomosaicContainer;
        mPtrCcOrthomosaicContainer=nullptr;
        QString title=functionName;
        QString msg=QObject::tr("Error reserving texture for orthomosaic:\n%1").arg(filename);
        QMessageBox::information(this,title,msg);
        return;
    }
    // B ------ C
    // |        |
    // A ------ D
    CCVector3d B = origin + Pshift; //origin is 'top left'
    CCVector3d C = B + CCVector3d(rasterX * adfGeoTransform[1], rasterX * adfGeoTransform[4], 0);
    CCVector3d A = B + CCVector3d(rasterY * adfGeoTransform[2], rasterY * adfGeoTransform[5], 0);
    CCVector3d D = C + CCVector3d(rasterY * adfGeoTransform[2], rasterY * adfGeoTransform[5], 0);
    A.z=mMinZ+POINTCLOUDFILE_ORTHOMOSAIC_ZMIN_OFFSET;
    B.z=mMinZ+POINTCLOUDFILE_ORTHOMOSAIC_ZMIN_OFFSET;
    C.z=mMinZ+POINTCLOUDFILE_ORTHOMOSAIC_ZMIN_OFFSET;
    D.z=mMinZ+POINTCLOUDFILE_ORTHOMOSAIC_ZMIN_OFFSET;
    pc->addPoint(A.toPC());
    pc->addPoint(B.toPC());
    pc->addPoint(C.toPC());
    pc->addPoint(D.toPC());
    quad->addTriangle(0, 2, 1); //A C B
    quad->addTriangle(0, 3, 2); //A D C
    //fetch raster bands
    bool zRasterProcessed = false;
    CCCoreLib::ReferenceCloud validPoints(pc);
    double zMinMax[2] = { 0, 0 };
    for (int i = 1; i <= rasterCount; ++i)
    {
        ccLog::Print( "Orthomosaic - [GDAL] Reading band #%i", i);
        GDALRasterBand* poBand = poDataset->GetRasterBand(i);
        GDALColorInterp colorInterp = poBand->GetColorInterpretation();

        int nBlockXSize, nBlockYSize;
        poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
        ccLog::Print( "Orthomosaic - [GDAL] Block=%dx%d, Type=%s, ColorInterp=%s",
                      nBlockXSize, nBlockYSize, GDALGetDataTypeName(poBand->GetRasterDataType()),
                      GDALGetColorInterpretationName(colorInterp) );
        //fetching raster scan-line
        int nXSize = poBand->GetXSize();
        int nYSize = poBand->GetYSize();
        assert(nXSize == rasterX);
        assert(nYSize == rasterY);
        int bGotMin, bGotMax;
        double adfMinMax[2] = {0, 0};
        adfMinMax[0] = poBand->GetMinimum( &bGotMin );
        adfMinMax[1] = poBand->GetMaximum( &bGotMax );
        if (!bGotMin || !bGotMax)
        {
            //DGM FIXME: if the file is corrupted (e.g. ASCII ArcGrid with missing rows) this method will enter in a infinite loop!
            GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
        }
        ccLog::Print( "Orthomosaic - [GDAL] Min=%.3fd, Max=%.3f", adfMinMax[0], adfMinMax[1] );
        GDALColorTable* colTable = poBand->GetColorTable();
        if( colTable != nullptr )
            printf( "Orthomosaic - [GDAL] Band has a color table with %d entries", colTable->GetColorEntryCount() );
        if( poBand->GetOverviewCount() > 0 )
            printf( "Orthomosaic - [GDAL] Band has %d overviews", poBand->GetOverviewCount() );
        bool isRGB = false;
        bool isScalar = false;
        bool isPalette = false;
        switch(colorInterp)
        {
        case GCI_Undefined:
            isScalar = true;
            break;
        case GCI_PaletteIndex:
            isPalette = true;
            break;
        case GCI_RedBand:
        case GCI_GreenBand:
        case GCI_BlueBand:
            isRGB = true;
            break;
        case GCI_AlphaBand:
            if (adfMinMax[0] != adfMinMax[1])
            {
                if (loadAsTexturedQuad)
                    isRGB = true;
                else
                    isScalar = true; //we can't load the alpha band as a cloud color (transparency is not handled yet)
            }
            else
            {
                ccLog::Warning(QString("Orthomosaic - Alpha band ignored as it has a unique value (%1)").arg(adfMinMax[0]));
            }
            break;
        default:
            isScalar = true;
            break;
        }
        if (isRGB || isPalette)
        {
            //first check that a palette exists if the band is a palette index
            if (isPalette && !colTable)
            {
                ccLog::Warning(QString("Orthomosaic - Band is declared as a '%1' but no palette is associated!").arg(GDALGetColorInterpretationName(colorInterp)));
            }
            else
            {
                //instantiate memory for RBG colors if necessary
                if (!loadAsTexturedQuad && !pc->hasColors() && !pc->setColor(ccColor::white))
                {
                    ccLog::Warning(QString("Orthomosaic - Failed to instantiate memory for storing color band '%1'!").arg(GDALGetColorInterpretationName(colorInterp)));
                }
                else
                {
                    assert(poBand->GetRasterDataType() <= GDT_Int32);
                    int* colIndexes = (int*)CPLMalloc(sizeof(int)*nXSize);
                    //double* scanline = new double[nXSize];
                    memset(colIndexes, 0, sizeof(int)*nXSize);
                    for (int j = 0; j < nYSize; ++j)
                    {
                        if (poBand->RasterIO( GF_Read, /*xOffset=*/0, /*yOffset=*/j, /*xSize=*/nXSize, /*ySize=*/1, /*buffer=*/colIndexes, /*bufferSizeX=*/nXSize, /*bufferSizeY=*/1, /*bufferType=*/GDT_Int32, /*x_offset=*/0, /*y_offset=*/0 ) != CE_None)
                        {
                            CPLFree(colIndexes);
                            if (quad)
                                delete quad;
                            else
                                delete pc;
                            delete pc;
                            delete quad;
                            GDALClose(poDataset);
                            delete mPtrCcOrthomosaicContainer;
                            mPtrCcOrthomosaicContainer=nullptr;
                            QString title=functionName;
                            QString msg=QObject::tr("Error reading raster band: %1 for orthomosaic:\n%1")
                                    .arg(QString::number(i)).arg(filename);
                            QMessageBox::information(this,title,msg);
                            return;

                        }
                        for (int k = 0; k < nXSize; ++k)
                        {
                            unsigned pointIndex = static_cast<unsigned>(k + j * rasterX);
                            if (loadAsTexturedQuad || pointIndex <= pc->size())
                            {
                                ccColor::Rgba C;
                                if (loadAsTexturedQuad)
                                {
                                    QRgb origColor = quadTexture.pixel(k, j);
                                    C = ccColor::FromQRgba(origColor);
                                }
                                else
                                {
                                    C = pc->getPointColor(pointIndex);
                                }
                                switch (colorInterp)
                                {
                                case GCI_PaletteIndex:
                                    assert(colTable);
                                    {
                                        GDALColorEntry col;
                                        colTable->GetColorEntryAsRGB(colIndexes[k], &col);
                                        C.r = static_cast<ColorCompType>(col.c1 & ccColor::MAX);
                                        C.g = static_cast<ColorCompType>(col.c2 & ccColor::MAX);
                                        C.b = static_cast<ColorCompType>(col.c3 & ccColor::MAX);
                                    }
                                    break;
                                case GCI_RedBand:
                                    C.r = static_cast<ColorCompType>(colIndexes[k] & ccColor::MAX);
                                    break;
                                case GCI_GreenBand:
                                    C.g = static_cast<ColorCompType>(colIndexes[k] & ccColor::MAX);
                                    break;
                                case GCI_BlueBand:
                                    C.b = static_cast<ColorCompType>(colIndexes[k] & ccColor::MAX);
                                    break;
                                case GCI_AlphaBand:
                                    C.a = static_cast<ColorCompType>(colIndexes[k] & ccColor::MAX);
                                    break;
                                default:
                                    assert(false);
                                    break;
                                }
                                if (loadAsTexturedQuad)
                                {
                                    quadTexture.setPixel(k, j, qRgba(C.r, C.g, C.b, C.a));
                                }
                                else
                                {
                                    pc->setPointColor(pointIndex, C);
                                }
                            }
                        }
                    }

                    if (colIndexes)
                        CPLFree(colIndexes);
                    colIndexes = 0;
                }
            }
        }
    }
    ccPlane::SetQuadTexture(quad, quadTexture.mirrored());
    mPtrCcOrthomosaicContainer->addChild(quad);
    GDALClose(poDataset);
    unsigned childCount = mPtrCcOrthomosaicContainer->getChildrenNumber();
    if (childCount != 0)
    {
        //we set the main container name as the full filename (with path)
        //            container->setName(QString("%1").arg(POINT_CLOUD_TILES_FILE_ROIS_LAYER_NAME));
        for (unsigned i = 0; i < childCount; ++i)
        {
            ccHObject* child = mPtrCcOrthomosaicContainer->getChild(i);
            QString newName = child->getName();
            //			if (newName.startsWith("unnamed"))
            //			{
            //				//we automatically replace occurrences of 'unnamed' in entities names by the base filename (no path, no extension)
            //				newName.replace(QString("unnamed"), fi.baseName());
            //				child->setName(newName);
            //			}
            //			else if (newName.isEmpty())
            //			{
            //				//just in case
            //				child->setName(fi.baseName());
            //			}
        }
        //            mPtrApp->addToDB(container, true, true, false);
        QString orthomosaicLayerName=mProjectName+":Orthomosaic";
        mPtrCcOrthomosaicContainer->setName(QString("%1").arg(orthomosaicLayerName));
        mPtrApp->addToDB(mPtrCcOrthomosaicContainer, true, true, false);
        mPtrApp->refreshAll();
        mPtrApp->updateUI();
//        mPtrApp->redrawAll(); // no resuelve el problema de que no se vea al principio
    }
    return;
}

void ccPointCloudTilesFileDlg::onHorizontalCRSsComboBoxCurrentIndexChanged(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::onHorizontalCRSsComboBoxCurrentIndexChanged";
    QString crsId=horizontalCRSsComboBox->currentText();
    if(crsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        return;
    int horizontalCrsEpsgCode;
    QString strAuxError;
    if(!mPtrCrsTools->getCrsEpsgCode(crsId,horizontalCrsEpsgCode,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting CRS EPSG code for CRS Id: %1\nError:\n%2")
                .arg(crsId).arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    setVerticalCRS(horizontalCrsEpsgCode);
    int defaultAddPCFsHorizontalCrsPosition=addPCFsHorizontalCRSsComboBox->findText(crsId);
    if(defaultAddPCFsHorizontalCrsPosition!=-1)
        addPCFsHorizontalCRSsComboBox->setCurrentIndex(defaultAddPCFsHorizontalCrsPosition);
    int defaultPpToolsIPFCsHorizontalCrsPosition=ppToolsIPCFsHorizontalCRSsComboBox->findText(crsId);
    if(defaultPpToolsIPFCsHorizontalCrsPosition!=-1)
        ppToolsIPCFsHorizontalCRSsComboBox->setCurrentIndex(defaultPpToolsIPFCsHorizontalCrsPosition);
}

void ccPointCloudTilesFileDlg::onVerticalCRSsComboBoxCurrentIndexChanged(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::onVerticalCRSsComboBoxCurrentIndexChanged";
    QString verticalCrsId=verticalCRSsComboBox->currentText();
    int defaultAddPCFsVerticalCrsPosition=addPCFsVerticalCRSsComboBox->findText(verticalCrsId);
    if(defaultAddPCFsVerticalCrsPosition!=-1)
        addPCFsVerticalCRSsComboBox->setCurrentIndex(defaultAddPCFsVerticalCrsPosition);
    int defaultPpToolsIPFCsVerticalCrsPosition=ppToolsIPCFsVerticalCRSsComboBox->findText(verticalCrsId);
    if(defaultPpToolsIPFCsVerticalCrsPosition!=-1)
        ppToolsIPCFsVerticalCRSsComboBox->setCurrentIndex(defaultPpToolsIPFCsVerticalCrsPosition);
//    if(verticalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
//        return;
}

void ccPointCloudTilesFileDlg::onAddPCFsHorizontalCRSsComboBoxCurrentIndexChanged(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::onAddPCFsHorizontalCRSsComboBoxCurrentIndexChanged";
    QString crsId=addPCFsHorizontalCRSsComboBox->currentText();
    if(crsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        return;
    int horizontalCrsEpsgCode;
    QString strAuxError;
    if(!mPtrCrsTools->getCrsEpsgCode(crsId,horizontalCrsEpsgCode,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting CRS EPSG code for CRS Id: %1\nError:\n%2")
                .arg(crsId).arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    setAddPCFsVerticalCRS(horizontalCrsEpsgCode);
}

void ccPointCloudTilesFileDlg::onModelDbsComboBoxCurrentIndexChanged(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::onModelDbsComboBoxCurrentIndexChanged";
    QString modelDb=modelDbsComboBox->currentText();
    if(modelDb.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        addModelDbPushButton->setEnabled(true);
        openModelDbPushButton->setEnabled(false);
        closeModelDbPushButton->setEnabled(false);
        removeModelDbPushButton->setEnabled(false);
        mVisibleModelClassesComboCheckBox->clear();
        mVisibleModelClassesComboCheckBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_CLASSES_TITLE);
        mVisibleModelClassesComboCheckBox->setEnabled(false);
        mVisibleModelObjectsComboCheckBox->clear();
        mVisibleModelObjectsComboCheckBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_OBJECTS_TITLE);
        mVisibleModelObjectsComboCheckBox->setEnabled(false);
        visibleModelObjectsNonePushButton->setEnabled(false);
        visibleModelObjectsAllPushButton->setEnabled(false);
        modelDbObjectComboBox->clear();
        modelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
        modelDbObjectComboBox->setEnabled(false);
        modelDbObjectCandidateComboBox->clear();
//        modelDbObjectCandidateComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
        modelDbObjectCandidateComboBox->setEnabled(false);
        showObjectNamesCheckBox->setChecked(false);
        showObjectNamesCheckBox->setEnabled(false);
    }
    else
    {
        addModelDbPushButton->setEnabled(false);
        openModelDbPushButton->setEnabled(true);
        closeModelDbPushButton->setEnabled(false);
        removeModelDbPushButton->setEnabled(true);
        mVisibleModelClassesComboCheckBox->clear();
        mVisibleModelClassesComboCheckBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_CLASSES_TITLE);
        mVisibleModelClassesComboCheckBox->setEnabled(false);
        mVisibleModelObjectsComboCheckBox->clear();
        mVisibleModelObjectsComboCheckBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_OBJECTS_TITLE);
        mVisibleModelObjectsComboCheckBox->setEnabled(false);
        modelDbObjectComboBox->clear();
        modelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
        modelDbObjectComboBox->setEnabled(false);
        modelDbObjectCandidateComboBox->clear();
//        modelDbObjectCandidateComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
        modelDbObjectCandidateComboBox->setEnabled(false);
    }
}

void ccPointCloudTilesFileDlg::onPpToolsIPCFsHorizontalCRSsComboBoxCurrentIndexChanged(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::onPpToolsIPCFsHorizontalCRSsComboBoxCurrentIndexChanged";
    QString crsId=ppToolsIPCFsHorizontalCRSsComboBox->currentText();
    if(crsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        return;
    int horizontalCrsEpsgCode;
    QString strAuxError;
    if(!mPtrCrsTools->getCrsEpsgCode(crsId,horizontalCrsEpsgCode,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting CRS EPSG code for CRS Id: %1\nError:\n%2")
                .arg(crsId).arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    setPpToolsIPFCsVerticalCRS(horizontalCrsEpsgCode);
}

void ccPointCloudTilesFileDlg::onProjectsComboBoxCurrentIndexChanged(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::onProjectsComboBoxCurrentIndexChanged";
    QString project=projectsComboBox->currentText();
    if(project.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        addProjectPushButton->setEnabled(true);
        openProjectPushButton->setEnabled(false);
        closeProjectPushButton->setEnabled(false);
        removeProjectPushButton->setEnabled(false);
        QString horizontalCrsId=POINT_CLOUD_TILES_FILE_DIALOG_DEFAULT_CRS;
        int posHorizontalCrsId=horizontalCRSsComboBox->findText(horizontalCrsId);
        horizontalCRSsComboBox->setCurrentIndex(posHorizontalCrsId);
        QString verticalCrsId=POINT_CLOUD_TILES_FILE_DIALOG_DEFAULT_VERTICAL_CRS;
        int posVerticalCrsId=verticalCRSsComboBox->findText(verticalCrsId);
        verticalCRSsComboBox->setCurrentIndex(posVerticalCrsId);
        projectPathLineEdit->setText("");
        projectManagementTabWidget->setTabEnabled(0,true);
        projectManagementTabWidget->setTabEnabled(1,false);
        projectManagementTabWidget->setTabEnabled(2,false);
        projectManagementTabWidget->setCurrentIndex(0);
    }
    else
    {
        addProjectPushButton->setEnabled(false);
        openProjectPushButton->setEnabled(true);
        closeProjectPushButton->setEnabled(false);
        removeProjectPushButton->setEnabled(true);
        projectManagementTabWidget->setTabEnabled(0,false);
        projectManagementTabWidget->setTabEnabled(1,false);
        projectManagementTabWidget->setTabEnabled(2,false);
        projectManagementTabWidget->setCurrentIndex(0);
    }
}

void ccPointCloudTilesFileDlg::openProject()
{
    toolBox->setItemEnabled(1,false);
    QString functionName="ccPointCloudTilesFileDlg::openProject";
    QString strAuxError;
    QString projectPath=projectsComboBox->currentText();
    if(projectPath.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select project before");
        QMessageBox::information(this,title,msg);
        return;
    }
    if (QDir(projectPath).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Selected project path is empty:\n%1").arg(projectPath);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(!mPtrPointCloudFileManager->openPointCloudFile(projectPath,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Opening project path:\n%1\nError:\n%2")
                .arg(projectPath).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    int horizontalCrsEpsgCode,verticalCrsEpsgCode;
    if(!mPtrPointCloudFileManager->getProjectCrsEpsgCodes(projectPath,horizontalCrsEpsgCode,
                                                          verticalCrsEpsgCode,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting crs for project path:\n%1\nError:\n%2")
                .arg(projectPath).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString horizontalCrsId=POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX+QString::number(horizontalCrsEpsgCode);
//    {
//        QString title=functionName;
//        QString msg=QObject::tr("Horizontal CRS id: %1").arg(horizontalCrsId);
//        QMessageBox::information(this,title,msg);
//    }
    int posHorizontalCrsId=horizontalCRSsComboBox->findText(horizontalCrsId);
    horizontalCRSsComboBox->setCurrentIndex(posHorizontalCrsId);
    QString verticalCrsId=POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT;
    if(verticalCrsEpsgCode!=-1)
    {
        verticalCrsId=POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX+QString::number(verticalCrsEpsgCode);
    }
    int posVerticalCrsId=verticalCRSsComboBox->findText(verticalCrsId);
    verticalCRSsComboBox->setCurrentIndex(posVerticalCrsId);
    projectPathLineEdit->setText(projectPath);
    addProjectPushButton->setEnabled(false);
    openProjectPushButton->setEnabled(false);
    closeProjectPushButton->setEnabled(true);
    removeProjectPushButton->setEnabled(false);
    projectManagementTabWidget->setTabEnabled(0,false);
    projectManagementTabWidget->setTabEnabled(1,true);
    projectManagementTabWidget->setTabEnabled(2,true);
    projectManagementTabWidget->setCurrentIndex(1);
    projectsComboBox->setEnabled(false);
    addPCFsHorizontalCRSsComboBox->setEnabled(false);
    addPCFsVerticalCRSsComboBox->setEnabled(false);
    mProjectName=QDir(projectPath).dirName();
    mHorizontalCrsEpsgCode=horizontalCrsEpsgCode;
    mVerticalCrsEpsgCode=verticalCrsEpsgCode;
    loadROIsLayer();
    loadTilesLayer();
    toolBox->setItemEnabled(1,true);
}

void ccPointCloudTilesFileDlg::ppToolsAddProcessToList()
{
    QString functionName="ccPointCloudTilesFileDlg::ppToolsAddProcessToList";
    QString strAuxError;
    QString horizontalCrsId=ppToolsIPCFsHorizontalCRSsComboBox->currentText();
    if(horizontalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select horizontal CRS before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int horizontalCrsEpsgCode;
    if(!mPtrCrsTools->getCrsEpsgCode(horizontalCrsId,horizontalCrsEpsgCode,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting EPSG code for horizontal CRS: %1\nError:\n%2")
                .arg(horizontalCrsId).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString verticalCrsId=ppToolsIPCFsVerticalCRSsComboBox->currentText();
    if(verticalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select vertical CRS before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int verticalCrsEpsgCode=-1;
    if(verticalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT,Qt::CaseInsensitive)!=0)
    {
        QString strVerticalCrsEpsgCode=verticalCrsId.remove(POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX);
        verticalCrsEpsgCode=strVerticalCrsEpsgCode.toInt();
    }
    if(mPpToolsPCFiles.size()==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select input files before");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString outputFile = ppToolsOPCFsOutputFileLineEdit->text();
    QString outputPath = ppToolsOPCFsOutputPathLineEdit->text();
    QString suffixOutputFiles = ppToolsOPCFsSuffixLineEdit->text();
    QString prefixOutputFiles = ppToolsOPCFsPrefixLineEdit->text();
//    # ¿se puede ignorar para algunos procesos si hay varios ficheros de entrada? lasmerge ...
//    # tengo que definir en un contenedor aquellos comandos que lo permiten
    if(mPpToolsPCFiles.size()>1&&!outputFile.isEmpty())
    {
        if(ppToolsOPCFsOutputPathPushButton->isEnabled())
        {
            if(outputPath.isEmpty()&&suffixOutputFiles.isEmpty())
            {
                QString title=functionName;
                QString msg=QObject::tr("Select output path or suffix for several input files");
                QMessageBox::information(this,title,msg);
                return;
            }
        }
    }
    if(outputFile.isEmpty()&&outputPath.isEmpty()&&suffixOutputFiles.isEmpty())
    {
        QString title=functionName;
        QString msg=QObject::tr("Select output file, output path or suffix");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString command;
    if(ppToolsTabWidget->currentIndex()==0) // Lastools command
    {
        if(mLastoolsPath.isEmpty())
        {
            QString title=functionName;
            QString msg=QObject::tr("Select LAStools path");
            QMessageBox::information(this,title,msg);
            return;
        }
        command=ppToolsLastoolsCommandComboBox->currentText();
        if(command.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT)==0)
        {
            QString title=functionName;
            QString msg=QObject::tr("Select LAStools command");
            QMessageBox::information(this,title,msg);
            return;
        }
        QVector<QString> qtInputFiles;
        for (int i = 0; i < mPpToolsPCFiles.size() ; i++)
        {
            QString qtInputFile = mPpToolsPCFiles[i];
            qtInputFiles.push_back(qtInputFile);
        }
        QVector<QString> lastoolsCommandStrings;
        if(!mPtrPointCloudFileManager->getLastoolsCommandStrings(command,
                                                                 qtInputFiles,
                                                                 outputPath,
                                                                 outputFile,
                                                                 suffixOutputFiles,
                                                                 prefixOutputFiles,
                                                                 lastoolsCommandStrings,
                                                                 strAuxError))
        {
            QString title=functionName;
            QString msg=QObject::tr("Error getting lastool command string:\nError:\n%1").arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
        mProcessList.clear();
        for(int i=0;i<lastoolsCommandStrings.size();i++)
            mProcessList.append(lastoolsCommandStrings[i]);
    }
    else if(ppToolsTabWidget->currentIndex()==1) // internal command
    {
        command=ppToolsInternalCommandComboBox->currentText();
        if(command.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT)==0)
        {
            QString title=functionName;
            QString msg=QObject::tr("Select internal command");
            QMessageBox::information(this,title,msg);
            return;
        }
        {
            QString title=functionName;
            QString msg=QObject::tr("Invalid option for internal commands");
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    if(command.isEmpty())
    {
        QString title=functionName;
        QString msg=QObject::tr("Select command");
        QMessageBox::information(this,title,msg);
        return;
    }

    return;
}

void ccPointCloudTilesFileDlg::ppToolsTabWidgetChanged(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::ppToolsTabWidgetChanged";
    QString strAuxError;
    if(index==0)
    {
        addProcessToListPushButton->setVisible(true);
        processListEditionPushButton->setVisible(true);
        runProcessListPushButton->setText(POINT_CLOUD_TILES_FILE_DIALOG_PROCESSING_TOOLS_RUN_BUTTON_PROCESS_LIST_TEXT);
    }
    else if(index==1)
    {
        addProcessToListPushButton->setVisible(false);
        processListEditionPushButton->setVisible(false);
        runProcessListPushButton->setText(POINT_CLOUD_TILES_FILE_DIALOG_PROCESSING_TOOLS_RUN_BUTTON_PROCESS_TEXT);
    }
    return;
}

void ccPointCloudTilesFileDlg::ppToolsProcessListEdition()
{
    QString functionName="ccPointCloudTilesFileDlg::ppToolsProcessListEdition";
    QString strAuxError;
    if(mProcessList.size()==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Add process to list before");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString windowTitle=POINT_CLOUD_TILES_FILE_DIALOG_PROCESSING_TOOLS_PROCESS_LIST_EDITION_DIALOG_TITLE;
    ProcessListEditionDialog dlg(windowTitle,
                                 mProcessList,
                                 this);
    dlg.exec();
    mProcessList=dlg.getProcessList();
}

void ccPointCloudTilesFileDlg::ppToolsRunProcessList()
{
    QString functionName="ccPointCloudTilesFileDlg::ppToolsRunProcessList";
    QString strAuxError;
    if(ppToolsTabWidget->currentIndex()==0)
    {
        if(mProcessList.size()==0)
        {
            QString title=functionName;
            QString msg=QObject::tr("Process list is empty");
            QMessageBox::information(this,title,msg);
            return;
        }
        QString title = POINT_CLOUD_TILES_FILE_DIALOG_PROCESSING_TOOLS_RUN_BUTTON_PROCESS_LIST_TEXT;
        QVector<QString> processList;
        for(int i=0;i<mProcessList.size();i++)
            processList.push_back(mProcessList[i]);
        if(!mPtrPointCloudFileManager->runProcessList(processList,title,strAuxError))
        {
            QString title=functionName;
            QString msg=QObject::tr("Error running process list:\nError:\n%1").arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    else if(ppToolsTabWidget->currentIndex() == 1)
    {
        QString title=functionName;
        QString msg=QObject::tr("This option is disabled");
        QMessageBox::information(this,title,msg);
        return;
    }
}

void ccPointCloudTilesFileDlg::ppToolsSelectLastoolsCommandParameters()
{
    QString functionName="ccPointCloudTilesFileDlg::ppToolsSelectLastoolsCommandParameters";
    QString strAuxError;
    QString command=ppToolsLastoolsCommandComboBox->currentText();
    if(command.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        return;
    }
    if(!mPtrPointCloudFileManager->selectLastoolsCommandParameters(command,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error selecting parameters for command:\n%1\nError:\n%2")
                .arg(command).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
}

void ccPointCloudTilesFileDlg::ppToolsSelectInternalCommandParameters()
{
    QString functionName="ccPointCloudTilesFileDlg::ppToolsSelectInternalCommandParameters";
    QString strAuxError;
    QString command=ppToolsInternalCommandComboBox->currentText();
    if(command.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        return;
    }
    if(!mPtrPointCloudFileManager->selectInternalCommandParameters(command,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error selecting parameters for command:\n%1\nError:\n%2")
                .arg(command).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
}

void ccPointCloudTilesFileDlg::removeProject()
{
    QString functionName="ccPointCloudTilesFileDlg::removeProject";
    QString strAuxError;
    QString projectPath=projectsComboBox->currentText();
    if(projectPath.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select project before");
        QMessageBox::information(this,title,msg);
        return;
    }
//    if(!mPtrPointCloudFileManager->openPointCloudFile(projectPath,strAuxError))
//    {
//        QString title=functionName;
//        QString msg=QObject::tr("Opening project path:\n%1\nError:\n%2")
//                .arg(projectPath).arg(strAuxError);
//        QMessageBox::information(this,title,msg);
//        return;
//    }
    int pos=mProjects.indexOf(projectPath);
    mProjects.removeAt(pos);
    QString strProjects;
    projectsComboBox->clear();
    projectsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    for(int i=0;i<mProjects.size();i++)
    {
        if(!strProjects.isEmpty()) strProjects+=POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS;
        strProjects+=mProjects[i];
        projectsComboBox->addItem(mProjects[i]);
    }
    mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_PROJECTS,strProjects);
    mPtrSettings->sync();
    projectsComboBox->setEnabled(true);
    projectsComboBox->setCurrentIndex(0);
//    addProjectPushButton->setEnabled(false);
//    openProjectPushButton->setEnabled(false);
//    closeProjectPushButton->setEnabled(true);
//    removeProjectPushButton->setEnabled(false);
//    projectManagementTabWidget->setTabEnabled(0,false);
//    projectManagementTabWidget->setTabEnabled(1,true);
//    projectManagementTabWidget->setTabEnabled(2,true);
    //    projectManagementTabWidget->setCurrentIndex(1);
}

void ccPointCloudTilesFileDlg::ppToolsSelectInternalCommand(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::selectInternalCommand";
    QString strAuxError;
    ppToolsOPCFsOutputFilePushButton->setEnabled(false);
    ppToolsOPCFsOutputFileLineEdit->setText("");
    ppToolsOPCFsOutputPathPushButton->setEnabled(false);
    ppToolsOPCFsOutputPathLineEdit->setText("");
    ppToolsOPCFsSuffixPushButton->setEnabled(false);
    ppToolsOPCFsSuffixLineEdit->setText("");
    ppToolsOPCFsPrefixPushButton->setEnabled(false);
    ppToolsOPCFsPrefixLineEdit->setText("");
    QString internalCommand = ppToolsInternalCommandComboBox->currentText();
    if(internalCommand.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT)==0)
    {
        ppToolsInternalCommandParametersPushButton->setEnabled(false);
        return;
    }
    bool enableOutputPath=false;
    bool enableOutputFile=false;
    bool enableSuffix=false;
    bool enablePrefix=false;
    if(!mPtrPointCloudFileManager->getInternalCommandOutputDataFormat(internalCommand,
                                                                      enableOutputPath,
                                                                      enableOutputFile,
                                                                      enableSuffix,
                                                                      enablePrefix,
                                                                      strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error getting internal command output format:\nError:\n%1").arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(enableOutputPath)
        ppToolsOPCFsOutputPathPushButton->setEnabled(true);
    if(enableOutputFile)
        ppToolsOPCFsOutputFilePushButton->setEnabled(true);
    if(enableSuffix)
        ppToolsOPCFsSuffixPushButton->setEnabled(true);
    if(enablePrefix)
        ppToolsOPCFsPrefixPushButton->setEnabled(true);
    ppToolsInternalCommandParametersPushButton->setEnabled(true);
    return;
}

void ccPointCloudTilesFileDlg::ppToolsSelectLastoolsCommand(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::selectLastoolsCommand";
    QString strAuxError;
    ppToolsOPCFsOutputFilePushButton->setEnabled(false);
    ppToolsOPCFsOutputFileLineEdit->setText("");
    ppToolsOPCFsOutputPathPushButton->setEnabled(false);
    ppToolsOPCFsOutputPathLineEdit->setText("");
    ppToolsOPCFsSuffixPushButton->setEnabled(false);
    ppToolsOPCFsSuffixLineEdit->setText("");
    ppToolsOPCFsPrefixPushButton->setEnabled(false);
    ppToolsOPCFsPrefixLineEdit->setText("");
    QString lastoolsCommand = ppToolsLastoolsCommandComboBox->currentText();
    if(lastoolsCommand.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT)==0)
    {
        ppToolsLastoolsCommandParametersPushButton->setEnabled(false);
        return;
    }
    bool enableOutputPath=false;
    bool enableOutputFile=false;
    bool enableSuffix=false;
    bool enablePrefix=false;
    if(!mPtrPointCloudFileManager->getLastoolsCommandsOutputDataFormat(lastoolsCommand,
                                                                     enableOutputPath,
                                                                     enableOutputFile,
                                                                     enableSuffix,
                                                                     enablePrefix,
                                                                     strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error getting lastool command output format:\nError:\n%1").arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(enableOutputPath)
        ppToolsOPCFsOutputPathPushButton->setEnabled(true);
    if(enableOutputFile)
        ppToolsOPCFsOutputFilePushButton->setEnabled(true);
    if(enableSuffix)
        ppToolsOPCFsSuffixPushButton->setEnabled(true);
    if(enablePrefix)
        ppToolsOPCFsPrefixPushButton->setEnabled(true);
    ppToolsLastoolsCommandParametersPushButton->setEnabled(true);
    return;
}

void ccPointCloudTilesFileDlg::ppToolsSelectLastoolsPath()
{
    QString functionName="ccPointCloudTilesFileDlg::selectLastoolsPath";
    QString oldLastoolsPath=ppToolsLastoolsPathLineEdit->text();
    QString selectedPath=mLastPath;
    selectedPath = QFileDialog::getExistingDirectory(this,
                                                     QObject::tr("Select LAStools path"),
                                                     mLastPath,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (!selectedPath.isEmpty())
    {
        if(selectedPath.compare(oldLastoolsPath,Qt::CaseInsensitive)!=0)
        {
            ppToolsLastoolsPathLineEdit->setText(selectedPath);
            mLastoolsPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LASTOOLS_PATH,mLastoolsPath);
            if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
            {
                mLastPath=selectedPath;
                mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
                mPtrSettings->sync();
            }
        }
    }
}

void ccPointCloudTilesFileDlg::ppToolsSelectOutputFile()
{
    QString functionName="ccPointCloudTilesFileDlg::ppToolsSelectOutputFile";
    QString oldFile=ppToolsOPCFsOutputFileLineEdit->text();
    QString selectedPath=mLastPath;
    QString file = QFileDialog::getSaveFileName(this,
                                                tr("Select output file"),
                                                mLastPath,
                                                tr("GeoTIFF Files/LAS/LAZ (*.tif,*.las,*.laz)"));
    if (!file.isEmpty()
            &&file.compare(oldFile,Qt::CaseInsensitive)!=0)
    {
        ppToolsOPCFsOutputFileLineEdit->setText(file);
        QString selectedPath=QFileInfo(file).absolutePath();
        if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
        {
            mLastPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
            mPtrSettings->sync();
        }
//        mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_ORTHOMOSAIC,orthomosaicFileName);
//        mPtrSettings->sync();
    }
}

void ccPointCloudTilesFileDlg::ppToolsSelectOutputPath()
{
    QString functionName="ccPointCloudTilesFileDlg::selectPpToolsOutputPath";
    QString oldPath=ppToolsOPCFsOutputPathLineEdit->text();
    QString selectedPath=mLastPath;
    selectedPath = QFileDialog::getExistingDirectory(this,
                                                     QObject::tr("Select processing tools output path"),
                                                     mLastPath,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (!selectedPath.isEmpty())
    {
        if(selectedPath.compare(oldPath,Qt::CaseInsensitive)!=0)
        {
            ppToolsOPCFsOutputPathLineEdit->setText(selectedPath);
            mLastoolsPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LASTOOLS_PATH,mLastoolsPath);
            if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
            {
                mLastPath=selectedPath;
                mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
                mPtrSettings->sync();
            }
        }
    }
}

void ccPointCloudTilesFileDlg::ppToolsSelectPCFiles()
{
    QString functionName="ccPointCloudTilesFileDlg::selectPpToolsPCFiles";
    if(!mPtrPpToolsPCFsMultipleFileSelectorDialog)
    {
        QString title=QObject::tr("Select Point Cloud Files");
        QStringList fileTypes;
        fileTypes.append("las");
        fileTypes.append("laz");
        mPtrPpToolsPCFsMultipleFileSelectorDialog=new MultipleFileSelectorDialog(title,mLastPath,fileTypes,this);
    }
    else
    {
        mPtrPpToolsPCFsMultipleFileSelectorDialog->exec();
    }
    mPpToolsPCFiles.clear();
    mPpToolsPCFiles=mPtrPpToolsPCFsMultipleFileSelectorDialog->getFiles();
    QString selectedPath=mPtrPpToolsPCFsMultipleFileSelectorDialog->getPath();
    ppToolsNumberOfIPCFsLineEdit->setText(QString::number(mPpToolsPCFiles.size()));
}

void ccPointCloudTilesFileDlg::ppToolsSelectSuffix()
{
    QString functionName="ccPointCloudTilesFileDlg::selectPpToolsSuffix";
    QString oldValue=ppToolsOPCFsSuffixLineEdit->text().trimmed();
    QString title=tr("Processing tools output suffix");
    QString name=tr("Suffix:");
    bool ok=false;
    while(!ok)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,oldValue,&ok);
        if(ok)
        {
            inputStrValue=inputStrValue.trimmed();
            if(inputStrValue.compare(oldValue,Qt::CaseSensitive)!=0)
            {
                ppToolsOPCFsSuffixLineEdit->setText(inputStrValue);
            }
        }
    }
}

void ccPointCloudTilesFileDlg::ppToolsSelectPrefix()
{
    QString functionName="ccPointCloudTilesFileDlg::selectPpToolsPreffix";
    QString oldValue=ppToolsOPCFsPrefixLineEdit->text().trimmed();
    QString title=tr("Processing tools output prefix");
    QString name=tr("Prefix:");
    bool ok=false;
    while(!ok)
    {
        QString inputStrValue=QInputDialog::getText(this, title,name,QLineEdit::Normal,oldValue,&ok);
        if(ok)
        {
            inputStrValue=inputStrValue.trimmed();
            if(inputStrValue.compare(oldValue,Qt::CaseSensitive)!=0)
            {
                ppToolsOPCFsPrefixLineEdit->setText(inputStrValue);
            }
        }
    }
}

void ccPointCloudTilesFileDlg::selectOrthomosaic()
{
    QString functionName="ccPointCloudTilesFileDlg::selectOrthomosaic";
    QString oldOrthomosaicFileName=orthomosaicLineEdit->text();
    QString orthomosaicFileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select orthomosaic file"),
                                                    mLastPath,
                                                    tr("GeoTIFF Files (*.tif)"));
    if (!orthomosaicFileName.isEmpty()
            &&orthomosaicFileName.compare(oldOrthomosaicFileName,Qt::CaseInsensitive)!=0)
    {
        orthomosaicLineEdit->setText(orthomosaicFileName);
        QString selectedPath=QFileInfo(orthomosaicFileName).absolutePath();
        if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
        {
            mLastPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
            mPtrSettings->sync();
        }
        mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_ORTHOMOSAIC,orthomosaicFileName);
        mPtrSettings->sync();
    }
}

void ccPointCloudTilesFileDlg::selectOutputPath()
{
    QString functionName="ccPointCloudTilesFileDlg::selectOutputPath";
    QString oldOutputPath=pmOutputPathLineEdit->text();
    QString selectedPath=mLastPath;
    selectedPath = QFileDialog::getExistingDirectory(this,
                                                     QObject::tr("Select output path"),
                                                     mLastPath,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (!selectedPath.isEmpty())
    {
        if(selectedPath.compare(oldOutputPath,Qt::CaseInsensitive)!=0)
        {
            pmOutputPathLineEdit->setText(selectedPath);
            mOutputPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_TEMP_PATH,mOutputPath);
            if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
            {
                mLastPath=selectedPath;
                mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
                mPtrSettings->sync();
            }
        }
    }
}

void ccPointCloudTilesFileDlg::selectPCFs()
{
    QString functionName="ccPointCloudTilesFileDlg::selectPCFs";
    if(!mPtrPCFsMultipleFileSelectorDialog)
    {
        QString title=QObject::tr("Select Point Cloud Files");
        QStringList fileTypes;
        fileTypes.append("las");
        fileTypes.append("laz");
        mPtrPCFsMultipleFileSelectorDialog=new MultipleFileSelectorDialog(title,mLastPath,fileTypes,this);
    }
    else
    {
        mPtrPCFsMultipleFileSelectorDialog->exec();
    }
    mPCFiles.clear();
    mPCFiles=mPtrPCFsMultipleFileSelectorDialog->getFiles();
    QString selectedPath=mPtrPCFsMultipleFileSelectorDialog->getPath();
    numberOfPCFsLineEdit->setText(QString::number(mPCFiles.size()));
}

void ccPointCloudTilesFileDlg::selectProjectParameters()
{
    QString functionName="ccPointCloudTilesFileDlg::selectProjectParameters";
    QString projectType=projectTypeComboBox->currentText();
    if(projectType.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select project type before");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString strAuxError;
    if(!mPtrPointCloudFileManager->selectProjectParameters(projectType,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Selecting project parameters:\nError:\n%1").arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
}

void ccPointCloudTilesFileDlg::selectProjectPath()
{
    QString functionName="ccPointCloudTilesFileDlg::selectProjectPath";
    QString selectedPath=mLastPath;
    selectedPath = QFileDialog::getExistingDirectory(this,
                                                     QObject::tr("Select Project path"),
                                                     mLastPath,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (!selectedPath.isEmpty())
    {
        if (QDir(selectedPath).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() != 0)
        {
            QString title=functionName;
            QString msg=QObject::tr("Selected project path must be empty:\n%1").arg(selectedPath);
            QMessageBox::information(this,title,msg);
            return;
        }
        if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
        {
            mLastPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
            mPtrSettings->sync();
        }
        projectPathLineEdit->setText(selectedPath);
    }
}

void ccPointCloudTilesFileDlg::selectROIs()
{
    QString functionName="ccPointCloudTilesFileDlg::selectROIs";
    if(!mPtrROIsMultipleFileSelectorDialog)
    {
        QString title=QObject::tr("Select ROIs shapefiles");
        QStringList fileTypes;
        fileTypes.append("shp");
        mPtrROIsMultipleFileSelectorDialog=new MultipleFileSelectorDialog(title,mLastPath,fileTypes,this);
    }
    else
    {
        mPtrROIsMultipleFileSelectorDialog->show();
    }
    mROIsShapefiles.clear();
    mROIsShapefiles=mPtrROIsMultipleFileSelectorDialog->getFiles();
    QString selectedPath=mPtrROIsMultipleFileSelectorDialog->getPath();
    numberOfRoisLineEdit->setText(QString::number(mROIsShapefiles.size()));
}

void ccPointCloudTilesFileDlg::selectTempPath()
{
    QString functionName="ccPointCloudTilesFileDlg::selectTempPath";
    QString oldTempPath=pmTemporalPathLineEdit->text();
    QString selectedPath=mLastPath;
    selectedPath = QFileDialog::getExistingDirectory(this,
                                                     QObject::tr("Select temporal path"),
                                                     mLastPath,
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (!selectedPath.isEmpty())
    {
        if(selectedPath.compare(oldTempPath,Qt::CaseInsensitive)!=0)
        {
            pmTemporalPathLineEdit->setText(selectedPath);
            mTempPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_TEMP_PATH,mTempPath);
            if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
            {
                mLastPath=selectedPath;
                mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
                mPtrSettings->sync();
            }
        }
    }
}

void ccPointCloudTilesFileDlg::showObjectNames(int value)
{
    bool showObjectNames=showObjectNamesCheckBox->isChecked();
    QMap<int,ccHObject*>::iterator iterCubes=mPtrCcCubeContainerByCubeDbId.begin();
    while(iterCubes!=mPtrCcCubeContainerByCubeDbId.end())
    {
        ccHObject* ptrCcCubeConteiner=iterCubes.value();
        if(ptrCcCubeConteiner!=nullptr)
        {
            ptrCcCubeConteiner->showNameIn3D(showObjectNames);
//            mPtrApp->emit(ccObjectAppearanceChanged(ptrCcCubeConteiner));
//            Q_EMIT ccObjectAppearanceChanged(ptrCcCubeConteiner);
        }
        iterCubes++;
    }
//    mPtrApp->refreshAll(true);
    mPtrApp->redrawAll();
}

void ccPointCloudTilesFileDlg::editStartClicked()
{
    if (nullptr == mPtrApp->getActiveGLWindow())
    {
        return;
    }

    mPtrApp->getActiveGLWindow()->setPickingMode(ccGLWindow::PICKING_MODE::NO_PICKING);

    //set orthographic view (as this tool doesn't work in perspective mode)
    mPtrApp->getActiveGLWindow()->setPerspectiveState(false, true);
    mPtrApp->getActiveGLWindow()->setInteractionMode(ccGLWindow::INTERACT_SEND_ALL_SIGNALS);
    connect(m_associatedWin, &ccGLWindow::leftButtonClicked,	this, &ccPointCloudTilesFileDlg::addPointToPolyline);
    connect(m_associatedWin, &ccGLWindow::rightButtonClicked,	this, &ccPointCloudTilesFileDlg::closePolyLine);
    connect(m_associatedWin, &ccGLWindow::mouseMoved,			this, &ccPointCloudTilesFileDlg::updatePolyLine);
    connect(m_associatedWin, &ccGLWindow::buttonReleased,		this, &ccPointCloudTilesFileDlg::closeRectangle);
    if (m_selectionTilesPoly)
    {
        m_selectionTilesPoly->setDisplay(m_associatedWin);
    }
    loadTilesPushButton->setEnabled(true);
    unloadTilesPushButton->setEnabled(true);
//    unloadTilesAllPushButton->setEnabled(true);
    m_pointsEditionState = STARTED;

    pbEditStart->setEnabled(false);
    pbEditPause->setEnabled(true);
    rbEditPolygon->setEnabled(false);
    rbEditRectangle->setEnabled(false);
}

void ccPointCloudTilesFileDlg::editPauseClicked()
{
    if (nullptr == mPtrApp->getActiveGLWindow())
    {
        return;
    }
    stopRunning();
    m_pointsEditionState = PAUSED;
    if (m_selectionTilesPolyVertices->size() != 0)
    {
        m_selectionTilesPoly->clear();
        m_selectionTilesPolyVertices->clear();
        //			allowPolylineExport(false);
    }
    disconnect(m_associatedWin, &ccGLWindow::leftButtonClicked,	this, &ccPointCloudTilesFileDlg::addPointToPolyline);
    disconnect(m_associatedWin, &ccGLWindow::rightButtonClicked,	this, &ccPointCloudTilesFileDlg::closePolyLine);
    disconnect(m_associatedWin, &ccGLWindow::mouseMoved,			this, &ccPointCloudTilesFileDlg::updatePolyLine);
    disconnect(m_associatedWin, &ccGLWindow::buttonReleased,		this, &ccPointCloudTilesFileDlg::closeRectangle);
    if (m_selectionTilesPoly)
    {
        m_selectionTilesPoly->setDisplay(nullptr);
    }
    loadTilesPushButton->setEnabled(false);
    unloadTilesPushButton->setEnabled(false);
//    unloadTilesAllPushButton->setEnabled(false);

    mPtrApp->getActiveGLWindow()->setPickingMode(ccGLWindow::PICKING_MODE::DEFAULT_PICKING);
    mPtrApp->getActiveGLWindow()->setInteractionMode(ccGLWindow::MODE_TRANSFORM_CAMERA);
    mPtrApp->getActiveGLWindow()->redraw(true, false);

    pbEditStart->setEnabled(true);
    pbEditPause->setEnabled(false);
    rbEditPolygon->setEnabled(true);
    rbEditRectangle->setEnabled(true);
}

void ccPointCloudTilesFileDlg::loadSelectedTiles()
{
    QString functionName="ccPointCloudTilesFileDlg::loadSelectedTiles";
    QString strAuxError;
    QString project=projectsComboBox->currentText();
    if(project.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        return;
    }

    //    QMap<QString, QString> tilesWkt;
    //    if(!mPtrPointCloudFileManager->getTilesWktGeometry(project,
    //                                                       tilesWkt,
    //                                                       strAuxError))
    //    {
    //        QString title=functionName;
    //        QString msg=QObject::tr("Getting Tiles for project:\n%1\nError:\n%2")
    //                .arg(project).arg(strAuxError);
    //        QMessageBox::information(this,title,msg);
    //        return;
    //    }

    ccGLCameraParameters camera;
    m_associatedWin->getGLCameraParameters(camera);
    ccPointCloud* cloud = nullptr;
    cloud = static_cast<ccPointCloud*>(mPtrCcTilesCenterPointsContainer->getChild(0));
    if(!cloud)
    {
        QString title=functionName;
        QString msg=QObject::tr("Invalid cloud for selectection");
        QMessageBox::information(this,title,msg);
        return;
    }
    unsigned cloudSize = cloud->size();
    const double half_w = camera.viewport[2] / 2.0;
    const double half_h = camera.viewport[3] / 2.0;
    CCVector3d Q2D;
//    std::vector<CCVector2> projectedPoints;
//    std::vector<bool> pointInFrustum;
    bool pointInFrustum = false;
    QVector<QVector<double> > pointsCoordinates;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        const CCVector3* P3D = cloud->getPoint(i);
        camera.project(*P3D, Q2D, &pointInFrustum);
        if(!pointInFrustum)
            continue;
        CCVector2 P2D(	static_cast<PointCoordinateType>(Q2D.x - half_w),
                        static_cast<PointCoordinateType>(Q2D.y - half_h));
        bool pointInside=false;
        pointInside = CCCoreLib::ManualSegmentationTools::isPointInsidePoly(P2D, m_selectionTilesPoly);
        if(!pointInside)
            continue;
        double fc=P3D->x-mPtrPshift->x;
        double sc=P3D->y-mPtrPshift->y;
        QVector<double> pointCoordinates;
        pointCoordinates.push_back(fc);
        pointCoordinates.push_back(sc);
        pointsCoordinates.push_back(pointCoordinates);
    }
    editPauseClicked();
    if(pointsCoordinates.size()==0)
        return;
    if(pointsCoordinates.size()==1)
    {
        QVector<double> pointCoordinates1;
        pointCoordinates1.push_back(pointsCoordinates[0][0]+0.10);
        pointCoordinates1.push_back(pointsCoordinates[0][1]+0.10);
        pointsCoordinates.push_back(pointCoordinates1);
        QVector<double> pointCoordinates2;
        pointCoordinates2.push_back(pointsCoordinates[0][0]+0.10);
        pointCoordinates2.push_back(pointsCoordinates[0][1]-0.10);
        pointsCoordinates.push_back(pointCoordinates2);
    }
    if(pointsCoordinates.size()==2)
    {
        QVector<double> pointCoordinates1;
        pointCoordinates1.push_back(pointsCoordinates[0][0]+0.10);
        pointCoordinates1.push_back(pointsCoordinates[0][1]+0.10);
        pointsCoordinates.push_back(pointCoordinates1);
        QVector<double> pointCoordinates2;
        pointCoordinates2.push_back(pointsCoordinates[0][0]+0.10);
        pointCoordinates2.push_back(pointsCoordinates[0][1]-0.10);
        pointsCoordinates.push_back(pointCoordinates2);
        QVector<double> pointCoordinates3;
        pointCoordinates3.push_back(pointsCoordinates[1][0]+0.10);
        pointCoordinates3.push_back(pointsCoordinates[1][1]+0.10);
        pointsCoordinates.push_back(pointCoordinates3);
        QVector<double> pointCoordinates4;
        pointCoordinates4.push_back(pointsCoordinates[1][0]+0.10);
        pointCoordinates4.push_back(pointsCoordinates[1][1]-0.10);
        pointsCoordinates.push_back(pointCoordinates4);
    }
    QString wktMultiPoint="MULTIPOINT(";
    for(int i=0;i<pointsCoordinates.size();i++)
    {
        if(i>0) wktMultiPoint+=",";
        wktMultiPoint+="(";
        wktMultiPoint+=QString::number(pointsCoordinates[i][0],'f',3);
        wktMultiPoint+=" ";
        wktMultiPoint+=QString::number(pointsCoordinates[i][1],'f',3);
        wktMultiPoint+=")";
    }
    wktMultiPoint+=")";
    QByteArray byteWktMultiPointGeometry = wktMultiPoint.toUtf8();
    char *charsWktMultiPointGeometry = byteWktMultiPointGeometry.data();
    OGRGeometry* ptrMultiPointGeometry=OGRGeometryFactory::createGeometry(wkbMultiPoint);
    if(OGRERR_NONE!=ptrMultiPointGeometry->importFromWkt(&charsWktMultiPointGeometry))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error importing multipoint from wkt:\n%1").arg(wktMultiPoint);
        QMessageBox::information(this,title,msg);
        return;
    }
    OGRGeometry* ptrMultiPointConvexHullGeometry=NULL;
    ptrMultiPointConvexHullGeometry=ptrMultiPointGeometry->ConvexHull();
    char* ptrWKT;
    if(OGRERR_NONE!=ptrMultiPointConvexHullGeometry->exportToWkt(&ptrWKT))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error computing convexhull from wkt:\n%1").arg(wktMultiPoint);
        QMessageBox::information(this,title,msg);
        return;
    }
    OGRwkbGeometryType geomType=ptrMultiPointConvexHullGeometry->getGeometryType();
    if(geomType!=wkbPoint
        &&geomType!=wkbLineString
        &&geomType!=wkbPolygon
        &&geomType!=wkbMultiPoint
        &&geomType!=wkbMultiLineString
        &&geomType!=wkbMultiPolygon
        &&geomType!=wkbGeometryCollection
        &&geomType!=wkbLinearRing)
        //&&geomType!=wkbPoint25D
        //&&geomType!=wkbLineString25D
        //&&geomType!=wkbPolygon25D
        //&&geomType!=wkbMultiPoint25D
        //&&geomType!=wkbMultiLineString25D
        //&&geomType!=wkbMultiPolygon25D
        //&&geomType!=wkbGeometryCollection25D)
    {
        QString title=functionName;
        QString msg=QObject::tr("Invalid convexhull geometry from wkt:\n%1").arg(wktMultiPoint);
        QMessageBox::information(this,title,msg);
        return;
    }
    OGREnvelope* ptrEnvelope=new OGREnvelope();
    if(geomType==wkbPoint)
    {
        ((OGRPoint*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbLineString)
    {
        ((OGRLineString*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbPolygon)
    {
        ((OGRPolygon*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbMultiPoint)
    {
        ((OGRMultiPoint*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbMultiLineString)
    {
        ((OGRMultiLineString*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbMultiPolygon)
    {
        ((OGRMultiPolygon*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    OGRGeometryFactory::destroyGeometry(ptrMultiPointConvexHullGeometry);
    OGRGeometryFactory::destroyGeometry(ptrMultiPointGeometry);
    if(rbPointCloud->isChecked())
    {
        QString wktGeometry=QString::fromLatin1(ptrWKT);
        int geometryCrsEpsgCode=mHorizontalCrsEpsgCode;
        QString geometryCrsProj4String="";
        QMap<int,QMap<int,QString> > tilesTableName;
        QMap<int, QMap<int, QMap<int,QVector<PCFile::Point> > > > pointsByTileByFileId;
        QMap<int,QMap<QString,bool> > existsFieldsByFileId;
        QVector<QString> ignoreTilesTableName;
        bool tilesFullGeometry=true;
        if(!mPtrPointCloudFileManager->getPointsFromWktGeometry(project,wktGeometry,geometryCrsEpsgCode,
                                                                geometryCrsProj4String,tilesTableName,
                                                                pointsByTileByFileId,
                                                                existsFieldsByFileId,
                                                                ignoreTilesTableName,
                                                                tilesFullGeometry,
                                                                strAuxError))
        {
            QString title=functionName;
            QString msg=QObject::tr("In project:\n%1\ngetting points for wkt:\n%2\nerror:\n%3")
                    .arg(project).arg(wktGeometry).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
        //    existsFields[POINTCLOUDFILE_PARAMETER_COLOR]=false;
        //    existsFields[POINTCLOUDFILE_PARAMETER_GPS_TIME]=false;
        //    existsFields[POINTCLOUDFILE_PARAMETER_INTENSITY]=false;
        //    existsFields[POINTCLOUDFILE_PARAMETER_NIR]=false;
        //    existsFields[POINTCLOUDFILE_PARAMETER_RETURN]=false;
        //    existsFields[POINTCLOUDFILE_PARAMETER_RETURNS]=false;
        //    existsFields[POINTCLOUDFILE_PARAMETER_SOURCE_ID]=false;
        //    existsFields[POINTCLOUDFILE_PARAMETER_USER_DATA]=false;
        QMap<int, QMap<int, QMap<int,QVector<PCFile::Point> > > >::const_iterator iterFileId=pointsByTileByFileId.begin();
        while(iterFileId!=pointsByTileByFileId.end())
        {
            int fileId=iterFileId.key();
            bool existsRGB=false;
            float rgbColorToUnit=1.0;
            int colorNumberOfBytes=0;
            if(existsFieldsByFileId.contains(fileId))
            {
                existsRGB=existsFieldsByFileId[fileId][POINTCLOUDFILE_PARAMETER_COLOR];
            }
            if(existsRGB)
            {
                if(!mPtrPointCloudFileManager->getColorNumberOfBytes(project,
                                                             colorNumberOfBytes,
                                                             strAuxError))
                {
                    QString title=functionName;
                    QString msg=QObject::tr("In project:\n%1\ngetting color number of bytes\nerror:\n%2")
                            .arg(project).arg(strAuxError);
                    QMessageBox::information(this,title,msg);
                    delete(ptrEnvelope);
                    return;
                }
                if(colorNumberOfBytes==1) rgbColorToUnit=1.0/255.0;
                else if(colorNumberOfBytes==1) rgbColorToUnit=1.0/65535.0;
            }
            QMap<int, QMap<int,QVector<PCFile::Point> > >::const_iterator iterTileX=pointsByTileByFileId[fileId].begin();
            while(iterTileX!=pointsByTileByFileId[fileId].end())
            {
                int tileX=iterTileX.key();
                QMap<int,QVector<PCFile::Point> >::const_iterator iterTileY=pointsByTileByFileId[fileId][tileX].begin();
                while(iterTileY!=pointsByTileByFileId[fileId][tileX].end())
                {
                    int tileY=iterTileY.key();
                    bool existsContainer=false;
                    if(mPtrCcPointsContainerByTileByFileId.contains(fileId))
                    {
                        if(mPtrCcPointsContainerByTileByFileId.contains(tileX))
                        {
                            if(mPtrCcPointsContainerByTileByFileId.contains(tileY))
                            {
                                existsContainer=true;
                            }
                        }
                    }
                    if(existsContainer)
                    {
                        iterTileY++;
                        continue;
                    }
                    QVector<PCFile::Point> points=iterTileY.value();
                    int numberOfPoints=points.size();
                    ccHObject* tileContainer=new ccHObject();
                    ccPointCloud* cloudGrid = new ccPointCloud("vertices");
                    if (!cloudGrid->reserve(numberOfPoints))
                    {
                        delete cloudGrid;
                        delete tileContainer;
                        QString title=functionName;
                        QString msg=QObject::tr("Error creating vertices for tile: %1 - %2 \nfor project:\n%3")
                                .arg(QString::number(tileX)).arg(QString::number(tileY)).arg(project);
                        QMessageBox::information(this,title,msg);
                        delete(ptrEnvelope);
                        return;
                    }
                    if(existsRGB)
                    {
                        if(!cloudGrid->reserveTheRGBTable())
                        {
                            delete cloudGrid;
                            delete tileContainer;
                            QString title=functionName;
                            QString msg=QObject::tr("Error allocation for color vertices for tile: %1 - %2 \nfor project:\n%3")
                                    .arg(QString::number(tileX)).arg(QString::number(tileY)).arg(project);
                            QMessageBox::information(this,title,msg);
                            delete(ptrEnvelope);
                            return;
                        }
                        cloudGrid->showColors(true);
                    }
                    for(int np=0;np<numberOfPoints;np++) // -1 porque se repite al final el primer punto
                    {
                        PCFile::Point pto=points[np];
                        int ix=pto.getIx();
                        double fc=(double)tileX+ix/1000.;
                        int iy=pto.getIy();
                        double sc=(double)tileY+iy/1000.;
                        double tc=pto.getZ();
                        float rf=0.;
                        float gf=0.;
                        float bf=0.;
                        CCVector3 ccPto;
                        ccPto.x=static_cast<PointCoordinateType>(fc+mPtrPshift->x);
                        ccPto.y=static_cast<PointCoordinateType>(sc+mPtrPshift->y);
                        ccPto.z=static_cast<PointCoordinateType>(tc+mPtrPshift->z);
                        assert(cloudGrid->size() < cloudGrid->capacity());
                        cloudGrid->addPoint(ccPto);
                        if(existsRGB)
                        {
                            if(colorNumberOfBytes==1)
                            {
                                QMap<QString,quint8> values8bits;
                                pto.get8BitsValues(values8bits);
                                if(values8bits.contains(POINTCLOUDFILE_PARAMETER_COLOR_RED))
                                {
                                    quint8 uintRed=values8bits[POINTCLOUDFILE_PARAMETER_COLOR_RED];
                                    rf=rgbColorToUnit*((float)uintRed);
                                }
                                if(values8bits.contains(POINTCLOUDFILE_PARAMETER_COLOR_GREEN))
                                {
                                    quint8 uintGreen=values8bits[POINTCLOUDFILE_PARAMETER_COLOR_GREEN];
                                    gf=rgbColorToUnit*((float)uintGreen);
                                }
                                if(values8bits.contains(POINTCLOUDFILE_PARAMETER_COLOR_BLUE))
                                {
                                    quint8 uintBlue=values8bits[POINTCLOUDFILE_PARAMETER_COLOR_BLUE];
                                    bf=rgbColorToUnit*((float)uintBlue);
                                }
                            }
                            else if(colorNumberOfBytes==2)
                            {
                                QMap<QString,quint16> values16bits;
                                pto.get16BitsValues(values16bits);
                                if(values16bits.contains(POINTCLOUDFILE_PARAMETER_COLOR_RED))
                                {
                                    quint8 uintRed=values16bits[POINTCLOUDFILE_PARAMETER_COLOR_RED];
                                    rf=rgbColorToUnit*((float)uintRed);
                                }
                                if(values16bits.contains(POINTCLOUDFILE_PARAMETER_COLOR_GREEN))
                                {
                                    quint8 uintGreen=values16bits[POINTCLOUDFILE_PARAMETER_COLOR_GREEN];
                                    gf=rgbColorToUnit*((float)uintGreen);
                                }
                                if(values16bits.contains(POINTCLOUDFILE_PARAMETER_COLOR_BLUE))
                                {
                                    quint8 uintBlue=values16bits[POINTCLOUDFILE_PARAMETER_COLOR_BLUE];
                                    bf=rgbColorToUnit*((float)uintBlue);
                                }
                            }
                            int iR=floor(rf*255.0);
                            int iG=floor(gf*255.0);
                            int iB=floor(bf*255.0);
                            ccColor::Rgba col(iR, iG, iB, 255);
                            cloudGrid->addColor(col);
                        }
                    }
    //                vertices->setEnabled(false);
    //                if (preserveCoordinateShift)
    //                {
    //                    vertices->setGlobalShift(*mPtrPshift);
    //                }
                    cloudGrid->setGlobalShift(*mPtrPshift);
                    QString tileLayerName=QObject::tr("Tile (%1,%2)")
                            .arg(QString::number(tileX)).arg(QString::number(tileY));
                    tileContainer->setName(QString("%1").arg(tileLayerName));
                    tileContainer->addChild(cloudGrid);
                    mPtrCcPointsContainerByTileByFileId[fileId][tileX][tileY]=tileContainer;
                    mPtrApp->addToDB(tileContainer, true, true, false);
    //                mPtrApp->redrawAll();
                    iterTileY++;
                }
                iterTileX++;
            }
            iterFileId++;
        }
        if(mPtrCcPointsContainerByTileByFileId.size()>0)
        {
            unloadTilesAllPushButton->setEnabled(true);
        }
        else
        {
            unloadTilesAllPushButton->setEnabled(false);
        }
    }
    if(rbModelDb->isChecked())
    {
        QString modelDbFileName=modelDbsComboBox->currentText();
        if(modelDbFileName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        {
            QString title=functionName;
            QString msg=QObject::tr("Select model database file before");
            QMessageBox::information(this,title,msg);
            return;
        }
        if (!mPtrMsDb)
        {
            QString title=functionName;
            QString msg=QObject::tr("Open model database file before");
            QMessageBox::information(this,title,msg);
            return;
        }
        if(!mPtrMsDb->getModelDataFromEnvelope(ptrEnvelope,
                                               mPtrModelObjectsByDbId,
                                               strAuxError))
        {
            QString title=functionName;
            QString msg=QObject::tr("Recovering data from model database file:\n%1\nError:\n%2")
                    .arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            delete(ptrEnvelope);
            return;
        }
//        QMap<int,QString> mModelNameByModelObjectDbId;
//        QMap<QString,QVector<int> > mModelObjectsDbIdByModelName;
//        QMap<QString,int> mSelectedModelObjectDbIdByModelName;
        QMap<int,AicedroneModelDb::ModelObject*>::const_iterator iterModelObjects=mPtrModelObjectsByDbId.begin();
        while(iterModelObjects!=mPtrModelObjectsByDbId.end())
        {
            int modelObjectDbId=iterModelObjects.key();
            AicedroneModelDb::ModelObject* ptrModelObject=iterModelObjects.value();
            QString strModelObjectDbId=QString::number(modelObjectDbId);
            QString modelObjectName;
            if(!ptrModelObject->getName(mPtrMsDb->mPtrDb,modelObjectName,strAuxError))
            {
                QString title=functionName;
                QString msg=QObject::tr("Recovering name for object: %1 from model database file:\n%2\nError:\n%3")
                        .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
                QMessageBox::information(this,title,msg);
                delete(ptrEnvelope);
                return;
            }
            mModelNameByModelObjectDbId[modelObjectDbId]=modelObjectName;
            if(!mModelObjectsDbIdByModelName.contains(modelObjectName))
            {
                QVector<int> auxVInt;
                mModelObjectsDbIdByModelName[modelObjectName]=auxVInt;
            }
            if(mModelObjectsDbIdByModelName[modelObjectName].indexOf(modelObjectDbId)==-1)
            {
                mModelObjectsDbIdByModelName[modelObjectName].push_back(modelObjectDbId);
            }
            if(ptrModelObject->getSelected())
            {
                if(!ptrModelObject->setEnabledStateFromDb(mPtrMsDb->mPtrDb,strAuxError))
                {
                    QString title=functionName;
                    QString msg=QObject::tr("Recovering enabled state for object: %1 from model database file:\n%2\nError:\n%3")
                            .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
                    QMessageBox::information(this,title,msg);
                    delete(ptrEnvelope);
                    return;
                }
                mSelectedModelObjectDbIdByModelName[modelObjectName]=modelObjectDbId;
                mSelectedInComboModelObjectDbIdByModelName[modelObjectName]=modelObjectDbId;
                bool modelObjectEnabled=ptrModelObject->getEnabled();
                if(mVisibleModelObjectsComboCheckBox->findText(modelObjectName)==-1)
                {
//                    QMap<int,AicedroneModelDb::ParametricGeometry*> parametricGeometriesByDbId;
//                    ptrModelObject->getParametricGeometries(parametricGeometriesByDbId);
//                    QMap<int,AicedroneModelDb::ParametricGeometry*>::const_iterator iterPg=parametricGeometriesByDbId.begin();
//                    AicedroneModelDb::ParametricGeometry* ptrParametricGeometry=iterPg.value();
                    AicedroneModelDb::ParametricGeometry* ptrParametricGeometry=ptrModelObject->getParametricGeometry();
                    QString parametricGeometryType=ptrParametricGeometry->getType();
                    if(parametricGeometryType.compare(AICEDRONE_PARAMETRIC_GEOMETRY_TYPE_CUBE3D,Qt::CaseInsensitive)==0)
                    {
                        QVector<double> parameters;
                        ptrParametricGeometry->getParameters(parameters);
                        if(parameters.size()<24)// ahora son muchos mas
                        {
                            QString title=functionName;
                            QString msg=QObject::tr("Error recovering points for cube: %1 from model database file:\n%2\nError:\n%3")
                                    .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
                            QMessageBox::information(this,title,msg);
                            delete(ptrEnvelope);
                            return;
                        }
                        int np=0;
                        QVector<QVector<double> > cubePoints;
                        while(np<22)
                        {
                            double x=parameters[np];
                            double y=parameters[np+1];
                            double z=parameters[np+2];
                            QVector<double> point(3);
                            point[0]=x;
                            point[1]=y;
                            point[2]=z;
                            cubePoints.push_back(point);
                            np=np+3;
                        }
                        if(!addCube(modelObjectDbId,modelObjectName,cubePoints,strAuxError))
                        {
                            QString title=functionName;
                            QString msg=QObject::tr("Error adding cube: %1 from model database file:\n%2\nError:\n%3")
                                    .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
                            QMessageBox::information(this,title,msg);
                            delete(ptrEnvelope);
                            return;
                        }
                        AicedroneModelDb::ModelObjectClass* ptrModelObjectClass=ptrModelObject->getClass();
                        int modelObjectClassDbId=ptrModelObjectClass->getDbId();
                        QString modelObjectClassName=ptrModelObjectClass->getTableName();
                        if(mVisibleModelClassesComboCheckBox->findText(modelObjectClassName)==-1)
                        {
                            QString strModelObjectClassDbId=QString::number(modelObjectClassDbId);
                            mPtrModelObjectClassesByDbId[modelObjectClassDbId]=ptrModelObjectClass;
                            mVisibleModelClassesComboCheckBox->addCheckItem(modelObjectClassName,strModelObjectClassDbId,Qt::Checked);
                        }
                        mVisibleModelObjectsComboCheckBox->addCheckItem(modelObjectName,strModelObjectDbId,Qt::Checked);
                        modelDbObjectComboBox->addItem(modelObjectName);
                    }
                }
                int posvo=mVisibleModelObjectsComboCheckBox->findText(modelObjectName);
                if(!modelObjectEnabled)
                {
                    mVisibleModelObjectsComboCheckBox->uncheckItem(posvo);
                    mVisibleModelObjectsComboCheckBox->disableItem(posvo);
                }
                else
                {
                    mVisibleModelObjectsComboCheckBox->enableItem(posvo);
                }
            }
            iterModelObjects++;
        }
        if(mPtrModelObjectsByDbId.size()>0)
        {
//            mVisibleModelClassesComboCheckBox->setEnabled(true);
            mVisibleModelObjectsComboCheckBox->setEnabled(true);
            visibleModelObjectsNonePushButton->setEnabled(true);
            visibleModelObjectsAllPushButton->setEnabled(true);
            modelDbObjectComboBox->setEnabled(true);
            unloadTilesAllPushButton->setEnabled(true);
            showObjectNamesCheckBox->setEnabled(true);
            showObjectNamesCheckBox->setChecked(false);
        }
        else
        {
            unloadTilesAllPushButton->setEnabled(false);
            showObjectNamesCheckBox->setEnabled(false);
            showObjectNamesCheckBox->setChecked(false);
        }
    }
    delete(ptrEnvelope);
//    QString title=functionName;
//    QString msg=QObject::tr("Load selected tiles");
//    msg+=QObject::tr("\nContact the author:\n");
//    msg+=QTOOLS_AUTHOR_MAIL;
//    QMessageBox::information(this,title,msg);
//    QMap<int,ccHObject*>::iterator iterCubes=mPtrCcCubeContainerByCubeDbId.begin();
//    while(iterCubes!=mPtrCcCubeContainerByCubeDbId.end())
//    {
//        iterCubes.value()->toggleColors();
//        iterCubes++;
//    }
//    QApplication::processEvents();
    return;
}

void ccPointCloudTilesFileDlg::unloadTiles()
{
    QString functionName="ccPointCloudTilesFileDlg::unloadTiles";
    QString strAuxError;
    QString project=projectsComboBox->currentText();
    if(project.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        return;
    }

    //    QMap<QString, QString> tilesWkt;
    //    if(!mPtrPointCloudFileManager->getTilesWktGeometry(project,
    //                                                       tilesWkt,
    //                                                       strAuxError))
    //    {
    //        QString title=functionName;
    //        QString msg=QObject::tr("Getting Tiles for project:\n%1\nError:\n%2")
    //                .arg(project).arg(strAuxError);
    //        QMessageBox::information(this,title,msg);
    //        return;
    //    }

    ccGLCameraParameters camera;
    m_associatedWin->getGLCameraParameters(camera);
    ccPointCloud* cloud = nullptr;
    cloud = static_cast<ccPointCloud*>(mPtrCcTilesCenterPointsContainer->getChild(0));
    if(!cloud)
    {
        QString title=functionName;
        QString msg=QObject::tr("Invalid cloud for selectection");
        QMessageBox::information(this,title,msg);
        return;
    }
    unsigned cloudSize = cloud->size();
    const double half_w = camera.viewport[2] / 2.0;
    const double half_h = camera.viewport[3] / 2.0;
    CCVector3d Q2D;
//    std::vector<CCVector2> projectedPoints;
//    std::vector<bool> pointInFrustum;
    bool pointInFrustum = false;
    QVector<QVector<double> > pointsCoordinates;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        const CCVector3* P3D = cloud->getPoint(i);
        camera.project(*P3D, Q2D, &pointInFrustum);
        if(!pointInFrustum)
            continue;
        CCVector2 P2D(	static_cast<PointCoordinateType>(Q2D.x - half_w),
                        static_cast<PointCoordinateType>(Q2D.y - half_h));
        bool pointInside=false;
        pointInside = CCCoreLib::ManualSegmentationTools::isPointInsidePoly(P2D, m_selectionTilesPoly);
        if(!pointInside)
            continue;
        double fc=P3D->x-mPtrPshift->x;
        double sc=P3D->y-mPtrPshift->y;
        QVector<double> pointCoordinates;
        pointCoordinates.push_back(fc);
        pointCoordinates.push_back(sc);
        pointsCoordinates.push_back(pointCoordinates);
    }
    editPauseClicked();
    if(pointsCoordinates.size()==0)
        return;
    if(pointsCoordinates.size()==1)
    {
        QVector<double> pointCoordinates1;
        pointCoordinates1.push_back(pointsCoordinates[0][0]+0.10);
        pointCoordinates1.push_back(pointsCoordinates[0][1]+0.10);
        pointsCoordinates.push_back(pointCoordinates1);
        QVector<double> pointCoordinates2;
        pointCoordinates2.push_back(pointsCoordinates[0][0]+0.10);
        pointCoordinates2.push_back(pointsCoordinates[0][1]-0.10);
        pointsCoordinates.push_back(pointCoordinates2);
    }
    if(pointsCoordinates.size()==2)
    {
        double fc,sc;
        if(fabs(pointsCoordinates[0][0]-pointsCoordinates[1][0])>0.1)
        {
            fc=(pointsCoordinates[0][0]+pointsCoordinates[1][0])/2.;
            sc=(pointsCoordinates[0][1]+pointsCoordinates[1][1])/2.-0.1;
        }
        else if(fabs(pointsCoordinates[0][1]-pointsCoordinates[1][1])>0.1)
        {
            fc=(pointsCoordinates[0][0]+pointsCoordinates[1][0])/2.+0.1;
            sc=(pointsCoordinates[0][1]+pointsCoordinates[1][1])/2.;
        }
        else
        {
            QString title=functionName;
            QString msg=QObject::tr("Invalid two points selected case");
            QMessageBox::information(this,title,msg);
            return;
        }
        QVector<double> pointCoordinates;
        pointCoordinates.push_back(fc);
        pointCoordinates.push_back(sc);
        pointsCoordinates.push_back(pointCoordinates);
    }
    QString wktMultiPoint="MULTIPOINT(";
    for(int i=0;i<pointsCoordinates.size();i++)
    {
        if(i>0) wktMultiPoint+=",";
        wktMultiPoint+="(";
        wktMultiPoint+=QString::number(pointsCoordinates[i][0],'f',3);
        wktMultiPoint+=" ";
        wktMultiPoint+=QString::number(pointsCoordinates[i][1],'f',3);
        wktMultiPoint+=")";
    }
    wktMultiPoint+=")";
    QByteArray byteWktMultiPointGeometry = wktMultiPoint.toUtf8();
    char *charsWktMultiPointGeometry = byteWktMultiPointGeometry.data();
    OGRGeometry* ptrMultiPointGeometry=OGRGeometryFactory::createGeometry(wkbMultiPoint);
    if(OGRERR_NONE!=ptrMultiPointGeometry->importFromWkt(&charsWktMultiPointGeometry))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error importing multipoint from wkt:\n%1").arg(wktMultiPoint);
        QMessageBox::information(this,title,msg);
        return;
    }
    OGRGeometry* ptrMultiPointConvexHullGeometry=NULL;
    ptrMultiPointConvexHullGeometry=ptrMultiPointGeometry->ConvexHull();
    char* ptrWKT;
    if(OGRERR_NONE!=ptrMultiPointConvexHullGeometry->exportToWkt(&ptrWKT))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error computing convexhull from wkt:\n%1").arg(wktMultiPoint);
        QMessageBox::information(this,title,msg);
        return;
    }
    OGRwkbGeometryType geomType=ptrMultiPointConvexHullGeometry->getGeometryType();
    if(geomType!=wkbPoint
        &&geomType!=wkbLineString
        &&geomType!=wkbPolygon
        &&geomType!=wkbMultiPoint
        &&geomType!=wkbMultiLineString
        &&geomType!=wkbMultiPolygon
        &&geomType!=wkbGeometryCollection
        &&geomType!=wkbLinearRing)
        //&&geomType!=wkbPoint25D
        //&&geomType!=wkbLineString25D
        //&&geomType!=wkbPolygon25D
        //&&geomType!=wkbMultiPoint25D
        //&&geomType!=wkbMultiLineString25D
        //&&geomType!=wkbMultiPolygon25D
        //&&geomType!=wkbGeometryCollection25D)
    {
        QString title=functionName;
        QString msg=QObject::tr("Invalid convexhull geometry from wkt:\n%1").arg(wktMultiPoint);
        QMessageBox::information(this,title,msg);
        return;
    }
    OGREnvelope* ptrEnvelope=new OGREnvelope();
    if(geomType==wkbPoint)
    {
        ((OGRPoint*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbLineString)
    {
        ((OGRLineString*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbPolygon)
    {
        ((OGRPolygon*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbMultiPoint)
    {
        ((OGRMultiPoint*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbMultiLineString)
    {
        ((OGRMultiLineString*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    if(geomType==wkbMultiPolygon)
    {
        ((OGRMultiPolygon*)ptrMultiPointConvexHullGeometry)->getEnvelope(ptrEnvelope);
    }
    OGRGeometryFactory::destroyGeometry(ptrMultiPointConvexHullGeometry);
    OGRGeometryFactory::destroyGeometry(ptrMultiPointGeometry);
    bool neededRedraw=false;
    if(rbPointCloud->isChecked())
    {
        QString wktGeometry=QString::fromLatin1(ptrWKT);
        int geometryCrsEpsgCode=mHorizontalCrsEpsgCode;
        QString geometryCrsProj4String="";
        QMap<int,QMap<int,QString> > tilesTableName;
        if(!mPtrPointCloudFileManager->getTilesNamesFromWktGeometry(project,wktGeometry,geometryCrsEpsgCode,
                                                                    geometryCrsProj4String,tilesTableName,
                                                                    strAuxError))
        {
            QString title=functionName;
            QString msg=QObject::tr("In project:\n%1\ngetting tiles for wkt:\n%2\nerror:\n%3")
                    .arg(project).arg(wktGeometry).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
        QMap<int,QMap<int,QString> >::const_iterator iterTileX=tilesTableName.begin();
        QVector<int> filesIdToRemove;
        QVector<int> tilesXToRemove;
        QVector<int> tilesYToRemove;
        while(iterTileX!=tilesTableName.end())
        {
            int tileX=iterTileX.key();
            QMap<int,QString>::const_iterator iterTileY=tilesTableName[tileX].begin();
            while(iterTileY!=tilesTableName[tileX].end())
            {
                int tileY=iterTileY.key();
                QMap<int, QMap<int, QMap<int,ccHObject*> > >::iterator iterFileId=mPtrCcPointsContainerByTileByFileId.begin();
                while(iterFileId!=mPtrCcPointsContainerByTileByFileId.end())
                {
                    int fileId=iterFileId.key();
                    if(mPtrCcPointsContainerByTileByFileId[fileId].contains(tileX))
                    {
                        if(mPtrCcPointsContainerByTileByFileId[fileId][tileX].contains(tileY))
                        {
                            ccHObject* tileContainer=mPtrCcPointsContainerByTileByFileId[fileId][tileX][tileY];
                            bool autoRemove=true;
                            mPtrApp->removeFromDB(tileContainer, autoRemove);
                            mPtrCcPointsContainerByTileByFileId[fileId][tileX][tileY]=nullptr;
                            filesIdToRemove.push_back(fileId);
                            tilesXToRemove.push_back(tileX);
                            tilesYToRemove.push_back(tileY);
                            if(!neededRedraw)
                                neededRedraw=true;
                        }
                    }
                    iterFileId++;
                }
                iterTileY++;
            }
            iterTileX++;
        }
        for(int i=0;i<filesIdToRemove.size();i++)
        {
            int fileId=filesIdToRemove[i];
            int tileX=tilesXToRemove[i];
            int tileY=tilesYToRemove[i];
            mPtrCcPointsContainerByTileByFileId[fileId][tileX].remove(tileY);
            if(mPtrCcPointsContainerByTileByFileId[fileId][tileX].size()==0)
            {
                mPtrCcPointsContainerByTileByFileId[fileId].remove(tileX);
                if(mPtrCcPointsContainerByTileByFileId[fileId].size()==0)
                {
                    mPtrCcPointsContainerByTileByFileId.remove(fileId);
                    if(mPtrCcPointsContainerByTileByFileId.size()==0)
                    {
                        mPtrCcPointsContainerByTileByFileId.clear();
                    }
                }
            }
        }
    }
    if(rbModelDb->isChecked())
    {
        QString modelDbFileName=modelDbsComboBox->currentText();
        if(modelDbFileName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        {
            QString title=functionName;
            QString msg=QObject::tr("Select model database file before");
            QMessageBox::information(this,title,msg);
            delete(ptrEnvelope);
            return;
        }
        if (!mPtrMsDb)
        {
            QString title=functionName;
            QString msg=QObject::tr("Open model database file before");
            QMessageBox::information(this,title,msg);
            delete(ptrEnvelope);
            return;
        }
        QVector<int> modelObjectClassesDbIdsNotToRemove;
        QVector<int> modelObjectClassesDbIdsToRemove;
        QVector<int> modelObjectsDbIdsToRemove;
        QMap<int,AicedroneModelDb::ModelObject*>::const_iterator iterModelObjects=mPtrModelObjectsByDbId.begin();
        while(iterModelObjects!=mPtrModelObjectsByDbId.end())
        {
            int modelObjectDbId=iterModelObjects.key();
            AicedroneModelDb::ModelObject* ptrModelObject=iterModelObjects.value();
            AicedroneModelDb::ModelObjectClass* ptrModelObjectClass=ptrModelObject->getClass();
            int modelObjectClassDbId=ptrModelObjectClass->getDbId();
            OGREnvelope3D* ptrModelObjectEnvelope3D=ptrModelObject->getEnvelope3D();
            double minX=ptrModelObjectEnvelope3D->MinX;
            double minY=ptrModelObjectEnvelope3D->MinY;
            double maxX=ptrModelObjectEnvelope3D->MaxX;
            double maxY=ptrModelObjectEnvelope3D->MaxY;
            if(minX>=ptrEnvelope->MinX&&minX<=ptrEnvelope->MaxX
                    &&maxX>=ptrEnvelope->MinX&&maxX<=ptrEnvelope->MaxX
                    &&minY>=ptrEnvelope->MinY&&minY<=ptrEnvelope->MaxY
                    &&maxY>=ptrEnvelope->MinY&&maxY<=ptrEnvelope->MaxY)
            {
                modelObjectsDbIdsToRemove.push_back(modelObjectDbId);
                if(modelObjectClassesDbIdsToRemove.indexOf(modelObjectClassDbId)==-1)
                {
                    modelObjectClassesDbIdsToRemove.push_back(modelObjectClassDbId);
                }
            }
            else
            {
                if(modelObjectClassesDbIdsNotToRemove.indexOf(modelObjectClassDbId)==-1)
                {
                    modelObjectClassesDbIdsNotToRemove.push_back(modelObjectClassDbId);
                }
            }
            iterModelObjects++;
        }
        for(int nmo=0;nmo<modelObjectsDbIdsToRemove.size();nmo++)
        {
            int modelObjectDbId=modelObjectsDbIdsToRemove[nmo];
            AicedroneModelDb::ModelObject* ptrModelObject=mPtrModelObjectsByDbId[modelObjectDbId];
            QString strModelObjectDbId=QString::number(modelObjectDbId);
            QString modelObjectName;
            if(!ptrModelObject->getName(mPtrMsDb->mPtrDb,modelObjectName,strAuxError))
            {
                QString title=functionName;
                QString msg=QObject::tr("Recovering name for object: %1 from model database file:\n%2\nError:\n%3")
                        .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
                QMessageBox::information(this,title,msg);
                delete(ptrEnvelope);
                return;
            }
            mPtrModelObjectsByDbId.remove(modelObjectDbId);
            ccHObject* ptrCcCubeConteiner=mPtrCcCubeContainerByCubeDbId[modelObjectDbId];
            bool autoRemove=true;
            mPtrApp->removeFromDB(ptrCcCubeConteiner, autoRemove);
//            delete(ptrCcCubeConteiner);
            mPtrCcCubeContainerByCubeDbId[modelObjectDbId]=nullptr;
            mPtrCcCubeContainerByCubeDbId.remove(modelObjectDbId);
            if(!neededRedraw)
                neededRedraw=true;
            int pos=mVisibleModelObjectsComboCheckBox->findText(modelObjectName);
            if(pos>-1)
            {
                mVisibleModelObjectsComboCheckBox->removeItem(pos);
            }
            pos=modelDbObjectComboBox->findText(modelObjectName);
            if(pos>-1)
            {
                modelDbObjectComboBox->removeItem(pos);
            }
        }
        for(int nmoc=0;nmoc<modelObjectClassesDbIdsToRemove.size();nmoc++)
        {
            int modelObjectClassDbId=modelObjectClassesDbIdsToRemove[nmoc];
            if(modelObjectClassesDbIdsNotToRemove.indexOf(modelObjectClassDbId)!=-1)
                continue;
            AicedroneModelDb::ModelObjectClass* ptrModelObjectClass=mPtrModelObjectClassesByDbId[modelObjectClassDbId];
            QString modelObjectClassName=ptrModelObjectClass->getTableName();
            int pos=mVisibleModelClassesComboCheckBox->findText(modelObjectClassName);
            if(pos>-1)
            {
                mVisibleModelClassesComboCheckBox->removeItem(pos);
            }
            mPtrModelObjectClassesByDbId.remove(modelObjectClassDbId);
        }
    }
    if(neededRedraw)
    {
        mPtrApp->redrawAll();
    }
    delete(ptrEnvelope);
    if(mPtrModelObjectsByDbId.size()<1)
    {
        mVisibleModelClassesComboCheckBox->setEnabled(false);
        mVisibleModelObjectsComboCheckBox->setEnabled(false);
        visibleModelObjectsNonePushButton->setEnabled(false);
        visibleModelObjectsAllPushButton->setEnabled(false);
        modelDbObjectComboBox->setEnabled(false);
        if(mPtrCcPointsContainerByTileByFileId.size()==0)
        {
            unloadTilesAllPushButton->setEnabled(false);
        }
        showObjectNamesCheckBox->setEnabled(false);
    }
}

void ccPointCloudTilesFileDlg::unloadTilesAll()
{
    QString functionName="ccPointCloudTilesFileDlg::unloadTilesAll";
    bool neededRedraw=false;
    if(rbPointCloud->isChecked())
    {
        QMap<int, QMap<int, QMap<int,ccHObject*> > >::iterator iterFileId=mPtrCcPointsContainerByTileByFileId.begin();
        while(iterFileId!=mPtrCcPointsContainerByTileByFileId.end())
        {
            int fileId=iterFileId.key();
            QMap<int, QMap<int,ccHObject*> >::iterator iterTileX=mPtrCcPointsContainerByTileByFileId[fileId].begin();
            while(iterTileX!=mPtrCcPointsContainerByTileByFileId[fileId].end())
            {
                int tileX=iterTileX.key();
                QMap<int,ccHObject*>::iterator iterTileY=mPtrCcPointsContainerByTileByFileId[fileId][tileX].begin();
                while(iterTileY!=mPtrCcPointsContainerByTileByFileId[fileId][tileX].end())
                {
                    int tileY=iterTileY.key();
                    ccHObject* tileContainer=iterTileY.value();
                    bool autoRemove=true;
                    mPtrApp->removeFromDB(tileContainer, autoRemove);
                    mPtrCcPointsContainerByTileByFileId[fileId][tileX][tileY]=nullptr;
                    if(!neededRedraw)
                        neededRedraw=true;
                    iterTileY++;
                }
                iterTileX++;
            }
            iterFileId++;
        }
        mPtrCcPointsContainerByTileByFileId.clear();
    }
    if(rbModelDb->isChecked())
    {
        QString modelDbFileName=modelDbsComboBox->currentText();
        if(modelDbFileName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        {
            QString title=functionName;
            QString msg=QObject::tr("Select model database file before");
            QMessageBox::information(this,title,msg);
            return;
        }
        if (!mPtrMsDb)
        {
            QString title=functionName;
            QString msg=QObject::tr("Open model database file before");
            QMessageBox::information(this,title,msg);
            return;
        }
        QMap<int,ccHObject*>::iterator iterCubes=mPtrCcCubeContainerByCubeDbId.begin();
        while(iterCubes!=mPtrCcCubeContainerByCubeDbId.end())
        {
            ccHObject* ptrCcCubeConteiner=iterCubes.value();
            bool autoRemove=true;
            mPtrApp->removeFromDB(ptrCcCubeConteiner, autoRemove);
            if(!neededRedraw)
                neededRedraw=true;
            mPtrCcCubeContainerByCubeDbId[iterCubes.key()]=nullptr;
            iterCubes++;
        }
        mPtrCcCubeContainerByCubeDbId.clear();
        mPtrModelObjectClassesByDbId.clear();
        mPtrModelObjectsByDbId.clear();
        mModelNameByModelObjectDbId.clear();
        mModelObjectsDbIdByModelName.clear();
        mSelectedModelObjectDbIdByModelName.clear();
        mSelectedInComboModelObjectDbIdByModelName.clear();
        modelDbObjectComboBox->clear();
        mVisibleModelClassesComboCheckBox->clear();
        mVisibleModelObjectsComboCheckBox->clear();
        mVisibleModelClassesComboCheckBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_CLASSES_TITLE);
        mVisibleModelObjectsComboCheckBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_OBJECTS_TITLE);
        modelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
        showObjectNamesCheckBox->setChecked(false);
        showObjectNamesCheckBox->setEnabled(false);
    }
    if(neededRedraw)
    {
        mPtrApp->redrawAll();
    }
    if(mPtrModelObjectsByDbId.size()<1)
    {
        mVisibleModelClassesComboCheckBox->setEnabled(false);
        mVisibleModelObjectsComboCheckBox->setEnabled(false);
        visibleModelObjectsNonePushButton->setEnabled(false);
        visibleModelObjectsAllPushButton->setEnabled(false);
        modelDbObjectComboBox->setEnabled(false);
        if(mPtrCcPointsContainerByTileByFileId.size()==0)
        {
            unloadTilesAllPushButton->setEnabled(false);
        }
    }
}

void ccPointCloudTilesFileDlg::editPolygonClicked()
{

}

void ccPointCloudTilesFileDlg::editRectangleClicked()
{

}

void ccPointCloudTilesFileDlg::addPointToPolylineExt(int x, int y, bool allowClicksOutside)
{
    if ((m_pointsEditionState & STARTED) == 0)
    {
        return;
    }
    if (!m_associatedWin)
    {
        assert(false);
        return;
    }

    if (	!allowClicksOutside
            &&	(x < 0 || y < 0 || x >= m_associatedWin->qtWidth() || y >= m_associatedWin->qtHeight())
       )
    {
        //ignore clicks outside of the 3D view
        return;
    }

    assert(m_selectionTilesPolyVertices);
    assert(m_selectionTilesPoly);
    unsigned vertCount = m_selectionTilesPolyVertices->size();

    //particular case: we close the rectangular selection by a 2nd click
    if (rbEditRectangle->isChecked() && vertCount == 4 && (m_pointsEditionState & RUNNING))
        return;

    //new point
    QPointF pos2D = m_associatedWin->toCenteredGLCoordinates(x, y);
    CCVector3 P(static_cast<PointCoordinateType>(pos2D.x()),
                static_cast<PointCoordinateType>(pos2D.y()),
                0);

    //CTRL key pressed at the same time?
//    bool ctrlKeyPressed = m_rectangularSelection || ((QApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier);

    //start new polyline?
//    if (((m_state & RUNNING) == 0) || vertCount == 0 || ctrlKeyPressed)
    if (((m_pointsEditionState & RUNNING) == 0) || vertCount == 0 )
    {
        //reset state
//        m_state = (ctrlKeyPressed ? RECTANGLE : POLYLINE);
        if (rbEditRectangle->isChecked())
        {
            m_pointsEditionState=RECTANGLE;
        }
        if (rbEditPolygon->isChecked())
        {
            m_pointsEditionState=POLYLINE;
        }
        m_pointsEditionState |= STARTED;
        run();

        //reset polyline
        m_selectionTilesPolyVertices->clear();
        if (!m_selectionTilesPolyVertices->reserve(2))
        {
            ccLog::Error("Out of memory!");
//            allowPolylineExport(false);
            return;
        }
        //we add the same point twice (the last point will be used for display only)
        m_selectionTilesPolyVertices->addPoint(P);
        m_selectionTilesPolyVertices->addPoint(P);
        m_selectionTilesPoly->clear();
        if (!m_selectionTilesPoly->addPointIndex(0, 2))
        {
            ccLog::Error("Out of memory!");
//            allowPolylineExport(false);
            return;
        }
    }
    else //next points in "polyline mode" only
    {
        //we were already in 'polyline' mode?
        if (m_pointsEditionState & POLYLINE)
        {
            if (!m_selectionTilesPolyVertices->reserve(vertCount+1))
            {
                ccLog::Error("Out of memory!");
//                allowPolylineExport(false);
                return;
            }

            //we replace last point by the current one
            CCVector3 *lastP = const_cast<CCVector3 *>(m_selectionTilesPolyVertices->getPointPersistentPtr(vertCount-1));
            *lastP = P;
            //and add a new (equivalent) one
            m_selectionTilesPolyVertices->addPoint(P);
            if (!m_selectionTilesPoly->addPointIndex(vertCount))
            {
                ccLog::Error("Out of memory!");
                return;
            }
            m_selectionTilesPoly->setClosed(true);
        }
        else //we must change mode
        {
            assert(false); //we shouldn't fall here?!
            stopRunning();
            addPointToPolylineExt(x, y, allowClicksOutside);
            return;
        }
    }

    //DGM: to increase the poll rate of the mouse movements in ccGLWindow::mouseMoveEvent
    //we have to completely grab the mouse focus!
    //(the only way to take back the control is to right-click now...)
    m_associatedWin->grabMouse();
    m_associatedWin->redraw(true, false);
}

void ccPointCloudTilesFileDlg::closePolyLine(int x, int y)
{
    //only for polyline in RUNNING mode
    if ((m_pointsEditionState & POLYLINE) == 0
            || (m_pointsEditionState & RUNNING) == 0)
        return;

    if (m_associatedWin)
    {
        m_associatedWin->releaseMouse();
    }

    assert(m_selectionTilesPoly);
    unsigned vertCount = m_selectionTilesPoly->size();
    if (vertCount < 4)
    {
        m_selectionTilesPoly->clear();
        m_selectionTilesPolyVertices->clear();
    }
    else
    {
        //remove last point!
        m_selectionTilesPoly->resize(vertCount-1); //can't fail --> smaller
        m_selectionTilesPoly->setClosed(true);
    }

    //stop
    stopRunning();

    //set the default import/export icon to 'export' mode
//    loadSaveToolButton->setDefaultAction(actionExportSegmentationPolyline);
//    allowPolylineExport(m_segmentationPoly->size() > 1);

    if (m_associatedWin)
    {
        m_associatedWin->redraw(true, false);
    }
}

void ccPointCloudTilesFileDlg::closeRectangle()
{
    //only for rectangle selection in RUNNING mode
    if ((m_pointsEditionState & RECTANGLE) == 0
            || (m_pointsEditionState & RUNNING) == 0)
        return;

    assert(m_selectionTilesPoly);
    unsigned vertCount = m_selectionTilesPoly->size();
    if (vertCount < 4)
    {
        //first point only? we keep the real time update mechanism
        if (rbEditRectangle->isChecked())
            return;
        m_selectionTilesPoly->clear();
        m_selectionTilesPolyVertices->clear();
//        allowPolylineExport(false);
    }
//    else
//    {
//        allowPolylineExport(true);
//    }

    //stop
    stopRunning();

    if (m_associatedWin)
    {
        m_associatedWin->releaseMouse();
        m_associatedWin->redraw(true, false);
    }
}

void ccPointCloudTilesFileDlg::updatePolyLine(int x, int y, Qt::MouseButtons buttons)
{
    //process not started yet?
    if ((m_pointsEditionState & RUNNING) == 0)
    {
        return;
    }
    if (!m_associatedWin)
    {
        assert(false);
        return;
    }

    assert(m_selectionTilesPolyVertices);
    assert(m_selectionTilesPoly);

    unsigned vertCount = m_selectionTilesPolyVertices->size();

    //new point (expressed relatively to the screen center)
    QPointF pos2D = m_associatedWin->toCenteredGLCoordinates(x, y);
    CCVector3 P(static_cast<PointCoordinateType>(pos2D.x()),
                static_cast<PointCoordinateType>(pos2D.y()),
                0);

    if (m_pointsEditionState & RECTANGLE)
    {
        //we need 4 points for the rectangle!
        if (vertCount != 4)
            m_selectionTilesPolyVertices->resize(4);

        const CCVector3 *A = m_selectionTilesPolyVertices->getPointPersistentPtr(0);
        CCVector3 *B = const_cast<CCVector3 *>(m_selectionTilesPolyVertices->getPointPersistentPtr(1));
        CCVector3 *C = const_cast<CCVector3 *>(m_selectionTilesPolyVertices->getPointPersistentPtr(2));
        CCVector3 *D = const_cast<CCVector3 *>(m_selectionTilesPolyVertices->getPointPersistentPtr(3));
        *B = CCVector3(A->x, P.y, 0);
        *C = P;
        *D = CCVector3(P.x, A->y, 0);

        if (vertCount != 4)
        {
            m_selectionTilesPoly->clear();
            if (!m_selectionTilesPoly->addPointIndex(0, 4))
            {
                ccLog::Error("Out of memory!");
//                allowPolylineExport(false);
                return;
            }
            m_selectionTilesPoly->setClosed(true);
        }
    }
    else if (m_pointsEditionState & POLYLINE)
    {
        if (vertCount < 2)
            return;
        //we replace last point by the current one
        CCVector3 *lastP = const_cast<CCVector3 *>(m_selectionTilesPolyVertices->getPointPersistentPtr(vertCount - 1));
        *lastP = P;
    }

    m_associatedWin->redraw(true, false);
}

void ccPointCloudTilesFileDlg::run()
{
    m_pointsEditionState |= RUNNING;
    loadTilesPushButton->setEnabled(false);
    unloadTilesPushButton->setEnabled(false);
//    unloadTilesAllPushButton->setEnabled(false);
}

void ccPointCloudTilesFileDlg::stopRunning()
{
    m_pointsEditionState &= (~RUNNING);
    loadTilesPushButton->setEnabled(true); // we restore the buttons when running is stopped
    unloadTilesPushButton->setEnabled(true); // we restore the buttons when running is stopped
//    unloadTilesAllPushButton->setEnabled(true); // we restore the buttons when running is stopped
}

void ccPointCloudTilesFileDlg::onVisibleModelClassesComboCheckBox(int row, bool isChecked, QString text, QString data)
{
    int modelClassDbId=data.toInt();

}

void ccPointCloudTilesFileDlg::onVisibleModelObjectsComboCheckBox(int row, bool isChecked, QString text, QString data)
{
    if(text.compare(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_OBJECTS_TITLE,
                    Qt::CaseInsensitive)==0)
    {
        return;
    }
    QString modelObjectName=text;
    int modelObjectDbId=mSelectedInComboModelObjectDbIdByModelName[modelObjectName]; //before restore initial candidate, if different
    if((mPreviousModelName.isEmpty()
            ||mPreviousModelName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)!=0)
            &&mPreviousModelName.compare(modelObjectName,Qt::CaseInsensitive)==0)
    {
        if(mSelectedInComboModelObjectDbIdByModelName[mPreviousModelName]
                !=mSelectedModelObjectDbIdByModelName[mPreviousModelName])
        {
            int candidateModelObjectDbId=mSelectedModelObjectDbIdByModelName[modelObjectName];
            QString strCandidateModelObjectDbId=QString::number(candidateModelObjectDbId);
            int posCandidateInCombo=modelDbObjectCandidateComboBox->findText(strCandidateModelObjectDbId);
            if(posCandidateInCombo!=-1)
            {
                modelDbObjectCandidateComboBox->setCurrentIndex(posCandidateInCombo);
            }
//            mPreviousModelName=POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT;
        }
    }
    modelObjectDbId=mSelectedInComboModelObjectDbIdByModelName[modelObjectName];
//    int modelObjectDbId=data.toInt();
//    QString modelObjectName=mModelNameByModelObjectDbId[modelObjectDbId];
    ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbId];
    if(!isChecked)
    {
        bool autoRemove=false;
        mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
//        int pos=modelDbObjectComboBox->findText(modelObjectName);
//        if(pos>-1)
//        {
//            modelDbObjectComboBox->removeItem(pos);
//        }
    }
    else
    {
        mPtrApp->addToDB(ptrCcHObject, false, false, false);
        bool showObjectNames=showObjectNamesCheckBox->isChecked();
        ptrCcHObject->showNameIn3D(showObjectNames);
//        int pos=modelDbObjectComboBox->findText(modelObjectName);
//        if(pos==-1)
//        {
//            modelDbObjectComboBox->addItem(modelObjectName);
//        }
    }
    {
        modelDbObjectComboBox->clear();
        modelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
        QVector<int> visibleModelObjectsComboCheckBoxRows;
        QVector<bool> visibleModelObjectsComboCheckBoxCheckStates;
        QVector<QString> visibleModelObjectsComboCheckBoxTexts;
        QVector<QString> visibleModelObjectsComboCheckBoxDatas;
        mVisibleModelObjectsComboCheckBox->getItemsValues(visibleModelObjectsComboCheckBoxRows,
                                                          visibleModelObjectsComboCheckBoxCheckStates,
                                                          visibleModelObjectsComboCheckBoxTexts,
                                                          visibleModelObjectsComboCheckBoxDatas);
        for(int ncbc=0;ncbc<visibleModelObjectsComboCheckBoxTexts.size();ncbc++)
        {
            QString modelObjectName=visibleModelObjectsComboCheckBoxTexts[ncbc];
            if(mModelObjectsDbIdByModelName.contains(modelObjectName))
            {
                if(visibleModelObjectsComboCheckBoxCheckStates[ncbc])
                {
                    modelDbObjectComboBox->addItem(modelObjectName);
                }
            }
        }
    }
    if(modelDbObjectComboBox->count()==1)
    {
        modelDbObjectComboBox->setEnabled(false);
    }
    else
    {
        modelDbObjectComboBox->setEnabled(true);
    }
    mPtrApp->refreshAll(false);
//    ptrCcHObject->toggleVisibility();
//    ptrCcHObject->prepareDisplayForRefresh();
//    mVisibleModelObjectByDbId[modelObjectDbId]=isChecked;
////    updatePropertiesView();
}

void ccPointCloudTilesFileDlg::addModelDb()
{
    QString functionName="ccPointCloudTilesFileDlg::addModelDb";
    QString path=mLastPath;
    QString fileType="File";
    fileType+="(*.sqlite)";
    QString text=tr("Spatialite Database ");
    text+="(*.sqlite):";
    QString fileName = QFileDialog::getOpenFileName(this,
                                text,
                                path,
                                fileType);
    if (!fileName.isEmpty())
    {
        QString selectedPath=QFileInfo(fileName).absolutePath();
        if(selectedPath.compare(mLastPath,Qt::CaseInsensitive)!=0)
        {
            mLastPath=selectedPath;
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
            mPtrSettings->sync();
        }
        addModelDbFile(fileName);
    }
}

void ccPointCloudTilesFileDlg::openModelDb()
{
    QString functionName="ccPointCloudTilesFileDlg::openModelDb";
    QString strAuxError;
    QString modelDbFileName=modelDbsComboBox->currentText();
    if(modelDbFileName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    if (!QFile::exists(modelDbFileName))
    {
        QString title=functionName;
        QString msg=QObject::tr("Not exists selected model database file:\n%1").arg(modelDbFileName);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(mPtrMsDb)
    {
        QString currentModelDbFileName=mPtrMsDb->mPtrDb->getFileName();
        if(modelDbFileName.compare(currentModelDbFileName,Qt::CaseInsensitive)==0)
            return;
        else
        {
            delete(mPtrMsDb);
            mPtrMsDb=nullptr;
        }
    }
    mPtrMsDb=new AicedroneModelDb::ModelSpatialiteDb(mPtrCrsTools);
    if(!mPtrMsDb->openDatabase(modelDbFileName,
                               strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Opening model database file:\n%1\nError:\n%2")
                .arg(modelDbFileName).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
//    else
//    {
//        QString title=functionName;
//        QString msg=QObject::tr("Opening model database file:\n%1\nNumber of objects: %2")
//                .arg(modelDbFileName).arg(QString::number(mPtrMsDb->mPtrObjectByDbId.size()));
//        QMessageBox::information(this,title,msg);
//        return;
//    }
    addModelDbPushButton->setEnabled(false);
    openModelDbPushButton->setEnabled(false);
    closeModelDbPushButton->setEnabled(true);
    removeModelDbPushButton->setEnabled(false);
    mVisibleModelClassesComboCheckBox->clear();
    mVisibleModelClassesComboCheckBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_CLASSES_TITLE);
    mVisibleModelClassesComboCheckBox->setEnabled(false);
    mVisibleModelObjectsComboCheckBox->clear();
    visibleModelObjectsNonePushButton->setEnabled(false);
    visibleModelObjectsAllPushButton->setEnabled(false);
    mVisibleModelObjectsComboCheckBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_OBJECTS_TITLE);
    mVisibleModelObjectsComboCheckBox->setEnabled(false);
    modelDbObjectComboBox->clear();
    modelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    modelDbObjectComboBox->setEnabled(false);
    modelDbsComboBox->setEnabled(false);
}

void ccPointCloudTilesFileDlg::closeModelDb()
{
    bool pointCloudChecked=rbPointCloud->isChecked();
    if(rbPointCloud->isChecked())
    {
        rbModelDb->setChecked(true);
    }
    unloadTilesAll();
    if(pointCloudChecked)
    {
        rbPointCloud->setChecked(true);
    }
    if(!mPtrMsDb) return;
    delete(mPtrMsDb);
    mPtrMsDb=nullptr;
    modelDbsComboBox->setCurrentIndex(0);
    modelDbsComboBox->setEnabled(true);
}

void ccPointCloudTilesFileDlg::onModelDbObjectCandidateComboBoxCurrentIndexChanged(int index)
{
    if(!modelDbObjectCandidateComboBox->isEnabled())
        return;
    QString functionName="ccPointCloudTilesFileDlg::onModelDbObjectCandidateComboBoxCurrentIndexChanged";
    QString strAuxError;
    QString strCandidateModelObjectDbId=modelDbObjectCandidateComboBox->currentText();
    if(strCandidateModelObjectDbId.isEmpty())
        return;
    int candidateModelObjectDbId=strCandidateModelObjectDbId.toInt();
    if(mPtrCcCubeContainerByCubeDbId.contains(candidateModelObjectDbId))
        return;
    QString modelName=mModelNameByModelObjectDbId[candidateModelObjectDbId];
    for(int nc=0;nc<mModelObjectsDbIdByModelName[modelName].size();nc++)
    {
        int modelObjectDbId=mModelObjectsDbIdByModelName[modelName][nc];
        if(modelObjectDbId==candidateModelObjectDbId) continue;
        if(mPtrCcCubeContainerByCubeDbId.contains(modelObjectDbId))
        {
            ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbId];
            bool autoRemove=true;
            mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
            ptrCcHObject=nullptr;
            mPtrCcCubeContainerByCubeDbId[modelObjectDbId]=nullptr;
            mPtrCcCubeContainerByCubeDbId.remove(modelObjectDbId);
            break;
        }
    }
    AicedroneModelDb::ModelObject* ptrCandidateModelObject=mPtrModelObjectsByDbId[candidateModelObjectDbId];
    AicedroneModelDb::ParametricGeometry* ptrParametricGeometry=ptrCandidateModelObject->getParametricGeometry();
    QString parametricGeometryType=ptrParametricGeometry->getType();
    if(parametricGeometryType.compare(AICEDRONE_PARAMETRIC_GEOMETRY_TYPE_CUBE3D,Qt::CaseInsensitive)==0)
    {
        QVector<double> parameters;
        ptrParametricGeometry->getParameters(parameters);
        if(parameters.size()<24) // ahora son mas
        {
            QString title=functionName;
            QString msg=QObject::tr("Error recovering points for cube: %1 from model database file:\n%2\nError:\n%3")
                    .arg(strCandidateModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
        int np=0;
        QVector<QVector<double> > cubePoints;
        while(np<22)
        {
            double x=parameters[np];
            double y=parameters[np+1];
            double z=parameters[np+2];
            QVector<double> point(3);
            point[0]=x;
            point[1]=y;
            point[2]=z;
            cubePoints.push_back(point);
            np=np+3;
        }
        if(!addCube(candidateModelObjectDbId,modelName,cubePoints,strAuxError)) // never
        {
            QString title=functionName;
            QString msg=QObject::tr("Error adding cube: %1 from model database file:\n%2\nError:\n%3")
                    .arg(strCandidateModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
    mSelectedInComboModelObjectDbIdByModelName[modelName]=candidateModelObjectDbId;
}

void ccPointCloudTilesFileDlg::onModelDbObjectComboBoxCurrentIndexChanged(int index)
{
    if(!modelDbObjectComboBox->isEnabled())
        return;
    QString functionName="ccPointCloudTilesFileDlg::onModelDbObjectComboBoxCurrentIndexChanged";
    QString modelObjectName=modelDbObjectComboBox->currentText();
//    bool needRedraw=false;
    if((mPreviousModelName.isEmpty()
        ||mPreviousModelName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)!=0)
            &&mPreviousModelName.compare(modelObjectName,Qt::CaseInsensitive)!=0)
    {
        if(mSelectedInComboModelObjectDbIdByModelName[mPreviousModelName]
                !=mSelectedModelObjectDbIdByModelName[mPreviousModelName])
        {
            int modelObjectDbIdToAdd=mSelectedModelObjectDbIdByModelName[mPreviousModelName];
            QString strModelObjectDbIdSelected=QString::number(modelObjectDbIdToAdd);
            int posSelectedModelObjectDbId=modelDbObjectCandidateComboBox->findText(strModelObjectDbIdSelected);
            if(posSelectedModelObjectDbId!=-1)
            {
                modelDbObjectCandidateComboBox->setCurrentIndex(posSelectedModelObjectDbId);
            }
//            int modelObjectDbIdToRemove=mSelectedInComboModelObjectDbIdByModelName[mPreviousModelName];
//            ccHObject* ptrCcHObjectToRemove=mPtrCcCubeContainerByCubeDbId[modelObjectDbIdToRemove];
//            bool autoRemove=false;
//            mPtrApp->removeFromDB(ptrCcHObjectToRemove, autoRemove);

//            if(modelObjectName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
//            {
//                ccHObject* ptrCcHObjectToAdd=mPtrCcCubeContainerByCubeDbId[modelObjectDbIdToAdd];
//                mPtrApp->addToDB(ptrCcHObjectToAdd, false, false, false);
//            }
//            needRedraw=true;
        }
    }

    modelDbObjectCandidateComboBox->clear();
//    modelDbObjectCandidateComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
//    modelDbObjectCandidateComboBox->setCurrentIndex(0);
    modelDbObjectCandidateComboBox->setEnabled(false);
    processModelDbObjectComboBox->setCurrentIndex(0);
    processModelDbObjectComboBox->setEnabled(false);
    processModelDbObjectPushButton->setEnabled(false);
    if(modelObjectName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
//        setSelectedCandidateForPreviousModel();
        mPreviousModelName=modelObjectName;
//        if(needRedraw)
//        {
//            mPtrApp->refreshAll(false);
//        }
        return;
    }
    if(mModelObjectsDbIdByModelName[modelObjectName].size()==0) // called from clear
    {
//        if(needRedraw)
//        {
//            mPtrApp->refreshAll(false);
//        }
        return;
    }
    modelDbObjectCandidateComboBox->clear();
    for(int nc=0;nc<mModelObjectsDbIdByModelName[modelObjectName].size();nc++)
    {
        modelDbObjectCandidateComboBox->addItem(QString::number(mModelObjectsDbIdByModelName[modelObjectName][nc]));
    }
    int selectedModelObjectdDbId=mSelectedModelObjectDbIdByModelName[modelObjectName];
    int posSelectectModelObjectDbId=modelDbObjectCandidateComboBox->findText(QString::number(selectedModelObjectdDbId));
    if(posSelectectModelObjectDbId>-1)
    {
        modelDbObjectCandidateComboBox->setCurrentIndex(posSelectectModelObjectDbId);
    }
    if(mPtrModelObjectsByDbId[selectedModelObjectdDbId]->getEnabled()
            &&!mPtrModelObjectsByDbId[selectedModelObjectdDbId]->getChecked()) // si no esta chequeado se puede seleccionar otra geometria
    {
        modelDbObjectCandidateComboBox->setEnabled(true);
    }
//    setSelectedCandidateForPreviousModel();
    mPreviousModelName=modelObjectName;
    processModelDbObjectComboBox->setEnabled(true);
    return;
}

void ccPointCloudTilesFileDlg::onProcessModelDbObjectComboBoxCurrentIndexChanged(int index)
{
    if(!processModelDbObjectComboBox->isEnabled())
        return;
    QString functionName="ccPointCloudTilesFileDlg::onProcessModelDbObjectComboBoxCurrentIndexChanged";
    QString processModelObjectNameToRemove=processModelDbObjectComboBox->currentText();
    processModelDbObjectPushButton->setEnabled(false);
    if(processModelObjectNameToRemove.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        return;
    processModelDbObjectPushButton->setEnabled(true);
    return;
}

void ccPointCloudTilesFileDlg::onToolBoxCurrentIndexChanged(int index)
{
    QString functionName="ccPointCloudTilesFileDlg::onToolBoxCurrentIndexChanged";
    if(index==1&&mPtrPointCloudFileManager==nullptr)
    {
        QString title=functionName;
        QString msg=QObject::tr("Open project before");
        QMessageBox::information(this,title,msg);
        return;
    }
    return;
}

void ccPointCloudTilesFileDlg::removeModelDb()
{
    QString functionName="ccPointCloudTilesFileDlg::removeModelDb";
    QString strAuxError;
    QString modelDbFile=modelDbsComboBox->currentText();
    if(modelDbFile.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int pos=mModelDbs.indexOf(modelDbFile);
    mModelDbs.removeAt(pos);
    QString strModelDbs;
    modelDbsComboBox->clear();
    modelDbsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    for(int i=0;i<mModelDbs.size();i++)
    {
        if(!strModelDbs.isEmpty()) strModelDbs+=POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS;
        strModelDbs+=mModelDbs[i];
        modelDbsComboBox->addItem(mModelDbs[i]);
    }
    mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_MODEL_DATABASES,strModelDbs);
    mPtrSettings->sync();
    modelDbsComboBox->setEnabled(true);
    modelDbsComboBox->setCurrentIndex(0);
}

void ccPointCloudTilesFileDlg::removeModelDbObject()
{
    QString functionName="ccPointCloudTilesFileDlg::removeModelDbObject";
    QString strAuxError;
    QString modelObjectNameToRemove=modelDbObjectComboBox->currentText();
    int index=modelDbObjectComboBox->findText(modelObjectNameToRemove);
    if(index==0) return;
//    removeModelDbObjectPushButton->setEnabled(false);
    QString modelDbFile=modelDbsComboBox->currentText();
    if(modelDbFile.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int modelObjectDbIdToRemove=mSelectedModelObjectDbIdByModelName[modelObjectNameToRemove];
    if(modelObjectDbIdToRemove==-1)
        return;
//    AicedroneModelDb::ModelObject* ptrModelObject=mPtrModelObjectsByDbId[modelObjectDbIdToRemove];
    if(!mPtrMsDb->removeObject(modelObjectDbIdToRemove,strAuxError)) // se eliminan los AicedroneModelDb::ModelObject*
    {
        QString title=functionName;
        QString msg=QObject::tr("Removing object: %1 from model database file:\n%2\nError:\n%3")
                .arg(modelObjectNameToRemove).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    // ya se ha eliminado en la funcion remove
//    delete(ptrModelObject);
//    ptrModelObject=nullptr;
    bool needRefresh=false;
    for(int nc=0;nc<mModelObjectsDbIdByModelName[modelObjectNameToRemove].size();nc++)
    {
        int modelObjectDbId=mModelObjectsDbIdByModelName[modelObjectNameToRemove][nc];
        if(mPtrCcCubeContainerByCubeDbId.contains(modelObjectDbId))
        {
            ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbId];
            bool autoRemove=true;
            mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
            ptrCcHObject=nullptr;
            mPtrCcCubeContainerByCubeDbId[modelObjectDbId]=nullptr;
            mPtrCcCubeContainerByCubeDbId.remove(modelObjectDbId);
        }
        mPtrModelObjectsByDbId.remove(modelObjectDbId);
        mModelNameByModelObjectDbId.remove(modelObjectDbId);
    }
    mModelObjectsDbIdByModelName.remove(modelObjectNameToRemove);
    mSelectedModelObjectDbIdByModelName.remove(modelObjectNameToRemove);
    mSelectedInComboModelObjectDbIdByModelName.remove(modelObjectNameToRemove);
    int pos=mVisibleModelObjectsComboCheckBox->findText(modelObjectNameToRemove);
    mPreviousModelName.clear();
    if(pos>-1)
        mVisibleModelObjectsComboCheckBox->removeItem(pos);
    modelDbObjectComboBox->removeItem(index);
    modelDbObjectComboBox->setCurrentIndex(0);
//    if(needRefresh)
//    {
//        mPtrApp->refreshAll(false);
//    }
    bool existsCubes=false;
    QMap<int,ccHObject*>::const_iterator iterCubes=mPtrCcCubeContainerByCubeDbId.begin();
    while(iterCubes!=mPtrCcCubeContainerByCubeDbId.end())
    {
        if(iterCubes.value()!=nullptr)
        {
            existsCubes=true;
            break;
        }
        iterCubes++;
    }
    if(existsCubes)
    {
        showObjectNamesCheckBox->setEnabled(true);
    }
    else
    {
        showObjectNamesCheckBox->setChecked(false);
        showObjectNamesCheckBox->setEnabled(false);
    }
    mPtrApp->refreshAll(false);
    return;
}

void ccPointCloudTilesFileDlg::setCandidateModelDbObject()
{
    QString functionName="ccPointCloudTilesFileDlg::setCandidateModelDbObject";
    QString strAuxError;
    QString modelObjectNameToUpdate=modelDbObjectComboBox->currentText();
    int index=modelDbObjectComboBox->findText(modelObjectNameToUpdate);
    if(index==0) return;
//    removeModelDbObjectPushButton->setEnabled(false);
    QString modelDbFile=modelDbsComboBox->currentText();
    if(modelDbFile.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int modelObjectDbIdToChange=mSelectedModelObjectDbIdByModelName[modelObjectNameToUpdate];
    if(modelObjectDbIdToChange==-1)
        return;
    QString strCandidateModelObjectDbId=modelDbObjectCandidateComboBox->currentText();
    if(strCandidateModelObjectDbId.isEmpty())
        return;
    int candidateModelObjectDbId=strCandidateModelObjectDbId.toInt();
    if(modelObjectDbIdToChange==candidateModelObjectDbId)
        return;
//    AicedroneModelDb::ModelObject* ptrModelObject=mPtrModelObjectsByDbId[modelObjectDbIdToEnable];
    if(!mPtrMsDb->setCandidateObject(candidateModelObjectDbId,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Setting candidate object: %1 from model database file:\n%2\nError:\n%3")
                .arg(modelObjectNameToUpdate).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    int pos=mVisibleModelObjectsComboCheckBox->findText(modelObjectNameToUpdate);
    if(pos>-1)
        mVisibleModelObjectsComboCheckBox->changeItemData(pos,strCandidateModelObjectDbId);
    mSelectedModelObjectDbIdByModelName[modelObjectNameToUpdate]=candidateModelObjectDbId;
//    setSelectedCandidateForPreviousModel();
    if(mPtrCcCubeContainerByCubeDbId.contains(candidateModelObjectDbId))
    {
        bool showObjectNames=showObjectNamesCheckBox->isChecked();
        mPtrCcCubeContainerByCubeDbId[candidateModelObjectDbId]->showNameIn3D(showObjectNames);
    }
    mPtrApp->redrawAll();
    return;
}

void ccPointCloudTilesFileDlg::pointCloudClicked()
{
    modelDbGroupBox->setEnabled(false);
}

void ccPointCloudTilesFileDlg::processModelDbObject()
{
    QString functionName="ccPointCloudTilesFileDlg::processModelDbObject";
    QString strAuxError;
    QString modelObjectNameToRemove=modelDbObjectComboBox->currentText();
    int index=modelDbObjectComboBox->findText(modelObjectNameToRemove);
    QString modelObjectName=modelDbObjectComboBox->currentText();
    if(modelObjectName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select object before");
        QMessageBox::information(this,title,msg);
        return;
    }
////    if(index==0) return;
//    QString modelObjectNameToRemove=modelDbObjectComboBox->currentText();
//    {
//        QString title=functionName;
//        QString msg=QObject::tr("Select model database file before");
//        QMessageBox::information(this,title,msg);
//        return;
//    }
//    modelDbObjectComboBox->setCurrentIndex(0);
//    removeModelDbObjectPushButton->setEnabled(false);
    QString modelDbFile=modelDbsComboBox->currentText();
    if(modelDbFile.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    QString process=processModelDbObjectComboBox->currentText();
    if(process.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select process before");
        QMessageBox::information(this,title,msg);
        return;
    }
    disconnect(mVisibleModelObjectsComboCheckBox,SIGNAL(pressChecked(int,bool,QString,QString)),
            this, SLOT(onVisibleModelObjectsComboCheckBox(int,bool,QString,QString)));
    if(process.compare(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_ENABLE,
                            Qt::CaseInsensitive)==0)
    {
        enableModelDbObject();
    }
    if(process.compare(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_DISABLE,
                            Qt::CaseInsensitive)==0)
    {
        disableModelDbObject();
    }
    if(process.compare(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_REMOVE,
                            Qt::CaseInsensitive)==0)
    {
        removeModelDbObject();
    }
    if(process.compare(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_CHECK,
                            Qt::CaseInsensitive)==0)
    {
        checkModelDbObject();
    }
    if(process.compare(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_UNCHECK,
                            Qt::CaseInsensitive)==0)
    {
        uncheckModelDbObject();
    }
    if(process.compare(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_SET_CANDIDATE,
                            Qt::CaseInsensitive)==0)
    {
        setCandidateModelDbObject();
    }
    connect(mVisibleModelObjectsComboCheckBox,SIGNAL(pressChecked(int,bool,QString,QString)),
            this, SLOT(onVisibleModelObjectsComboCheckBox(int,bool,QString,QString)));
    return;
}

void ccPointCloudTilesFileDlg::modelDbClicked()
{
    modelDbGroupBox->setEnabled(true);
}

void ccPointCloudTilesFileDlg::visibleModelObjectsAll()
{
    //    mVisibleModelObjectsComboCheckBox->checkAll();
    QString functionName="ccPointCloudTilesFileDlg::visibleModelObjectsAll";
    QString strAuxError;
    if(mPreviousModelName.isEmpty()
        ||mPreviousModelName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)!=0)
    {
        if(mSelectedInComboModelObjectDbIdByModelName[mPreviousModelName]
                !=mSelectedModelObjectDbIdByModelName[mPreviousModelName])
        {
            int candidateModelObjectDbId=mSelectedModelObjectDbIdByModelName[mPreviousModelName];
            QString strCandidateModelObjectDbId=QString::number(candidateModelObjectDbId);
            int posCandidateInCombo=modelDbObjectCandidateComboBox->findText(strCandidateModelObjectDbId);
            if(posCandidateInCombo!=-1)
            {
                modelDbObjectCandidateComboBox->setCurrentIndex(posCandidateInCombo);
            }
//            mPreviousModelName=POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT;
        }
    }
    mVisibleModelObjectsComboCheckBox->checkAll();
    mPtrApp->refreshAll(false);
//    mPtrApp->redrawAll();
    /*
    modelDbObjectComboBox->clear();
    modelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    QVector<int> visibleModelObjectsComboCheckBoxRows;
    QVector<bool> visibleModelObjectsComboCheckBoxCheckStates;
    QVector<QString> visibleModelObjectsComboCheckBoxTexts;
    QVector<QString> visibleModelObjectsComboCheckBoxDatas;
    mVisibleModelObjectsComboCheckBox->getItemsValues(visibleModelObjectsComboCheckBoxRows,
                                                      visibleModelObjectsComboCheckBoxCheckStates,
                                                      visibleModelObjectsComboCheckBoxTexts,
                                                      visibleModelObjectsComboCheckBoxDatas);
    for(int ncbc=0;ncbc<visibleModelObjectsComboCheckBoxTexts.size();ncbc++)
    {
        QString modelObjectName=visibleModelObjectsComboCheckBoxTexts[ncbc];
        if(mModelObjectsDbIdByModelName.contains(modelObjectName)) // evita la cabecera
        {
            mVisibleModelObjectsComboCheckBox->checkItem(ncbc+1);
//            int modelObjectDbId=mModelObjectsDbIdByModelName[modelObjectName];
//            AicedroneModelDb::ModelObject* ptrModelObject=mPtrModelObjectsByDbId[modelObjectDbId];
//            if(!ptrModelObject->setEnabledStateFromDb(mPtrMsDb->mPtrDb,strAuxError))
//            {
//                QString title=functionName;
//                QString msg=QObject::tr("Recovering enabled state for object: %1 from model database file:\n%2\nError:\n%3")
//                        .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
//                QMessageBox::information(this,title,msg);
//                return;
//            }
//            bool modelObjectEnabled=ptrModelObject->getEnabled();
//            if(modelObjectEnabled)
//            {
//                mVisibleModelObjectsComboCheckBox->checkItem(ncbc);
//                modelDbObjectComboBox->addItem(modelObjectName);
//            }
        }
    }
    if(modelDbObjectComboBox->count()==1)// nunca
    {
        modelDbObjectComboBox->setEnabled(false);
    }
    else
    {
        modelDbObjectComboBox->setEnabled(true);
    }

//    QMap<int,AicedroneModelDb::ModelObject*>::const_iterator iterModelObjects=mPtrModelObjectsByDbId.begin();
//    while(iterModelObjects!=mPtrModelObjectsByDbId.end())
//    {
//        int modelObjectDbId=iterModelObjects.key();
//        AicedroneModelDb::ModelObject* ptrModelObject=iterModelObjects.value();
//        QString strModelObjectDbId=QString::number(modelObjectDbId);
//        QString modelObjectName;
//        if(!ptrModelObject->getName(mPtrMsDb->mPtrDb,modelObjectName,strAuxError))
//        {
//            QString title=functionName;
//            QString msg=QObject::tr("Recovering name for object: %1 from model database file:\n%2\nError:\n%3")
//                    .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
//            QMessageBox::information(this,title,msg);
//            return;
//        }
//        if(!ptrModelObject->setEnabledStateFromDb(mPtrMsDb->mPtrDb,strAuxError))
//        {
//            QString title=functionName;
//            QString msg=QObject::tr("Recovering enabled state for object: %1 from model database file:\n%2\nError:\n%3")
//                    .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
//            QMessageBox::information(this,title,msg);
//            return;
//        }
//        bool modelObjectEnabled=ptrModelObject->getEnabled();
//        if(modelObjectEnabled)
//        {
//            mVisibleModelObjectsComboCheckBox->checkItem(pos);
//            mVisibleModelObjectsComboCheckBox->enableItem(pos);
////            int pos=modelDbObjectComboBox->findText(modelObjectName);
////            if(pos>-1)
////            {
////                mVisibleModelObjectsComboCheckBox->checkItem(pos);
////                mVisibleModelObjectsComboCheckBox->enableItem(pos);
////            }
//        }
//        iterModelObjects++;
//    }
//    mPtrApp->refreshAll(false);
    */
}

void ccPointCloudTilesFileDlg::visibleModelObjectsNone()
{
    mVisibleModelObjectsComboCheckBox->uncheckAll();
    /*
    QString functionName="ccPointCloudTilesFileDlg::visibleModelObjectsNone";
    QString strAuxError;
    QMap<int,AicedroneModelDb::ModelObject*>::const_iterator iterModelObjects=mPtrModelObjectsByDbId.begin();
    while(iterModelObjects!=mPtrModelObjectsByDbId.end())
    {
        int modelObjectDbId=iterModelObjects.key();
        AicedroneModelDb::ModelObject* ptrModelObject=iterModelObjects.value();
        QString strModelObjectDbId=QString::number(modelObjectDbId);
        QString modelObjectName;
        if(!ptrModelObject->getName(mPtrMsDb->mPtrDb,modelObjectName,strAuxError))
        {
            QString title=functionName;
            QString msg=QObject::tr("Recovering name for object: %1 from model database file:\n%2\nError:\n%3")
                    .arg(strModelObjectDbId).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
        int pos=mVisibleModelObjectsComboCheckBox->findText(modelObjectName);
        if(pos>-1)
        {
            ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbId];
            bool autoRemove=false;
            mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
            mVisibleModelObjectsComboCheckBox->removeItem(pos);
        }
    }
    mPtrApp->refreshAll(false);
    */
}

bool ccPointCloudTilesFileDlg::addCube(int dbId, QString& name,
                                       QVector<QVector<double> >& vertices,
                                       QString &strError)
{
    QString functionName="ccPointCloudTilesFileDlg::addCube";
    QString strAuxError;
    strError.clear();
    QString project=projectsComboBox->currentText();
    if(project.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        return(false);
    }
    int numberOfPoints=vertices.size();// 8 -> 24
    ccHObject* cubeContainer=new ccHObject();
    ccPointCloud* cubeCloud = new ccPointCloud("vertices");
//    if (!cubeCloud->reserve(numberOfPoints))
    if (!cubeCloud->reserve(24))
    {
        delete cubeCloud;
        delete cubeContainer;
        strError=functionName;
        strError+=QObject::tr("\nError creating vertices for cube: %1 \nfor project:\n%3")
                .arg(name).arg(project);
        return(false);
    }
    for(int np=0;np<numberOfPoints;np++) // -1 porque se repite al final el primer punto
    {
        double fc=vertices[np][0];
        double sc=vertices[np][1];
        double tc=vertices[np][2];
        CCVector3 ccPto;
        ccPto.x=static_cast<PointCoordinateType>(fc+mPtrPshift->x);
        ccPto.y=static_cast<PointCoordinateType>(sc+mPtrPshift->y);
        ccPto.z=static_cast<PointCoordinateType>(tc+mPtrPshift->z);
        assert(cubeCloud->size() < cubeCloud->capacity());
        cubeCloud->addPoint(ccPto);
    }
    for(int np=numberOfPoints;np<24;np++) // -1 porque se repite al final el primer punto
    {
        CCVector3 ccPto;
        ccPto.x=static_cast<PointCoordinateType>(0);
        ccPto.y=static_cast<PointCoordinateType>(0);
        ccPto.z=static_cast<PointCoordinateType>(0);
        assert(cubeCloud->size() < cubeCloud->capacity());
        cubeCloud->addPoint(ccPto);
    }
    cubeCloud->setGlobalShift(*mPtrPshift);
    ccMesh* cubeMesh = nullptr;
    unsigned numberOfFacets = 12;
    cubeMesh = new ccMesh(cubeCloud);
    if (!cubeMesh->reserve(numberOfFacets))
    {
        delete cubeCloud;
        delete cubeContainer;
        delete cubeMesh;
        strError=functionName;
        strError+=QObject::tr("\nError creating meshes for cube: %1 \nfor project:\n%3")
                .arg(name).arg(project);
        return(false);
    }
//        3 0 1 2
//        3 0 3 2
//        3 3 4 5
//        3 3 2 5
//        3 4 7 6
//        3 4 5 6
//        3 7 6 1
//        3 7 0 1
//        3 1 2 5
//        3 1 6 5
//        3 0 3 4
//        3 0 7 4
    cubeMesh->addTriangle(0, 1, 2);
    cubeMesh->addTriangle(0, 3, 2);
    cubeMesh->addTriangle(3, 4, 5);
    cubeMesh->addTriangle(3, 2, 5);
    cubeMesh->addTriangle(4, 7, 6);
    cubeMesh->addTriangle(4, 5, 6);
    cubeMesh->addTriangle(7, 6, 1);
    cubeMesh->addTriangle(7, 0, 1);
    cubeMesh->addTriangle(1, 2, 5);
    cubeMesh->addTriangle(1, 6, 5);
    cubeMesh->addTriangle(0, 3, 4);
    cubeMesh->addTriangle(0, 7, 4);
    cubeMesh->addChild(cubeCloud);
    cubeCloud->setEnabled(false);
    cubeCloud->setName("Vertices");
    QString cubeLayerName=QObject::tr("%1").arg(name);
    cubeContainer->setName(QString("%1").arg(cubeLayerName));
//    cubeContainer->addChild(cubeCloud);
    cubeContainer->addChild(cubeMesh);
//    cubeMesh->showSF(false);
    mPtrCcCubeContainerByCubeDbId[dbId]=cubeContainer;
    bool updateZoom=false;/*=true*/
    bool autoExpandDBTree=true;/*=true*/
    bool checkDimensions=true;/*=true*/
    bool autoRedraw=true;/*=true*/
    mPtrApp->addToDB(cubeContainer,updateZoom,autoExpandDBTree,checkDimensions,autoRedraw);
    ccColor::Rgb col = ccColor::Generator::Random();
    cubeCloud->setColor(col);
//    cubeContainer->setColor(col);
    cubeMesh->showSF(false);
    cubeMesh->showColors(true);
    cubeCloud->showSF(false);
    cubeCloud->showColors(true);
    cubeContainer->showSF(false);
    cubeContainer->showColors(true);
//    cubeContainer->showNameIn3D(true);
//    cubeContainer->toggleColors();
    bool showObjectNames=showObjectNamesCheckBox->isChecked();
    cubeContainer->showNameIn3D(showObjectNames);
    return(true);
}

void ccPointCloudTilesFileDlg::addPointCloudFiles()
{
    QString functionName="ccPointCloudTilesFileDlg::addPointCloudFiles";
    QString strAuxError;
    QString horizontalCrsId=addPCFsHorizontalCRSsComboBox->currentText();
    if(horizontalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select horizonta CRS before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int horizontalCrsEpsgCode;
    if(!mPtrCrsTools->getCrsEpsgCode(horizontalCrsId,horizontalCrsEpsgCode,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting EPSG code for horizontal CRS: %1\nError:\n%2")
                .arg(horizontalCrsId).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    QString verticalCrsId=addPCFsVerticalCRSsComboBox->currentText();
    if(verticalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select vertical CRS before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int verticalCrsEpsgCode=-1;
    if(verticalCrsId.compare(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT,Qt::CaseInsensitive)!=0)
    {
        QString strVerticalCrsEpsgCode=verticalCrsId.remove(POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX);
        verticalCrsEpsgCode=strVerticalCrsEpsgCode.toInt();
    }
    QString projectPath=projectPathLineEdit->text();
    if(projectPath.isEmpty())
    {
        QString title=functionName;
        QString msg=QObject::tr("Select project path before");
        QMessageBox::information(this,title,msg);
        return;
    }
    QDateTime initialDateTime=QDateTime::currentDateTime();
    QVector<QString> pcfiles=mPCFiles.toVector();
    if(!mPtrPointCloudFileManager->addPointCloudFilesToPointCloudFile(projectPath,
                                                                      horizontalCrsEpsgCode,
                                                                      verticalCrsEpsgCode,
                                                                      pcfiles,
                                                                      strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Adding % point cloud files to project:\n%1\nError:\n%2")
                .arg(QString::number(pcfiles.size())).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    QDateTime finalDateTime=QDateTime::currentDateTime();
    int initialSeconds=(int)initialDateTime.toTime_t();
    int finalSeconds=(int)finalDateTime.toTime_t();
    int totalDurationSeconds=finalSeconds-initialSeconds;
    double dblTotalDurationSeconds=(double)totalDurationSeconds;
    int durationDays=(int)floor(dblTotalDurationSeconds/60.0/60.0/24.0);
    int durationHours=(int)floor((dblTotalDurationSeconds-durationDays*60.0*60.0*24.0)/60.0/60.0);
    int durationMinutes=(int)floor((dblTotalDurationSeconds-durationDays*60.0*60.0*24.0-durationHours*60.0*60.0)/60.0);
    int durationSeconds=dblTotalDurationSeconds-durationDays*60.0*60.0*24.0-durationHours*60.0*60.0-durationMinutes*60.0;
    QString msg=QObject::tr("Added %1 point cloud files").arg(QString::number(mPCFiles.size()));
    {
        QString msgTtime="\n- Process time:\n";
        msgTtime+="  - Start time of the process ......................: ";
        msgTtime+=initialDateTime.toString("yyyy/MM/dd - hh/mm/ss.zzz");
        msgTtime+="\n";
        msgTtime+="  - End time of the process ........................: ";
        msgTtime+=finalDateTime.toString("yyyy/MM/dd - hh/mm/ss.zzz");
        msgTtime+="\n";
        msgTtime+="  - Number of total seconds ........................: ";
        msgTtime+=QString::number(dblTotalDurationSeconds,'f',3);
        msgTtime+="\n";
        msgTtime+="    - Number of days ...............................: ";
        msgTtime+=QString::number(durationDays);
        msgTtime+="\n";
        msgTtime+="    - Number of hours ..............................: ";
        msgTtime+=QString::number(durationHours);
        msgTtime+="\n";
        msgTtime+="    - Number of minutes ............................: ";
        msgTtime+=QString::number(durationMinutes);
        msgTtime+="\n";
        msgTtime+="    - Number of seconds ............................: ";
        msgTtime+=QString::number(durationSeconds,'f',3);
        msgTtime+="\n";
        msg+=msgTtime;
    }
    QString title=functionName;
    QMessageBox::information(new QWidget(),title,msg);
}

void ccPointCloudTilesFileDlg::addModelDbFile(QString fileName)
{
    QString functionName="ccPointCloudTilesFileDlg::addModelDbFile";
    if (QFile::exists(fileName))
    {
        if(mModelDbs.contains(fileName))
            return;
        QString strModelDbs;
        mModelDbs.append(fileName);
        for(int i=0;i<mModelDbs.size();i++)
        {
            if(!strModelDbs.isEmpty()) strModelDbs+=POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS;
            strModelDbs+=mModelDbs[i];
            modelDbsComboBox->addItem(mModelDbs[i]);
        }
        mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_MODEL_DATABASES,strModelDbs);
        mPtrSettings->sync();
        int pos=modelDbsComboBox->findText(fileName);
        modelDbsComboBox->setCurrentIndex(pos);
    }
}

void ccPointCloudTilesFileDlg::addProjectPath(QString projectPath)
{
    QString functionName="ccPointCloudTilesFileDlg::addProject";
    if (!projectPath.isEmpty())
    {
        if(mProjects.contains(projectPath))
            return;
        QString strProjects;
        mProjects.append(projectPath);
        for(int i=0;i<mProjects.size();i++)
        {
            if(!strProjects.isEmpty()) strProjects+=POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS;
            strProjects+=mProjects[i];
            projectsComboBox->addItem(mProjects[i]);
        }
        mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_PROJECTS,strProjects);
        mPtrSettings->sync();
        int pos=projectsComboBox->findText(projectPath);
        projectsComboBox->setCurrentIndex(pos);
    }
}

void ccPointCloudTilesFileDlg::checkModelDbObject()
{
    QString functionName="ccPointCloudTilesFileDlg::checkModelDbObject";
    QString strAuxError;
    QString modelObjectNameToEnable=modelDbObjectComboBox->currentText();
    int index=modelDbObjectComboBox->findText(modelObjectNameToEnable);
    if(index==0) return;
//    removeModelDbObjectPushButton->setEnabled(false);
    QString modelDbFile=modelDbsComboBox->currentText();
    if(modelDbFile.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int modelObjectDbIdToCheck=mSelectedModelObjectDbIdByModelName[modelObjectNameToEnable];
    if(modelObjectDbIdToCheck==-1)
        return;
    QString strCandidateModelObjectDbId=modelDbObjectCandidateComboBox->currentText();
    if(strCandidateModelObjectDbId.isEmpty())
        return;
    int candidateModelObjectDbId=strCandidateModelObjectDbId.toInt();
    if(modelObjectDbIdToCheck!=candidateModelObjectDbId)
    {
        QString title=functionName;
        QString msg=QObject::tr("Only setted candidate: %1 can be checked").arg(QString::number(modelObjectDbIdToCheck));
        msg+=QObject::tr("\nSelect setted candidate or set this candidate before checked");
        QMessageBox::information(this,title,msg);
        return;
    }
//    AicedroneModelDb::ModelObject* ptrModelObject=mPtrModelObjectsByDbId[modelObjectDbIdToEnable];
    if(!mPtrMsDb->checkObject(modelObjectDbIdToCheck,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Checking object: %1 from model database file:\n%2\nError:\n%3")
                .arg(modelObjectNameToEnable).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    modelDbObjectComboBox->setCurrentIndex(0);
    // ya se ha eliminado en la funcion remove
//    delete(ptrModelObject);
//    ptrModelObject=nullptr;

//    mPtrModelObjectsByDbId.remove(modelObjectDbIdToRemove);
//    if(mPtrCcCubeContainerByCubeDbId.contains(modelObjectDbIdToRemove))
//    {
//        ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbIdToRemove];
//        bool autoRemove=true;
//        mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
//        ptrCcHObject=nullptr;
//        mPtrCcCubeContainerByCubeDbId.remove(modelObjectDbIdToRemove);
//    }
//    QMap<int,ccHObject*> mPtrCcCubeContainerByCubeDbId;
//    int pos=mVisibleModelObjectsComboCheckBox->findText(modelObjectNameToEnable);
//    if(pos>-1)
//    {
//        mVisibleModelObjectsComboCheckBox->checkItem(pos);
//        mVisibleModelObjectsComboCheckBox->enableItem(pos);
//    }
//    modelDbObjectComboBox->removeItem(index);
    return;
}

void ccPointCloudTilesFileDlg::disableModelDbObject()
{
    QString functionName="ccPointCloudTilesFileDlg::disableModelDbObject";//
    QString strAuxError;
    QString modelObjectNameToDisable=modelDbObjectComboBox->currentText();
    int index=modelDbObjectComboBox->findText(modelObjectNameToDisable);
    if(index==0) return;
    modelDbObjectComboBox->setCurrentIndex(0);
//    removeModelDbObjectPushButton->setEnabled(false);
    QString modelDbFile=modelDbsComboBox->currentText();
    if(modelDbFile.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int modelObjectDbIdToDisable=mSelectedModelObjectDbIdByModelName[modelObjectNameToDisable];
    if(modelObjectDbIdToDisable==-1)
        return;
//    AicedroneModelDb::ModelObject* ptrModelObject=mPtrModelObjectsByDbId[modelObjectDbIdToDisable];
    if(!mPtrMsDb->disableObject(modelObjectDbIdToDisable,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Disabling object: %1 from model database file:\n%2\nError:\n%3")
                .arg(modelObjectNameToDisable).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    // ya se ha eliminado en la funcion remove
//    delete(ptrModelObject);
//    ptrModelObject=nullptr;

//    mPtrModelObjectsByDbId.remove(modelObjectDbIdToRemove);
//    if(mPtrCcCubeContainerByCubeDbId.contains(modelObjectDbIdToRemove))
//    {
//        ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbIdToRemove];
//        bool autoRemove=true;
//        mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
//        ptrCcHObject=nullptr;
//        mPtrCcCubeContainerByCubeDbId.remove(modelObjectDbIdToRemove);
//    }
//    QMap<int,ccHObject*> mPtrCcCubeContainerByCubeDbId;
    int pos=mVisibleModelObjectsComboCheckBox->findText(modelObjectNameToDisable);
    if(pos>-1)
    {
        mVisibleModelObjectsComboCheckBox->uncheckItem(pos);
        mVisibleModelObjectsComboCheckBox->disableItem(pos);
    }
//    modelDbObjectComboBox->removeItem(index);
    return;
}

void ccPointCloudTilesFileDlg::enableModelDbObject()
{
    QString functionName="ccPointCloudTilesFileDlg::enableModelDbObject";
    QString strAuxError;
    QString modelObjectNameToEnable=modelDbObjectComboBox->currentText();
    int index=modelDbObjectComboBox->findText(modelObjectNameToEnable);
    if(index==0) return;
    modelDbObjectComboBox->setCurrentIndex(0);
//    removeModelDbObjectPushButton->setEnabled(false);
    QString modelDbFile=modelDbsComboBox->currentText();
    if(modelDbFile.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int modelObjectDbIdToEnable=mSelectedModelObjectDbIdByModelName[modelObjectNameToEnable];
    if(modelObjectDbIdToEnable==-1)
        return;
//    AicedroneModelDb::ModelObject* ptrModelObject=mPtrModelObjectsByDbId[modelObjectDbIdToEnable];
    if(!mPtrMsDb->enableObject(modelObjectDbIdToEnable,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Enabling object: %1 from model database file:\n%2\nError:\n%3")
                .arg(modelObjectNameToEnable).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    // ya se ha eliminado en la funcion remove
//    delete(ptrModelObject);
//    ptrModelObject=nullptr;

//    mPtrModelObjectsByDbId.remove(modelObjectDbIdToRemove);
//    if(mPtrCcCubeContainerByCubeDbId.contains(modelObjectDbIdToRemove))
//    {
//        ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbIdToRemove];
//        bool autoRemove=true;
//        mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
//        ptrCcHObject=nullptr;
//        mPtrCcCubeContainerByCubeDbId.remove(modelObjectDbIdToRemove);
//    }
//    QMap<int,ccHObject*> mPtrCcCubeContainerByCubeDbId;
    int pos=mVisibleModelObjectsComboCheckBox->findText(modelObjectNameToEnable);
    if(pos>-1)
    {
        mVisibleModelObjectsComboCheckBox->checkItem(pos);
        mVisibleModelObjectsComboCheckBox->enableItem(pos);
    }
//    modelDbObjectComboBox->removeItem(index);
    return;
}

void ccPointCloudTilesFileDlg::getMinimumCoordinates()
{
    QString functionName="ccPointCloudTilesFileDlg::getMinimumCoordinates";
    QString strAuxError;
    QString projectPath=projectsComboBox->currentText();
    if(projectPath.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select project before");
        QMessageBox::information(this,title,msg);
        return;
    }
    if(fabs(mMinZ-POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE)>1)
        return;
    mPtrPshift=new CCVector3d(0, 0, 0);
    double minFc,minSc,minTc;
    if(!mPtrPointCloudFileManager->getMinimumCoordinates(projectPath,minFc,minSc,minTc,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting minimum coordinates for project:\n%1\nError:\n%2")
                .arg(projectPath).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(minFc!=POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE
            ||minSc!=POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE
            ||minTc!=POINTCLOUDFILE_NO_DOUBLE_MINIMUM_VALUE)
    {
//        minFc=floor(minFc/1000.)*1000.;
//        minSc=floor(minSc/1000.)*1000.;
//        minTc=floor(minTc/100.)*100.;
        mMinX=minFc;
        mMinY=minSc;
        mMinZ=minTc;
        mMinZDecrement=POINTCLOUDFILE_MIN_Z_DECREMENT_STEP;
    }
}

void ccPointCloudTilesFileDlg::loadROIsLayer()
{
    QString functionName="ccPointCloudTilesFileDlg::loadROIsLayer";
    QString strAuxError;
    QString project=projectsComboBox->currentText();
    if(project.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        return;
    }
    QMap<QString, QString> roisWkt;
    if(!mPtrPointCloudFileManager->getROIsWktGeometry(project,
                                                      roisWkt,
                                                      strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting ROIs for project:\n%1\nError:\n%2")
                .arg(project).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(roisWkt.size()==0)
        return;
    getMinimumCoordinates();
    bool loadCoordinatesTransEnabled = false;
    FileIOFilter::LoadParameters parameters;
    {
        parameters.alwaysDisplayLoadDialog = true;
        parameters.shiftHandlingMode = ccGlobalShiftManager::DIALOG_IF_NECESSARY;
        parameters._coordinatesShift = mPtrPshift;
        parameters._coordinatesShiftEnabled = &loadCoordinatesTransEnabled;
        parameters.parentWidget = this;
    }
    if(mPtrCcROIsContainer)
    {
        bool autoRemove=true;
        mPtrApp->removeFromDB(mPtrCcROIsContainer, autoRemove);
        //        delete(mPtrCcProjectContainer);
        mPtrCcROIsContainer=nullptr;
    }
    mPtrCcROIsContainer=new ccHObject();
    FileIOFilter::ResetSesionCounter();
    unsigned sessionCounter = FileIOFilter::IncreaseSesionCounter();
    parameters.sessionStart = (sessionCounter == 1);
    //global shift
    bool preserveCoordinateShift = true;
    //    double minFc=mPtrPshift->x;
    //    double minSc=mPtrPshift->y;
    //    double minTc=mPtrPshift->z;
    //    CCVector3d Pmin(minFc,minSc,minTc);
    CCVector3d Pshift(0,0,0);
    CCVector3d Pmin(mMinX,mMinY,mMinZ);
    if (FileIOFilter::HandleGlobalShift(Pmin, Pshift, preserveCoordinateShift, parameters))
    {
        ccLog::Warning("[ROIs] Entities will be recentered! Translation: (%.2f ; %.2f ; %.2f)",
                       Pshift.x, Pshift.y, Pshift.z);
    }
    mPtrPshift->x=Pshift.x;
    mPtrPshift->y=Pshift.y;
    mPtrPshift->z=Pshift.z;
    QMap<QString,QString>::const_iterator iterRois=roisWkt.begin();
    mMinZDecrement-=POINTCLOUDFILE_MIN_Z_DECREMENT_STEP;
    while(iterRois!=roisWkt.end())
    {
        QString strRoiName=iterRois.key();
        QStringList strRoiNamesValues=strRoiName.split(";");
        QString roiFileName=strRoiNamesValues[2];
        QString roiName=QFileInfo(roiFileName).baseName();
        QString wkt=iterRois.value();
        QByteArray byteArrayWktGeometry = wkt.toUtf8();
        char *charsWktGeometry = byteArrayWktGeometry.data();
        OGRGeometry* ptrGeometry=NULL;
        OGRwkbGeometryType geometryType=wkbPolygon;
        ptrGeometry=OGRGeometryFactory::createGeometry(geometryType);
        if(OGRERR_NONE!=ptrGeometry->importFromWkt(&charsWktGeometry))
        {
            delete mPtrCcROIsContainer;
            mPtrCcROIsContainer=nullptr;
            QString title=functionName;
            QString msg=QObject::tr("Error importing geometry for ROI: %1\nfrom wkt:\n%2\nfor project:\n%3")
                    .arg(roiName).arg(wkt).arg(project);
            QMessageBox::information(this,title,msg);
            return;
        }
        OGRLinearRing* ptrObjectExtLR=((OGRPolygon*)ptrGeometry)->getExteriorRing();
        int nop=ptrObjectExtLR->getNumPoints();
        QVector<QVector<double> > objectPoints;
        //vertices
        ccPointCloud* vertices = new ccPointCloud("vertices");
        if (!vertices->reserve(nop))
        {
            delete vertices;
            delete mPtrCcROIsContainer;
            mPtrCcROIsContainer=nullptr;
            QString title=functionName;
            QString msg=QObject::tr("Error creating vertices for ROI: %1\nfor project:\n%2")
                    .arg(roiName).arg(project);
            QMessageBox::information(this,title,msg);
            OGRGeometryFactory::destroyGeometry(ptrGeometry);
            return;
        }
        std::vector<CCVector3> points;
        points.resize(nop);
        //        for(int np=0;np<(nop-1);np++) // -1 porque se repite al final el primer punto
        for(int np=0;np<nop;np++) // -1 porque se repite al final el primer punto
        {
            double fc=ptrObjectExtLR->getX(np);
            double sc=ptrObjectExtLR->getY(np);
            double tc=ptrObjectExtLR->getZ(np);
            if(tc<=0.) tc=mMinZ+POINTCLOUDFILE_ROIS_ZMIN_OFFSET;//+mMinZDecrement;
            points[np].x=static_cast<PointCoordinateType>(fc+mPtrPshift->x);
            points[np].y=static_cast<PointCoordinateType>(sc+mPtrPshift->y);
            points[np].z=static_cast<PointCoordinateType>(tc+mPtrPshift->z);
            vertices->addPoint(points[np]);
        }
        OGRGeometryFactory::destroyGeometry(ptrGeometry);
        vertices->setEnabled(false);
        if (preserveCoordinateShift)
        {
            vertices->setGlobalShift(*mPtrPshift);
        }
        //polyline
        ccPolyline* poly = new ccPolyline(vertices);
        poly->addChild(vertices);
        if (preserveCoordinateShift)
        {
            poly->setGlobalShift(*mPtrPshift); //shouldn't be necessary but who knows ;)
        }

        if (!poly->reserve(nop))
        {
            delete poly;
            delete vertices;
            delete mPtrCcROIsContainer;
            mPtrCcROIsContainer=nullptr;
            QString title=functionName;
            QString msg=QObject::tr("Error creating poly for ROI: %1\nfor project:\n%2")
                    .arg(roiName).arg(project);
            QMessageBox::information(this,title,msg);
            return;
        }
        poly->addPointIndex(0, static_cast<unsigned>(nop));
        poly->showSF(vertices->sfShown());
        QString name = QString("%1").arg(roiName);
        poly->setName(name);
        poly->setClosed(true);
        poly->set2DMode(false);
//        poly->setColor(ccColor::FromQColor(QColor(255,0,0,255)));
        mPtrCcROIsContainer->addChild(poly);
        iterRois++;
    }
    unsigned childCount = mPtrCcROIsContainer->getChildrenNumber();
    if (childCount != 0)
    {
        //we set the main container name as the full filename (with path)
        //            container->setName(QString("%1").arg(POINT_CLOUD_TILES_FILE_ROIS_LAYER_NAME));
        for (unsigned i = 0; i < childCount; ++i)
        {
            ccHObject* child = mPtrCcROIsContainer->getChild(i);
            QString newName = child->getName();
            //			if (newName.startsWith("unnamed"))
            //			{
            //				//we automatically replace occurrences of 'unnamed' in entities names by the base filename (no path, no extension)
            //				newName.replace(QString("unnamed"), fi.baseName());
            //				child->setName(newName);
            //			}
            //			else if (newName.isEmpty())
            //			{
            //				//just in case
            //				child->setName(fi.baseName());
            //			}
        }
        //            mPtrApp->addToDB(container, true, true, false);
        QString roisLayerName=mProjectName+":ROIs";
        mPtrCcROIsContainer->setName(QString("%1").arg(roisLayerName));
        mPtrApp->addToDB(mPtrCcROIsContainer, true, true, false);
//        ccHObjectCaster::ToPolyline(mPtrCcROIsContainer->getChild(0))->setColor(ccColor::FromQColor(QColor(255,0,0,255)));
        mPtrApp->redrawAll();
    }
    //        else
    //        {
    //            delete container;
    //            container = nullptr;
    //        }
//    mPtrApp->removeFromDB(mPtrCcProjectContainer, false);
//    mPtrApp->addToDB(mPtrCcProjectContainer, true, true, false);
//    mPtrCcProjectContainer->draw();
//    mPtrCcProjectContainer->refreshDisplay();
    return;
}

void ccPointCloudTilesFileDlg::loadTilesLayer()
{
    QString functionName="ccPointCloudTilesFileDlg::loadTilesLayer";
    QString strAuxError;
    QString project=projectsComboBox->currentText();
    if(project.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        return;
    }
    QMap<QString, QString> tilesWkt;
    if(!mPtrPointCloudFileManager->getTilesWktGeometry(project,
                                                       tilesWkt,
                                                       strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting Tiles for project:\n%1\nError:\n%2")
                .arg(project).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    if(tilesWkt.size()==0)
        return;
    getMinimumCoordinates();
    bool loadCoordinatesTransEnabled = false;
    FileIOFilter::LoadParameters parameters;
    {
        parameters.alwaysDisplayLoadDialog = true;
        parameters.shiftHandlingMode = ccGlobalShiftManager::DIALOG_IF_NECESSARY;
        parameters._coordinatesShift = mPtrPshift;
        parameters._coordinatesShiftEnabled = &loadCoordinatesTransEnabled;
        parameters.parentWidget = this;
    }
    if(mPtrCcTilesContainer)
    {
        bool autoRemove=true;
        mPtrApp->removeFromDB(mPtrCcTilesContainer, autoRemove);
        //        delete(mPtrCcProjectContainer);
        mPtrCcTilesContainer=nullptr;
    }
    if(mPtrCcTilesCenterPointsContainer)
    {
        bool autoRemove=true;
        mPtrApp->removeFromDB(mPtrCcTilesCenterPointsContainer, autoRemove);
        //        delete(mPtrCcProjectContainer);
        mPtrCcTilesCenterPointsContainer=nullptr;
    }
    mPtrCcTilesContainer=new ccHObject();
    mPtrCcTilesCenterPointsContainer=new ccHObject();
    FileIOFilter::ResetSesionCounter();
    unsigned sessionCounter = FileIOFilter::IncreaseSesionCounter();
    parameters.sessionStart = (sessionCounter == 1);
    //global shift
    bool preserveCoordinateShift = true;
//    double minFc=mPtrPshift->x;
//    double minSc=mPtrPshift->y;
//    double minTc=mPtrPshift->z;
//    CCVector3d Pmin(minFc,minSc,minTc);
    CCVector3d Pshift(0,0,0);
    CCVector3d Pmin(mMinX,mMinY,mMinZ);
    if (FileIOFilter::HandleGlobalShift(Pmin, Pshift, preserveCoordinateShift, parameters))
    {
        ccLog::Warning("[Tiles] Entities will be recentered! Translation: (%.2f ; %.2f ; %.2f)",
                       Pshift.x, Pshift.y, Pshift.z);
    }
    mPtrPshift->x=Pshift.x;
    mPtrPshift->y=Pshift.y;
    mPtrPshift->z=Pshift.z;
    mMinZDecrement-=POINTCLOUDFILE_MIN_Z_DECREMENT_STEP;
    ccPointCloud* centerPoints = new ccPointCloud("center");
    if (!centerPoints->reserve(tilesWkt.size()))
    {
        delete mPtrCcTilesContainer;
        mPtrCcTilesContainer=nullptr;
        delete mPtrCcTilesCenterPointsContainer;
        mPtrCcTilesCenterPointsContainer=nullptr;
        QString title=functionName;
        QString msg=QObject::tr("Error creating center points for tiles for project:\n%2")
                .arg(project);
        QMessageBox::information(this,title,msg);
        return;
    }
    QMap<QString,QString>::const_iterator iterTiles=tilesWkt.begin();
    while(iterTiles!=tilesWkt.end())
    {
        QString tileName=iterTiles.key();
        QString wkt=iterTiles.value();
        QByteArray byteArrayWktGeometry = wkt.toUtf8();
        char *charsWktGeometry = byteArrayWktGeometry.data();
        OGRGeometry* ptrGeometry=NULL;
        OGRwkbGeometryType geometryType=wkbPolygon;
        ptrGeometry=OGRGeometryFactory::createGeometry(geometryType);
        if(OGRERR_NONE!=ptrGeometry->importFromWkt(&charsWktGeometry))
        {
            delete centerPoints;
            delete mPtrCcTilesContainer;
            mPtrCcTilesContainer=nullptr;
            delete mPtrCcTilesCenterPointsContainer;
            mPtrCcTilesCenterPointsContainer=nullptr;
            QString title=functionName;
            QString msg=QObject::tr("Error importing geometry for tile: %1\nfrom wkt:\n%2\nfor project:\n%3")
                    .arg(tileName).arg(wkt).arg(project);
            QMessageBox::information(this,title,msg);
            return;
        }
        OGRLinearRing* ptrObjectExtLR=((OGRPolygon*)ptrGeometry)->getExteriorRing();
        int nop=ptrObjectExtLR->getNumPoints();
        QVector<QVector<double> > objectPoints;
        //vertices
        ccPointCloud* vertices = new ccPointCloud("vertices");
        if (!vertices->reserve(nop))
        {
            delete centerPoints;
            delete vertices;
            delete mPtrCcTilesContainer;
            mPtrCcTilesContainer=nullptr;
            delete mPtrCcTilesCenterPointsContainer;
            mPtrCcTilesCenterPointsContainer=nullptr;
            QString title=functionName;
            QString msg=QObject::tr("Error creating vertices for ROI: %1\nfor project:\n%2")
                    .arg(tileName).arg(project);
            QMessageBox::information(this,title,msg);
            OGRGeometryFactory::destroyGeometry(ptrGeometry);
            return;
        }
        std::vector<CCVector3> points;
        points.resize(nop);
        //        for(int np=0;np<(nop-1);np++) // -1 porque se repite al final el primer punto
        double meanFc=0.;
        double meanSc=0.;
        for(int np=0;np<nop;np++) // -1 porque se repite al final el primer punto
        {
            double fc=ptrObjectExtLR->getX(np);
            double sc=ptrObjectExtLR->getY(np);
            double tc=ptrObjectExtLR->getZ(np);
            if(tc<=0.) tc=mMinZ+POINTCLOUDFILE_TILES_ZMIN_OFFSET;//+mMinZDecrement;
            points[np].x=static_cast<PointCoordinateType>(fc+mPtrPshift->x);
            points[np].y=static_cast<PointCoordinateType>(sc+mPtrPshift->y);
            points[np].z=static_cast<PointCoordinateType>(tc+mPtrPshift->z);
            vertices->addPoint(points[np]);
            meanFc+=points[np].x;
            meanSc+=points[np].y;
        }
        OGRGeometryFactory::destroyGeometry(ptrGeometry);
        meanFc=meanFc/(double(nop));
        meanSc=meanSc/(double(nop));
        CCVector3 centerPoint;
        centerPoint.x=meanFc;
        centerPoint.y=meanSc;
        centerPoint.z=mMinZ+POINTCLOUDFILE_TILES_ZMIN_OFFSET;
        centerPoints->addPoint(centerPoint);
        vertices->setEnabled(false);
        if (preserveCoordinateShift)
        {
            vertices->setGlobalShift(*mPtrPshift);
        }
        //polyline
        ccPolyline* poly = new ccPolyline(vertices);
        poly->addChild(vertices);
        if (preserveCoordinateShift)
        {
            poly->setGlobalShift(*mPtrPshift); //shouldn't be necessary but who knows ;)
        }

        if (!poly->reserve(nop))
        {
            delete centerPoints;
            delete poly;
            delete vertices;
            delete mPtrCcTilesContainer;
            mPtrCcTilesContainer=nullptr;
            delete mPtrCcTilesCenterPointsContainer;
            mPtrCcTilesCenterPointsContainer=nullptr;
            QString title=functionName;
            QString msg=QObject::tr("Error creating poly for tile: %1\nfor project:\n%2")
                    .arg(tileName).arg(project);
            QMessageBox::information(this,title,msg);
            return;
        }
        poly->addPointIndex(0, static_cast<unsigned>(nop));
        poly->showSF(vertices->sfShown());
        QString name = QString("%1").arg(tileName);
        poly->setName(name);
        poly->setClosed(true);
        poly->set2DMode(false);
//        poly->setColor(ccColor::FromQColor(QColor(255,0,0,255)));
        mPtrCcTilesContainer->addChild(poly);
        iterTiles++;
    }
//    centerPoints->setColor(ccColor::FromQColor(QColor(255,0,0,255)));
    unsigned childCount = mPtrCcTilesContainer->getChildrenNumber();
    if (childCount != 0)
    {
        //we set the main container name as the full filename (with path)
        //            container->setName(QString("%1").arg(POINT_CLOUD_TILES_FILE_ROIS_LAYER_NAME));
        for (unsigned i = 0; i < childCount; ++i)
        {
            ccHObject* child = mPtrCcTilesContainer->getChild(i);
            QString newName = child->getName();
            //			if (newName.startsWith("unnamed"))
            //			{
            //				//we automatically replace occurrences of 'unnamed' in entities names by the base filename (no path, no extension)
            //				newName.replace(QString("unnamed"), fi.baseName());
            //				child->setName(newName);
            //			}
            //			else if (newName.isEmpty())
            //			{
            //				//just in case
            //				child->setName(fi.baseName());
            //			}
        }
        //            mPtrApp->addToDB(container, true, true, false);
        QString tilesLayerName=mProjectName+":Tiles";
        mPtrCcTilesContainer->setName(QString("%1").arg(tilesLayerName));
        mPtrApp->addToDB(mPtrCcTilesContainer, true, true, false);
        QString tilesCenterPointsLayerName=mProjectName+":TilesCenterPoints";
        if (preserveCoordinateShift)
        {
            centerPoints->setGlobalShift(*mPtrPshift);
        }
        mPtrCcTilesCenterPointsContainer->setName(QString("%1").arg(tilesCenterPointsLayerName));
        mPtrCcTilesCenterPointsContainer->addChild(centerPoints);
        mPtrApp->addToDB(mPtrCcTilesCenterPointsContainer, true, true, false);
        mPtrApp->redrawAll();
    }
    //        else
    //        {
    //            delete container;
    //            container = nullptr;
    //        }
//    mPtrApp->removeFromDB(mPtrCcProjectContainer, false);
//    mPtrApp->addToDB(mPtrCcProjectContainer, true, true, false);
//    mPtrCcProjectContainer->draw();
//    mPtrCcProjectContainer->refreshDisplay();
    resetUI();
    return;
}

void ccPointCloudTilesFileDlg::setVerticalCRS(int horizontalCRSEpsgCode)
{
    verticalCRSsComboBox->clear();
    verticalCRSsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT);
    QVector<int> verticalCrsEpsgCodes;
    mPtrCrsTools->getVerticalCrsEpsgCodesFromCrsEpsgCode(horizontalCRSEpsgCode,verticalCrsEpsgCodes);
    for(int i=0;i<verticalCrsEpsgCodes.size();i++)
    {
        QString verticalCrsId=POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX+QString::number(verticalCrsEpsgCodes[i]);
        verticalCRSsComboBox->addItem(verticalCrsId);
    }
    int defaultVerticalCrsPosition=verticalCRSsComboBox->findText(POINT_CLOUD_TILES_FILE_DIALOG_DEFAULT_VERTICAL_CRS);
    if(defaultVerticalCrsPosition!=-1)
        verticalCRSsComboBox->setCurrentIndex(defaultVerticalCrsPosition);
}

void ccPointCloudTilesFileDlg::setAddPCFsVerticalCRS(int horizontalCRSEpsgCode)
{
    addPCFsVerticalCRSsComboBox->clear();
    addPCFsVerticalCRSsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT);
    QVector<int> verticalCrsEpsgCodes;
    mPtrCrsTools->getVerticalCrsEpsgCodesFromCrsEpsgCode(horizontalCRSEpsgCode,verticalCrsEpsgCodes);
    for(int i=0;i<verticalCrsEpsgCodes.size();i++)
    {
        QString verticalCrsId=POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX+QString::number(verticalCrsEpsgCodes[i]);
        addPCFsVerticalCRSsComboBox->addItem(verticalCrsId);
    }
    int defaultVerticalCrsPosition=addPCFsVerticalCRSsComboBox->findText(POINT_CLOUD_TILES_FILE_DIALOG_DEFAULT_VERTICAL_CRS);
    if(defaultVerticalCrsPosition!=-1)
        addPCFsVerticalCRSsComboBox->setCurrentIndex(defaultVerticalCrsPosition);
}

void ccPointCloudTilesFileDlg::setPpToolsIPFCsVerticalCRS(int horizontalCRSEpsgCode)
{
    ppToolsIPCFsVerticalCRSsComboBox->clear();
    ppToolsIPCFsVerticalCRSsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT);
    QVector<int> verticalCrsEpsgCodes;
    mPtrCrsTools->getVerticalCrsEpsgCodesFromCrsEpsgCode(horizontalCRSEpsgCode,verticalCrsEpsgCodes);
    for(int i=0;i<verticalCrsEpsgCodes.size();i++)
    {
        QString verticalCrsId=POINT_CLOUD_TILES_FILE_DIALOG_EPSG_PREFIX+QString::number(verticalCrsEpsgCodes[i]);
        ppToolsIPCFsVerticalCRSsComboBox->addItem(verticalCrsId);
    }
    int defaultVerticalCrsPosition=ppToolsIPCFsVerticalCRSsComboBox->findText(POINT_CLOUD_TILES_FILE_DIALOG_DEFAULT_VERTICAL_CRS);
    if(defaultVerticalCrsPosition!=-1)
        ppToolsIPCFsVerticalCRSsComboBox->setCurrentIndex(defaultVerticalCrsPosition);
}
/*
void ccPointCloudTilesFileDlg::setSelectedCandidateForPreviousModel()
{
    QString functionName="ccPointCloudTilesFileDlg::setSelectedCandidateForPreviousModel";
    QString strAuxError;
    if(mPreviousModelName.isEmpty()
            ||mPreviousModelName.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
        return;
    int previousSelectedModelObjectDbId=mSelectedModelObjectDbIdByModelName[mPreviousModelName];
    if(mPtrCcCubeContainerByCubeDbId.contains(previousSelectedModelObjectDbId))
        return;
    for(int nc=0;nc<mModelObjectsDbIdByModelName[mPreviousModelName].size();nc++)
    {
        int modelObjectDbId=mModelObjectsDbIdByModelName[mPreviousModelName][nc];
        if(modelObjectDbId==previousSelectedModelObjectDbId) continue; // no puede ser
        if(mPtrCcCubeContainerByCubeDbId.contains(modelObjectDbId))
        {
            ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbId];
            bool autoRemove=true;
            mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
            ptrCcHObject=nullptr;
            mPtrCcCubeContainerByCubeDbId.remove(modelObjectDbId);
            break;
        }
    }
    AicedroneModelDb::ModelObject* ptrCandidateModelObject=mPtrModelObjectsByDbId[previousSelectedModelObjectDbId];
    AicedroneModelDb::ParametricGeometry* ptrParametricGeometry=ptrCandidateModelObject->getParametricGeometry();
    QString parametricGeometryType=ptrParametricGeometry->getType();
    if(parametricGeometryType.compare(AICEDRONE_PARAMETRIC_GEOMETRY_TYPE_CUBE3D,Qt::CaseInsensitive)==0)
    {
        QVector<double> parameters;
        ptrParametricGeometry->getParameters(parameters);
        if(parameters.size()<24) // ahora son mas
        {
            QString title=functionName;
            QString msg=QObject::tr("Error recovering points for cube: %1 from model database file:\n%2\nError:\n%3")
                    .arg(mPreviousModelName).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
        int np=0;
        QVector<QVector<double> > cubePoints;
        while(np<22)
        {
            double x=parameters[np];
            double y=parameters[np+1];
            double z=parameters[np+2];
            QVector<double> point(3);
            point[0]=x;
            point[1]=y;
            point[2]=z;
            cubePoints.push_back(point);
            np=np+3;
        }
        if(!addCube(previousSelectedModelObjectDbId,mPreviousModelName,cubePoints,strAuxError)) // never
        {
            QString title=functionName;
            QString msg=QObject::tr("Error adding cube: %1 from model database file:\n%2\nError:\n%3")
                    .arg(QString::number(previousSelectedModelObjectDbId))
                    .arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
            QMessageBox::information(this,title,msg);
            return;
        }
    }
}
*/
void ccPointCloudTilesFileDlg::uncheckModelDbObject()
{
    QString functionName="ccPointCloudTilesFileDlg::uncheckModelDbObject";
    QString strAuxError;
    QString modelObjectNameToEnable=modelDbObjectComboBox->currentText();
    int index=modelDbObjectComboBox->findText(modelObjectNameToEnable);
    if(index==0) return;
//    removeModelDbObjectPushButton->setEnabled(false);
    QString modelDbFile=modelDbsComboBox->currentText();
    if(modelDbFile.compare(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QString title=functionName;
        QString msg=QObject::tr("Select model database file before");
        QMessageBox::information(this,title,msg);
        return;
    }
    int modelObjectDbIdToEnable=mSelectedModelObjectDbIdByModelName[modelObjectNameToEnable];
    if(modelObjectDbIdToEnable==-1)
        return;
//    AicedroneModelDb::ModelObject* ptrModelObject=mPtrModelObjectsByDbId[modelObjectDbIdToEnable];
    if(!mPtrMsDb->uncheckObject(modelObjectDbIdToEnable,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Unchecking object: %1 from model database file:\n%2\nError:\n%3")
                .arg(modelObjectNameToEnable).arg(mPtrMsDb->mPtrDb->getFileName()).arg(strAuxError);
        QMessageBox::information(this,title,msg);
        return;
    }
    modelDbObjectComboBox->setCurrentIndex(0);
    // ya se ha eliminado en la funcion remove
//    delete(ptrModelObject);
//    ptrModelObject=nullptr;

//    mPtrModelObjectsByDbId.remove(modelObjectDbIdToRemove);
//    if(mPtrCcCubeContainerByCubeDbId.contains(modelObjectDbIdToRemove))
//    {
//        ccHObject* ptrCcHObject=mPtrCcCubeContainerByCubeDbId[modelObjectDbIdToRemove];
//        bool autoRemove=true;
//        mPtrApp->removeFromDB(ptrCcHObject, autoRemove);
//        ptrCcHObject=nullptr;
//        mPtrCcCubeContainerByCubeDbId.remove(modelObjectDbIdToRemove);
//    }
//    QMap<int,ccHObject*> mPtrCcCubeContainerByCubeDbId;
//    int pos=mVisibleModelObjectsComboCheckBox->findText(modelObjectNameToEnable);
//    if(pos>-1)
//    {
//        mVisibleModelObjectsComboCheckBox->checkItem(pos);
//        mVisibleModelObjectsComboCheckBox->enableItem(pos);
//    }
//    modelDbObjectComboBox->removeItem(index);
    return;
}
	
void ccPointCloudTilesFileDlg::initialize()
{
    QString functionName="ccPointCloudTilesFileDlg::initialize";
    mPtrPointCloudFileManager=PCFile::PointCloudFileManager::getInstance();
    QString strAuxError;
    mBasePath=QDir::currentPath()+"/plugins";
    if(!mPtrPointCloudFileManager->setBasePath(mBasePath,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Setting PointCloudFileManager:\nError:\n%1").arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    if(!mPtrPointCloudFileManager->initializeCrsTools(strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Initializing CrsTools:\nError:\n%1").arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    QDir currentDir(QDir::currentPath());
    mPtrCrsTools=libCRS::CRSTools::getInstance();
    QString settingsFileName=mBasePath+"/"+POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_FILENAME;
//    debug
    {
        QString title=functionName;
        QString msg=QObject::tr("Setting file:\n%1").arg(settingsFileName);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
    }
    mPtrSettings=new QSettings(settingsFileName,QSettings::IniFormat,this);
    mLastPath=QDir::currentPath();
    QString lastPath=mPtrSettings->value(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH).toString();
    if(lastPath.isEmpty())
    {
        mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
        mPtrSettings->sync();
    }
    else
    {
        if(!currentDir.exists(lastPath))
        {
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LAST_PATH,mLastPath);
            mPtrSettings->sync();
        }
        else
            mLastPath=lastPath;
    }
    // tempPath
    mTempPath=mPtrSettings->value(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_TEMP_PATH).toString();
    if(mTempPath.isEmpty())
    {
        if(!currentDir.exists(mTempPath))
        {
            mTempPath="";
        }
    }
    if(mTempPath.isEmpty())
    {
        mTempPath=mBasePath+"/"+POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_DEFAULT_TEMP_FOLDER;
        if(!currentDir.exists(mTempPath))
        {
            if(!currentDir.mkpath(mTempPath))
            {
                QString title=functionName;
                QString msg=QObject::tr("Error creating default temporal path:\n%1")
                        .arg(mTempPath);
                msg+=QObject::tr("\nContact the author:\n");
                msg+=QTOOLS_AUTHOR_MAIL;
                QMessageBox::information(this,title,msg);
                return;
            }
        }
    }
    if(!mPtrPointCloudFileManager->setTempPath(mTempPath,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Setting temporal path:\n%1\nError:\n%2")
                .arg(mTempPath).arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    pmTemporalPathLineEdit->setText(mTempPath);
    mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_TEMP_PATH,mTempPath);
    mPtrSettings->sync();
    //outputPath
    mOutputPath=mPtrSettings->value(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_OUTPUT_PATH).toString();
    if(mOutputPath.isEmpty())
    {
        if(!currentDir.exists(mOutputPath))
        {
            mOutputPath="";
        }
    }
    if(mOutputPath.isEmpty())
    {
        mOutputPath=mBasePath+"/"+POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_DEFAULT_OUTPUT_FOLDER;
        if(!currentDir.exists(mOutputPath))
        {
            if(!currentDir.mkpath(mOutputPath))
            {
                QString title=functionName;
                QString msg=QObject::tr("Error creating default output path:\n%1")
                        .arg(mOutputPath);
                msg+=QObject::tr("\nContact the author:\n");
                msg+=QTOOLS_AUTHOR_MAIL;
                QMessageBox::information(this,title,msg);
                return;
            }
        }
    }
    if(!mPtrPointCloudFileManager->setOutputPath(mOutputPath,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Setting output path:\n%1\nError:\n%2")
                .arg(mOutputPath).arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    pmOutputPathLineEdit->setText(mOutputPath);
    mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_OUTPUT_PATH,mOutputPath);
    mPtrSettings->sync();
    // orthomosaic
    QString orthomosaicFileName=mPtrSettings->value(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_ORTHOMOSAIC).toString();
    if(!orthomosaicFileName.isEmpty())
    {
        if(!QFile::exists(orthomosaicFileName))
        {
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_ORTHOMOSAIC,"");
            mPtrSettings->sync();
            orthomosaicFileName="";
        }
        else
        {
            orthomosaicLineEdit->setText(orthomosaicFileName);
        }
    }
    // lastoolsPath
    QString lastoolsPath=mPtrSettings->value(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LASTOOLS_PATH).toString();
    if(!lastoolsPath.isEmpty())
    {
        if(!currentDir.exists(lastoolsPath))
        {
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_LASTOOLS_PATH,"");
            mPtrSettings->sync();
            lastoolsPath="";
        }
        else
        {
            if(!mPtrPointCloudFileManager->setLastoolsPath(lastoolsPath,strAuxError))
            {
                QString title=functionName;
                QString msg=QObject::tr("Setting lastools path:\n%1\nError:\n%2")
                        .arg(lastoolsPath).arg(strAuxError);
                msg+=QObject::tr("\nContact the author:\n");
                msg+=QTOOLS_AUTHOR_MAIL;
                QMessageBox::information(this,title,msg);
                return;
            }
            mLastoolsPath=lastoolsPath;
            ppToolsLastoolsPathLineEdit->setText(lastoolsPath);
        }
    }
    projectsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    QString strProjects=mPtrSettings->value(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_PROJECTS).toString();
    if(!strProjects.isEmpty())
    {
        QStringList projects=strProjects.split(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS);
        if(projects.size()>0)
        {
            QString strNewProjects;
            for(int i=0;i<projects.size();i++)
            {
                if(currentDir.exists(projects[i]))
                {
                    if(!strNewProjects.isEmpty()) strNewProjects+=POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS;
                    strNewProjects+=projects[i];
                    mProjects.append(projects[i]);
                    projectsComboBox->addItem(projects[i]);
                }
            }
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_PROJECTS,strNewProjects);
            mPtrSettings->sync();
        }
    }

//    mPtrPointCloudFileManager->setMaximumNumberOfPoints(mNumberOfPointsInDemoVersion);
    QVector<QString> projectTypes;
    if(!mPtrPointCloudFileManager->getProjectTypes(projectTypes,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting project types:\nError:\n%1").arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    if(projectTypes.size()==1)
    {
        projectTypeComboBox->addItem(projectTypes[0]);
        projectTypeComboBox->setEnabled(false);
    }
    else
    {
        projectTypeComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
        for(int i=0;i<projectTypes.size();i++)
        {
            projectTypeComboBox->addItem(projectTypes[i]);
        }
    }
    QVector<int> gridSizes;
    if(!mPtrPointCloudFileManager->getGridSizes(gridSizes,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Getting grid sizes:\nError:\n%1").arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    gridSizeComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    for(int i=0;i<gridSizes.size();i++)
    {
        gridSizeComboBox->addItem(QString::number(gridSizes[i]));
    }
    QList<int> horizontalCRSsEpsgCodes=mPtrCrsTools->getCrsEpsgCodes();
    for(int i=0;i<horizontalCRSsEpsgCodes.size();i++)
    {
        QString crsId;
        if(!mPtrCrsTools->getCrsId(horizontalCRSsEpsgCodes[i],crsId,strAuxError))
        {
            QString title=functionName;
            QString msg=QObject::tr("Getting CRS id for epsg code: %1\nError:\n%2")
                    .arg(QString::number(horizontalCRSsEpsgCodes[i])).arg(strAuxError);
            msg+=QObject::tr("\nContact the author:\n");
            msg+=QTOOLS_AUTHOR_MAIL;
            QMessageBox::information(this,title,msg);
            return;
        }
        bool isCrsProjected=false;
        if(!mPtrCrsTools->isCrsProjected(crsId,isCrsProjected,strAuxError))
        {
            QString title=functionName;
            QString msg=QObject::tr("Getting if CRS: %1 is projected\nError:\n%2")
                    .arg(crsId).arg(strAuxError);
            msg+=QObject::tr("\nContact the author:\n");
            msg+=QTOOLS_AUTHOR_MAIL;
            QMessageBox::information(this,title,msg);
            return;
        }
        if(isCrsProjected)
        {
            horizontalCRSsComboBox->addItem(crsId);
            addPCFsHorizontalCRSsComboBox->addItem(crsId);
            ppToolsIPCFsHorizontalCRSsComboBox->addItem(crsId);
        }
    }
    verticalCRSsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT);
    addPCFsVerticalCRSsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT);
    ppToolsIPCFsVerticalCRSsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_ELLIPSOID_HEIGHT);

    modelDbsComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    QString strModelDbs=mPtrSettings->value(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_MODEL_DATABASES).toString();
    if(!strModelDbs.isEmpty())
    {
        QStringList modelDbs=strModelDbs.split(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS);
        if(modelDbs.size()>0)
        {
            QString strNewModelDbs;
            for(int i=0;i<modelDbs.size();i++)
            {
                if(QFile::exists(modelDbs[i]))
                {
                    if(!strNewModelDbs.isEmpty()) strNewModelDbs+=POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_STRING_SEPARATOR_PROJECTS;
                    strNewModelDbs+=modelDbs[i];
                    mModelDbs.append(modelDbs[i]);
                    modelDbsComboBox->addItem(modelDbs[i]);
                }
            }
            mPtrSettings->setValue(POINT_CLOUD_TILES_FILE_DIALOG_SETTINGS_TAG_MODEL_DATABASES,strNewModelDbs);
            mPtrSettings->sync();
        }
    }

    QString title=POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_CLASSES_TITLE;
    mVisibleModelClassesComboCheckBox=new QComboCheckBox(title,
                                                           visibleModelClassesFrame);
    connect(mVisibleModelClassesComboCheckBox,SIGNAL(pressChecked(int,bool,QString,QString)),
            this, SLOT(onVisibleModelClassesComboCheckBox(int,bool,QString,QString)));

    title=POINT_CLOUD_TILES_FILE_DIALOG_COMBOCHECKBOX_VISIBLE_MODEL_OBJECTS_TITLE;
    mVisibleModelObjectsComboCheckBox=new QComboCheckBox(title,
                                                           visibleModelObjectsFrame);
//    mVisibleModelObjectsComboCheckBox->setMinimumWidth(120);
//    mVisibleModelObjectsComboCheckBox->setMaximumWidth(120);
    connect(mVisibleModelObjectsComboCheckBox,SIGNAL(pressChecked(int,bool,QString,QString)),
            this, SLOT(onVisibleModelObjectsComboCheckBox(int,bool,QString,QString)));
    connect(visibleModelObjectsNonePushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::visibleModelObjectsNone);
    connect(visibleModelObjectsAllPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::visibleModelObjectsAll);

    connect(acceptDialogPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::acceptClicked);
    connect(closeDialogPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::closeClicked);
    connect(horizontalCRSsComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(onHorizontalCRSsComboBoxCurrentIndexChanged(int)));
    connect(verticalCRSsComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(onVerticalCRSsComboBoxCurrentIndexChanged(int)));
    connect(addPCFsHorizontalCRSsComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(onAddPCFsHorizontalCRSsComboBoxCurrentIndexChanged(int)));
    connect(ppToolsIPCFsHorizontalCRSsComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(onPpToolsIPCFsHorizontalCRSsComboBoxCurrentIndexChanged(int)));
    int defaultHorizontalCrsPosition=horizontalCRSsComboBox->findText(POINT_CLOUD_TILES_FILE_DIALOG_DEFAULT_CRS);
    if(defaultHorizontalCrsPosition!=-1)
        horizontalCRSsComboBox->setCurrentIndex(defaultHorizontalCrsPosition);
    connect(projectParametersPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::selectProjectParameters);
    connect(projectPathPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::selectProjectPath);
    connect(roisPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::selectROIs);
    numberOfRoisLineEdit->setText(QString::number(mROIsShapefiles.size()));
    connect(roisPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::selectROIs);
    connect(pmTemporalPathPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::selectTempPath);
    connect(pmOutputPathPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::selectOutputPath);
    QVector<QString> lastoolsCommands;
    if(!mPtrPointCloudFileManager->getLastoolsCommands(lastoolsCommands,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error getting lastools commands:\nError:\n%1").arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    ppToolsLastoolsCommandComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    for(int nltc=0;nltc<lastoolsCommands.size();nltc++)
    {
        ppToolsLastoolsCommandComboBox->addItem(lastoolsCommands[nltc]);
    }
    connect(ppToolsLastoolsCommandComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(ppToolsSelectLastoolsCommand(int)));
    ppToolsLastoolsCommandComboBox->setCurrentIndex(0);
    ppToolsLastoolsCommandParametersPushButton->setEnabled(false);
    connect(ppToolsLastoolsCommandParametersPushButton,&QPushButton::clicked,
            this, &ccPointCloudTilesFileDlg::ppToolsSelectLastoolsCommandParameters);

    QVector<QString> internalCommands;
    if(!mPtrPointCloudFileManager->getInternalCommands(internalCommands,strAuxError))
    {
        QString title=functionName;
        QString msg=QObject::tr("Error getting internal commands:\nError:\n%1").arg(strAuxError);
        msg+=QObject::tr("\nContact the author:\n");
        msg+=QTOOLS_AUTHOR_MAIL;
        QMessageBox::information(this,title,msg);
        return;
    }
    ppToolsInternalCommandComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    for(int nic=0;nic<internalCommands.size();nic++)
    {
        ppToolsInternalCommandComboBox->addItem(internalCommands[nic]);
    }
    connect(ppToolsInternalCommandComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(ppToolsSelectInternalCommand(int)));
    ppToolsInternalCommandComboBox->setCurrentIndex(0);
    ppToolsInternalCommandParametersPushButton->setEnabled(false);
    connect(ppToolsInternalCommandParametersPushButton,&QPushButton::clicked,
            this, &ccPointCloudTilesFileDlg::ppToolsSelectInternalCommandParameters);

    connect(ppToolsLastoolsPathPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsSelectLastoolsPath);
    connect(ppToolsOPCFsOutputFilePushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsSelectOutputFile);
    connect(ppToolsOPCFsOutputPathPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsSelectOutputPath);
    connect(ppToolsSelectIPCFsPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsSelectPCFiles);
    connect(ppToolsTabWidget,&QTabWidget::currentChanged,this,&ccPointCloudTilesFileDlg::ppToolsTabWidgetChanged);
    connect(ppToolsOPCFsPrefixPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsSelectPrefix);
    connect(ppToolsOPCFsSuffixPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsSelectSuffix);
    connect(addProcessToListPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsAddProcessToList);
    connect(processListEditionPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsProcessListEdition);
    connect(runProcessListPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::ppToolsRunProcessList);

    connect(projectsComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(onProjectsComboBoxCurrentIndexChanged(int)));
    connect(addProjectPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::addProject);
    connect(createProjectPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::createProject);
    connect(openProjectPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::openProject);
    connect(closeProjectPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::closeProject);
    connect(removeProjectPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::removeProject);
    connect(selectPCFsPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::selectPCFs);
    connect(addPCFsProcessPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::addPointCloudFiles);
    connect(orthomosaicPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::selectOrthomosaic);
    connect(loadOrthomosaicPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::loadOrthomosaic);
    connect(pbEditStart, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::editStartClicked);
    connect(pbEditPause, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::editPauseClicked);
    connect(loadTilesPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::loadSelectedTiles);
    connect(unloadTilesPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::unloadTiles);
    connect(unloadTilesAllPushButton, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::unloadTilesAll);
    connect(rbEditPolygon, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::editPolygonClicked);
    connect(rbEditRectangle, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::editRectangleClicked);
    connect(modelDbsComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(onModelDbsComboBoxCurrentIndexChanged(int)));
    connect(addModelDbPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::addModelDb);
    connect(openModelDbPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::openModelDb);
    connect(closeModelDbPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::closeModelDb);
    connect(removeModelDbPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::removeModelDb);
    connect(modelDbObjectComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(onModelDbObjectComboBoxCurrentIndexChanged(int)));
    connect(modelDbObjectCandidateComboBox,SIGNAL(currentIndexChanged(int)),
            this,SLOT(onModelDbObjectCandidateComboBoxCurrentIndexChanged(int)));
    connect(processModelDbObjectPushButton,&QPushButton::clicked, this, &ccPointCloudTilesFileDlg::processModelDbObject);
    connect(rbPointCloud, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::pointCloudClicked);
    connect(rbModelDb, &QPushButton::clicked, this, &ccPointCloudTilesFileDlg::modelDbClicked);
    connect(processModelDbObjectComboBox, SIGNAL(currentIndexChanged(int)),
            this,SLOT(onProcessModelDbObjectComboBoxCurrentIndexChanged(int)));
    connect(showObjectNamesCheckBox, SIGNAL(stateChanged(int)), this,SLOT(showObjectNames(int)));

    toolBox->setCurrentIndex(0);
    connect(toolBox, SIGNAL(currentChanged(int)),
            this,SLOT(onToolBoxCurrentIndexChanged(int)));
    toolBox->setItemEnabled(1,false);

    addProjectPushButton->setEnabled(true);
    openProjectPushButton->setEnabled(false);
    closeProjectPushButton->setEnabled(false);
    removeProjectPushButton->setEnabled(false);
    projectManagementTabWidget->setTabEnabled(0,true);
    projectManagementTabWidget->setTabEnabled(1,false);
    projectManagementTabWidget->setTabEnabled(2,false);
    projectManagementTabWidget->setCurrentIndex(0);
    resetUI();
    rbEditRectangle->setChecked(true);
    m_selectionTilesPolyVertices = new ccPointCloud("vertices", static_cast<unsigned>(ReservedIDs::INTERACTIVE_SEGMENTATION_TOOL_POLYLINE_VERTICES));
    m_selectionTilesPoly = new ccPolyline(m_selectionTilesPolyVertices, static_cast<unsigned>(ReservedIDs::INTERACTIVE_SEGMENTATION_TOOL_POLYLINE));
    m_selectionTilesPoly->setForeground(true);
    m_selectionTilesPoly->setColor(ccColor::green);
    m_selectionTilesPoly->showColors(true);
    m_selectionTilesPoly->set2DMode(true);
    addModelDbPushButton->setEnabled(true);
    openModelDbPushButton->setEnabled(false);
    closeModelDbPushButton->setEnabled(false);
    removeModelDbPushButton->setEnabled(false);
    mVisibleModelClassesComboCheckBox->setEnabled(false);
    mVisibleModelObjectsComboCheckBox->setEnabled(false);
    visibleModelObjectsNonePushButton->setEnabled(false);
    visibleModelObjectsAllPushButton->setEnabled(false);
    modelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    modelDbObjectComboBox->setEnabled(false);
    modelDbObjectCandidateComboBox->setEnabled(false);
    processModelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_NO_COMBO_SELECT);
    processModelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_ENABLE);
    processModelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_DISABLE);
    processModelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_REMOVE);
    processModelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_CHECK);
    processModelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_UNCHECK);
    processModelDbObjectComboBox->addItem(POINT_CLOUD_TILES_FILE_DIALOG_PROCESS_MODEL_DB_OBJECT_SET_CANDIDATE);
//    processModelDbObjectComboBox->addItem();
//    processModelDbObjectComboBox->addItem();
    processModelDbObjectComboBox->setEnabled(false);
    processModelDbObjectPushButton->setEnabled(false);
    modelDbGroupBox->setEnabled(false);
    unloadTilesAllPushButton->setEnabled(false);
    showObjectNamesCheckBox->setEnabled(false);
    showObjectNamesCheckBox->setChecked(false);

    mIsInitialized=true;
}

void ccPointCloudTilesFileDlg::projectCloud(const ccGLCameraParameters &camera)
{
    /*
//    // check camera parameters changes
//	bool hasChanges = false;
//	auto a = m_cameraParameters.modelViewMat.data();
//	auto b = camera.modelViewMat.data();
//	for (int i = 0; i < OPENGL_MATRIX_SIZE; ++i)
//	{
//		if (std::abs(a[i] - b[i]) > 1e-6)
//		{
//			hasChanges = true;
//			break;
//		}
//	}
//	if (!hasChanges)
//		return;

    m_cameraParameters = camera;
    ccPointCloud* cloud = nullptr;
//    if (mPtrCcTilesCenterPointsContainer->isA(CC_TYPES::POINT_CLOUD))
//    {
//        cloud = static_cast<ccPointCloud*>(mPtrCcTilesCenterPointsContainer);
//    }
//    else
//    {
//        if (mPtrCcTilesCenterPointsContainer->getChild(0)->isA(CC_TYPES::POINT_CLOUD))
//        {
//            cloud = static_cast<ccPointCloud*>(mPtrCcTilesCenterPointsContainer->getChild(0));
//        }
//    }
    cloud = static_cast<ccPointCloud*>(mPtrCcTilesCenterPointsContainer->getChild(0));
    if(!cloud)
    {
        int yo =1;
        yo++;
    }
    unsigned cloudSize = cloud->size();
    const double half_w = camera.viewport[2] / 2.0;
    const double half_h = camera.viewport[3] / 2.0;
    CCVector3d Q2D;
    std::vector<CCVector2> projectedPoints;
    std::vector<bool> pointInFrustum;
    bool pointInFrustum = false;
    for (unsigned i = start; i < end; ++i)
    {
        const CCVector3* P3D = m_ptrCloud->getPoint(i);
        camera.project(*P3D, Q2D, &pointInFrustum);
        projectedPoints[i] = CCVector2(static_cast<PointCoordinateType>(Q2D.x - half_w), static_cast<PointCoordinateType>(Q2D.y - half_h));
        pointInFrustum[i] = pointInFrustum;
    }

    unsigned processorCount = std::thread::hardware_concurrency();
    if (processorCount == 0)
        processorCount = 1;

    const size_t part_size = cloudSize / processorCount;
    std::vector<std::thread*> threads;
    threads.resize(processorCount, nullptr);
    for (unsigned i = 0; i < processorCount; ++i)
    {
        size_t start = i * part_size;
        size_t end = start + part_size;

        if (i == processorCount - 1)
            end = cloudSize;

        threads[i] = new std::thread(&ccToolsClassificationHelper::project, this, camera, start, end);
    }

    for (auto it = threads.begin(); it != threads.end(); ++it)
        (*it)->join();

    for (auto it = threads.begin(); it != threads.end(); ++it)
        delete (*it);
        */
}

void ccPointCloudTilesFileDlg::reject()
{
//    if (m_ptrHelper && m_ptrHelper->hasChanges())
//	{
//		if (QMessageBox::question(m_associatedWin->asWidget(), "Cloud layers plugin", "The cloud has been modified, are you sure you want exit?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
//		{
//			return;
//		}
//	}
    stop(false);
}

void ccPointCloudTilesFileDlg::resetUI()
{
    if(mPtrCcTilesCenterPointsContainer)
    {
        pbEditStart->setEnabled(true);
    }
    else
    {
        pbEditStart->setEnabled(false);
    }
    pbEditPause->setEnabled(false);
    loadTilesPushButton->setEnabled(false);
    unloadTilesPushButton->setEnabled(false);
//    unloadTilesAllPushButton->setEnabled(false);
}

bool ccPointCloudTilesFileDlg::start()
{
    resetUI();
    mPtrApp->freezeUI(true);
//    if(!mSelectionTilesPolyIsAdded)
//    {
//        m_associatedWin->addToOwnDB(m_selectionTilesPoly);
//        mSelectionTilesPolyIsAdded=true;
//    }
    m_associatedWin->addToOwnDB(m_selectionTilesPoly);
    mSelectionTilesPolyIsAdded=true;
    return ccOverlayDialog::start();
}

void ccPointCloudTilesFileDlg::stop(bool accepted)
{
    if (mPtrApp)
	{
        mPtrApp->freezeUI(false);
	}
//    if(mSelectionTilesPolyIsAdded)
//    {
//        m_associatedWin->removeFromOwnDB(m_selectionTilesPoly);
//    }
    if(mSelectionTilesPolyIsAdded)
    {
        m_associatedWin->removeFromOwnDB(m_selectionTilesPoly);
        mSelectionTilesPolyIsAdded=false;
    }
    ccOverlayDialog::stop(accepted);
}
