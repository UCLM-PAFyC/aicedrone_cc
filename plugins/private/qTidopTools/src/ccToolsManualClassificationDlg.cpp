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

#include "../include/ccToolsManualClassificationDlg.h"

#include "../include/ccColorStyledDelegate.h"
#include "../include/ccMouseCircle.h"

//QT
#include <QColorDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <QMap>

//CC
#include <ccGLWindow.h>
#include <ccPointCloud.h>
#include <ccPolyline.h>

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

ccToolsManualClassificationDlg::ccToolsManualClassificationDlg(QString classificationModelName,
                                                                         ccMainAppInterface* app,
                                                                         QWidget* parent)
	: ccOverlayDialog(parent)
	, Ui::ccToolsManualClassificationDlg()
    , m_segmentationPolyIsAdded(false)
    , m_ptrApp(app)
    , m_ptrHelper(nullptr)
    , m_ptrMouseCircle(nullptr)
    , m_pointsEditionState(0)
    , m_segmentationPoly(nullptr)
    , m_polyVertices(nullptr)
    , m_ptrClassificationModel(nullptr)
{
	setupUi(this);

    setWindowTitle(QString(TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE));

	// allow resize and move window
	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint/* | Qt::WindowTitleHint*/);

    m_ptrClassificationModel = new ccClassificationModel(classificationModelName);
	// set model to tableView
	initTableView();

	// connect buttons
	connect(pbAdd, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::addClicked);
	connect(pbDelete, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::deleteClicked);
    connect(pbEditStart, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::editStartClicked);
    connect(pbEditPause, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::editPauseClicked);
	connect(pbApply, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::applyClicked);
	connect(pbClose, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::closeClicked);
    connect(pbSelectedPointsProcess, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::selectedPointsProcess);
    connect(pbSetAllVisible, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::setAllVisible);
    connect(pbSetAllInvisible, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::setAllInvisible);
    connect(pbSetAllLocked, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::setAllLocked);
    connect(pbSetAllUnlocked, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::setAllUnlocked);
    connect(pbEditApply, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::editApplyClicked);
    connect(rbEditCircle, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::editCircleClicked);
    connect(rbEditPolygon, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::editPolygonClicked);
    connect(rbEditRectangle, &QPushButton::clicked, this, &ccToolsManualClassificationDlg::editRectangleClicked);

	// connect comboboxes
	connect(cbScalarField, qOverload<int>(&QComboBox::currentIndexChanged), this, &ccToolsManualClassificationDlg::scalarFieldIndexChanged);
    connect(cbSelectedPointsTools, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccToolsManualClassificationDlg::selectedPointsToolsChanged);
    connect(cbSelectedPointsScalarValues, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccToolsManualClassificationDlg::selectedPointsScalarValuesChanged);

	// color picker
	connect(tableView, &QTableView::doubleClicked, this, &ccToolsManualClassificationDlg::tableViewDoubleClicked);
    connect(tableView, &QTableView::clicked, this, &ccToolsManualClassificationDlg::tableViewClicked);

	// asprs model changed signals
    connect(m_ptrClassificationModel, &ccClassificationModel::codeChanged, this, &ccToolsManualClassificationDlg::codeChanged);
    connect(m_ptrClassificationModel, &ccClassificationModel::colorChanged, this, &ccToolsManualClassificationDlg::colorChanged);

    m_ptrMouseCircle = new ccMouseCircle(m_ptrApp, m_ptrApp->getActiveGLWindow());
    m_ptrMouseCircle->setVisible(false);
    rbSelect->setChecked(true);
    rbEditCircle->setChecked(true);

    m_polyVertices = new ccPointCloud("vertices", static_cast<unsigned>(ReservedIDs::INTERACTIVE_SEGMENTATION_TOOL_POLYLINE_VERTICES));
    m_segmentationPoly = new ccPolyline(m_polyVertices, static_cast<unsigned>(ReservedIDs::INTERACTIVE_SEGMENTATION_TOOL_POLYLINE));
    m_segmentationPoly->setForeground(true);
    m_segmentationPoly->setColor(ccColor::green);
    m_segmentationPoly->showColors(true);
    m_segmentationPoly->set2DMode(true);
}

ccToolsManualClassificationDlg::~ccToolsManualClassificationDlg()
{
	setPointCloud(nullptr);
    if (m_ptrMouseCircle)
        delete m_ptrMouseCircle;
    m_ptrMouseCircle = nullptr;
    if (m_segmentationPoly)
        delete m_segmentationPoly;
    m_segmentationPoly = nullptr;
    if (m_polyVertices)
        delete m_polyVertices;
    m_polyVertices = nullptr;
}

void ccToolsManualClassificationDlg::reject()
{
    if (m_ptrHelper && m_ptrHelper->hasChanges())
	{
		if (QMessageBox::question(m_associatedWin->asWidget(), "Cloud layers plugin", "The cloud has been modified, are you sure you want exit?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		{
			return;
		}
	}

	stop(false);
}

bool ccToolsManualClassificationDlg::start()
{
    if (!m_ptrHelper)
	{
		return false;
	}

    m_ptrClassificationModel->load();
    uptdateSelectedPointsTools();
	loadSettings();

	resetUI();
    m_ptrApp->freezeUI(true);

//    m_segmentationPoly->clear();
//    m_polyVertices->clear();
//	allowPolylineExport(false);

    //the user must not close this window!
//	m_associatedWin->setUnclosable(true);
    m_associatedWin->addToOwnDB(m_segmentationPoly);

//	connect(m_associatedWin, &ccGLWindow::mouseMoved, this, &ccTidopToolsManualClassificationDlg::mouseMoved);

    m_segmentationPolyIsAdded=true;
    return ccOverlayDialog::start();
}

void ccToolsManualClassificationDlg::stop(bool accepted)
{
    if (m_ptrMouseCircle && m_ptrMouseCircle->isVisible())
        editPauseClicked();

    if (accepted && m_ptrHelper)
	{
        m_ptrHelper->keepCurrentSFVisible();
	}

	setPointCloud(nullptr);

    if (m_ptrApp)
	{
        m_ptrApp->freezeUI(false);
	}

//    m_associatedWin->removeFromOwnDB(m_segmentationPoly);
    if(m_segmentationPolyIsAdded)
    {
        m_associatedWin->removeFromOwnDB(m_segmentationPoly);
        m_segmentationPolyIsAdded=false;
    }
    ccOverlayDialog::stop(accepted);
}

void ccToolsManualClassificationDlg::setPointCloud(ccPointCloud* cloud)
{
    if (m_ptrHelper)
	{
        delete m_ptrHelper;
        m_ptrHelper = nullptr;
	}

	cbScalarField->clear();

	if (cloud)
	{
        m_ptrHelper = new ccToolsClassificationHelper(m_ptrApp, cloud);

        cbScalarField->addItems(m_ptrHelper->getScalarFields());
	}
}

void ccToolsManualClassificationDlg::resetUI()
{
    pbEditStart->setEnabled(true);
    pbEditPause->setEnabled(false);
    pbEditApply->setEnabled(false);
}

void ccToolsManualClassificationDlg::initTableView()
{
	QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_ptrClassificationModel);
	tableView->setModel(proxyModel);
	tableView->setSortingEnabled(true);
    tableView->sortByColumn(ccClassificationModel::CODE, Qt::AscendingOrder);

	// set column delegates
    tableView->setItemDelegateForColumn(ccClassificationModel::COLOR, new ccColorStyledDelegate(this));

    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::NAME, QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::VISIBLE, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::CODE, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::COLOR, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::RGB, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::COUNT, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::LOCKED, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::TRAIN, QHeaderView::ResizeToContents);
    tableView->setColumnHidden(m_ptrClassificationModel->getTrainColumn(),true);
}

void ccToolsManualClassificationDlg::saveSettings()
{
	QSettings settings;
    settings.beginGroup("qTidopTools");
	{
		if (cbScalarField->currentIndex() >= 0)
			settings.setValue("ScalarField", cbScalarField->currentText());

		settings.beginGroup("Window");
		{
			settings.setValue("geometry", saveGeometry());
		}
		settings.sync();
	}
	settings.endGroup();
}

void ccToolsManualClassificationDlg::loadSettings()
{
    if (!m_ptrHelper)
	{
		return;
	}

	QSettings settings;
    settings.beginGroup("qTidopTools");
	{
		QString sfName = settings.value("ScalarField").toString();

        int sfIndex = m_ptrHelper->getScalarFields().indexOf(sfName);
		if (sfIndex < 0)
		{
			// previous scalar field not found
			sfName = "Classification";
            sfIndex = m_ptrHelper->getScalarFields().indexOf(sfName);
			if (sfIndex < 0)
			{
				// we'll take the first one
				sfIndex = 0;
			}
		}

		cbScalarField->setCurrentIndex(sfIndex);
        if(m_ptrHelper->getScalarFields().size()==1)
        {
            scalarFieldIndexChanged(0);
//            tableView->update();
        }

		settings.beginGroup("Window");
		{
			restoreGeometry(settings.value("geometry").toByteArray());
		}
		settings.endGroup();
	}
	settings.endGroup();
}

void ccToolsManualClassificationDlg::addClicked()
{
    QModelIndex index = m_ptrClassificationModel->createNewItem();
	tableView->selectRow(index.row());
	tableView->setCurrentIndex(index);

    uptdateSelectedPointsTools();
}

void ccToolsManualClassificationDlg::deleteClicked()
{
	QItemSelectionModel* select = tableView->selectionModel();
	if (!select->hasSelection())
		return;

    if (QMessageBox::question(m_associatedWin->asWidget(),
                              TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                              "Are you sure you want to delete this record(s)?",
		QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
	{
		return;
	}

	QModelIndexList mapIndices = select->selectedIndexes();
    // DHL
    int cont=0;
    for (QModelIndex index : mapIndices)
    {
        QModelIndex sourceIndex = static_cast<QSortFilterProxyModel*>(tableView->model())->mapToSource(index);
        ccClassificationModel::Item& item = m_ptrClassificationModel->getData()[sourceIndex.row()];
        QString itemName=item.name;
        if(itemName.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,Qt::CaseInsensitive)==0
                ||itemName.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,Qt::CaseInsensitive)==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "Selected and removed items cant be deleted");
            mapIndices.removeAt(cont);
        }
        else cont++;
    }
	qSort(mapIndices);

	QModelIndexList sourceIndices;
	for (QModelIndex index : mapIndices)
	{
		QModelIndex sourceIndex = static_cast<QSortFilterProxyModel*>(tableView->model())->mapToSource(index);
		sourceIndices.append(sourceIndex);
	}

    ccClassificationModel::Item* to = m_ptrClassificationModel->getData().size() > 0 ? &(m_ptrClassificationModel->getData().front()) : nullptr;
	for (int i = mapIndices.size(); i > 0; --i)
	{
        ccClassificationModel::Item& from = m_ptrClassificationModel->getData()[sourceIndices[i - 1].row()];
        int affected = m_ptrHelper ? m_ptrHelper->moveItem(from, to) : 0;
		if (to)
		{
			to->count += affected;
		}

		tableView->model()->removeRows(mapIndices[i - 1].row(), 1);
	}

    uptdateSelectedPointsTools();

	m_associatedWin->redraw();
}

void ccToolsManualClassificationDlg::editStartClicked()
{
    if (nullptr == m_ptrApp->getActiveGLWindow())
	{
		return;
	}

    m_ptrApp->getActiveGLWindow()->setPickingMode(ccGLWindow::PICKING_MODE::NO_PICKING);

	//set orthographic view (as this tool doesn't work in perspective mode)
    m_ptrApp->getActiveGLWindow()->setPerspectiveState(false, true);
    m_ptrApp->getActiveGLWindow()->setInteractionMode(ccGLWindow::INTERACT_SEND_ALL_SIGNALS);
    if(rbEditCircle->isChecked())
    {
        m_ptrMouseCircle->setVisible(true);
        pbEditApply->setEnabled(false);
        connect(m_associatedWin, &ccGLWindow::mouseMoved, this, &ccToolsManualClassificationDlg::mouseMoved);
    }
    else if(rbEditPolygon->isChecked()||rbEditRectangle->isChecked())
    {
        connect(m_associatedWin, &ccGLWindow::leftButtonClicked,	this, &ccToolsManualClassificationDlg::addPointToPolyline);
        connect(m_associatedWin, &ccGLWindow::rightButtonClicked,	this, &ccToolsManualClassificationDlg::closePolyLine);
        connect(m_associatedWin, &ccGLWindow::mouseMoved,			this, &ccToolsManualClassificationDlg::updatePolyLine);
        connect(m_associatedWin, &ccGLWindow::buttonReleased,		this, &ccToolsManualClassificationDlg::closeRectangle);
        if (m_segmentationPoly)
        {
            m_segmentationPoly->setDisplay(m_associatedWin);
        }
        pbEditApply->setEnabled(true);
        m_pointsEditionState = STARTED;
    }

    pbEditStart->setEnabled(false);
    pbEditPause->setEnabled(true);
    rbEditCircle->setEnabled(false);
    rbEditPolygon->setEnabled(false);
    rbEditRectangle->setEnabled(false);
    rbSelect->setEnabled(false);
    rbUnselect->setEnabled(false);
    rbRecover->setEnabled(false);
    rbRemove->setEnabled(false);
}

void ccToolsManualClassificationDlg::editPauseClicked()
{
    if (nullptr == m_ptrApp->getActiveGLWindow())
	{
		return;
	}

    if(rbEditCircle->isChecked())
    {
        m_ptrMouseCircle->setVisible(false);
        pbEditApply->setEnabled(false);
        disconnect(m_associatedWin, &ccGLWindow::mouseMoved, this, &ccToolsManualClassificationDlg::mouseMoved);
    }
    else if(rbEditPolygon->isChecked()||rbEditRectangle->isChecked())
    {
        stopRunning();
        m_pointsEditionState = PAUSED;
        if (m_polyVertices->size() != 0)
        {
            m_segmentationPoly->clear();
            m_polyVertices->clear();
//			allowPolylineExport(false);
        }
        disconnect(m_associatedWin, &ccGLWindow::leftButtonClicked,	this, &ccToolsManualClassificationDlg::addPointToPolyline);
        disconnect(m_associatedWin, &ccGLWindow::rightButtonClicked,	this, &ccToolsManualClassificationDlg::closePolyLine);
        disconnect(m_associatedWin, &ccGLWindow::mouseMoved,			this, &ccToolsManualClassificationDlg::updatePolyLine);
        disconnect(m_associatedWin, &ccGLWindow::buttonReleased,		this, &ccToolsManualClassificationDlg::closeRectangle);
        if (m_segmentationPoly)
        {
            m_segmentationPoly->setDisplay(nullptr);
        }
        pbEditApply->setEnabled(false);
    }
//    else if(rbEditRectangle->isChecked())
//    {
//        pbEditApply->setEnabled(false);
//    }

    m_ptrApp->getActiveGLWindow()->setPickingMode(ccGLWindow::PICKING_MODE::DEFAULT_PICKING);
    m_ptrApp->getActiveGLWindow()->setInteractionMode(ccGLWindow::MODE_TRANSFORM_CAMERA);
    m_ptrApp->getActiveGLWindow()->redraw(true, false);

    pbEditStart->setEnabled(true);
    pbEditPause->setEnabled(false);
    rbEditCircle->setEnabled(true);
    rbEditPolygon->setEnabled(true);
    rbEditRectangle->setEnabled(true);
    rbSelect->setEnabled(true);
    rbUnselect->setEnabled(true);
    rbRecover->setEnabled(true);
    rbRemove->setEnabled(true);
}

void ccToolsManualClassificationDlg::applyClicked()
{
    m_ptrHelper->removePoints(m_ptrClassificationModel);
    m_ptrClassificationModel->save();
	
    saveSettings();
	
    if (m_ptrHelper)
    {
        m_ptrHelper->setVisible(true);
    }

    stop(true);
}

void ccToolsManualClassificationDlg::editApplyClicked()
{
    if(rbEditCircle->isChecked())
    {
        return;
    }
    if (!m_ptrHelper)
    {
        return;
    }

    ccGLCameraParameters camera;
    m_associatedWin->getGLCameraParameters(camera);

    m_ptrHelper->projectCloud(camera);

    std::map<ScalarType, int> affected;

    ccToolsClassificationHelper::Parameters& params = m_ptrHelper->getParameters();
    params.anyPoints = false;
    params.visiblePoints = false;
    params.ptrInput = nullptr;
    params.ptrOutput = nullptr;
    bool select=rbSelect->isChecked();
    bool unselect=rbUnselect->isChecked();
    bool remove=rbRemove->isChecked();
    bool recover=rbRecover->isChecked();
    if(select||unselect)
    {
        ccClassificationModel::Item* selectedItem = m_ptrClassificationModel->getSelectedItem();
        if(selectedItem==nullptr)
        {
            return;
        }
        params.ptrOutput=selectedItem;
    }
    if(remove||recover)
    {
        ccClassificationModel::Item* removedItem = m_ptrClassificationModel->getRemovedItem();
        if(removedItem==nullptr)
        {
            return;
        }
        params.ptrOutput=removedItem;
    }
    m_ptrHelper->pointsInPolygon(m_segmentationPoly, affected,m_ptrClassificationModel,
                              select,unselect,remove,recover);

    // update point counts
    for (const auto& kv : affected)
    {
        auto item = m_ptrClassificationModel->find(kv.first);
        if (item)
            item->count += kv.second;
    }
    m_ptrClassificationModel->refreshData();
    editPauseClicked();
}

void ccToolsManualClassificationDlg::closeClicked()
{
    if (m_ptrHelper)
	{
        if (m_ptrHelper->hasChanges())
		{
			if (QMessageBox::question(m_associatedWin->asWidget(), "Cloud layers plugin", "The cloud has been modified, are you sure you want exit?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
			{
				return;
			}
		}

        m_ptrHelper->restoreState();
	}

    stop(true);
}

void ccToolsManualClassificationDlg::editCircleClicked()
{

}

void ccToolsManualClassificationDlg::editPolygonClicked()
{

}

void ccToolsManualClassificationDlg::editRectangleClicked()
{

}

void ccToolsManualClassificationDlg::mouseMoved(int x, int y, Qt::MouseButtons buttons)
{
    if(!rbEditCircle->isChecked())
    {
        return;
    }
    if (!m_ptrHelper)
	{
		return;
	}
	if (buttons != Qt::LeftButton)
	{
		return;
	}

	ccGLCameraParameters camera;
	m_associatedWin->getGLCameraParameters(camera);

    m_ptrHelper->projectCloud(camera);

    QPointF pos2D = m_ptrApp->getActiveGLWindow()->toCenteredGLCoordinates(x, y);
	CCVector2 center(static_cast<PointCoordinateType>(pos2D.x()), static_cast<PointCoordinateType>(pos2D.y()));

    int radius = m_ptrMouseCircle->getRadiusPx();
	std::map<ScalarType, int> affected;

    ccToolsClassificationHelper::Parameters& params = m_ptrHelper->getParameters();
    params.anyPoints = false;
    params.visiblePoints = false;
    params.ptrInput = nullptr;
    params.ptrOutput = nullptr;
    bool select=rbSelect->isChecked();
    bool unselect=rbUnselect->isChecked();
    bool remove=rbRemove->isChecked();
    bool recover=rbRecover->isChecked();
    if(select||unselect)
    {
        ccClassificationModel::Item* selectedItem = m_ptrClassificationModel->getSelectedItem();
        if(selectedItem==nullptr)
        {
            return;
        }
        params.ptrOutput=selectedItem;
    }
    if(remove||recover)
    {
        ccClassificationModel::Item* removedItem = m_ptrClassificationModel->getRemovedItem();
        if(removedItem==nullptr)
        {
            return;
        }
        params.ptrOutput=removedItem;
    }
    m_ptrHelper->mouseMove(center, static_cast<float>(radius * radius), affected,m_ptrClassificationModel,
                              select,unselect,remove,recover);

	// update point counts
	for (const auto& kv : affected)
	{
        auto item = m_ptrClassificationModel->find(kv.first);
		if (item)
			item->count += kv.second;
	}

    m_ptrClassificationModel->refreshData();
}

void ccToolsManualClassificationDlg::addPointToPolylineExt(int x, int y, bool allowClicksOutside)
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

    assert(m_polyVertices);
    assert(m_segmentationPoly);
    unsigned vertCount = m_polyVertices->size();

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
        m_polyVertices->clear();
        if (!m_polyVertices->reserve(2))
        {
            ccLog::Error("Out of memory!");
//            allowPolylineExport(false);
            return;
        }
        //we add the same point twice (the last point will be used for display only)
        m_polyVertices->addPoint(P);
        m_polyVertices->addPoint(P);
        m_segmentationPoly->clear();
        if (!m_segmentationPoly->addPointIndex(0, 2))
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
            if (!m_polyVertices->reserve(vertCount+1))
            {
                ccLog::Error("Out of memory!");
//                allowPolylineExport(false);
                return;
            }

            //we replace last point by the current one
            CCVector3 *lastP = const_cast<CCVector3 *>(m_polyVertices->getPointPersistentPtr(vertCount-1));
            *lastP = P;
            //and add a new (equivalent) one
            m_polyVertices->addPoint(P);
            if (!m_segmentationPoly->addPointIndex(vertCount))
            {
                ccLog::Error("Out of memory!");
                return;
            }
            m_segmentationPoly->setClosed(true);
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

void ccToolsManualClassificationDlg::closePolyLine(int x, int y)
{
    //only for polyline in RUNNING mode
    if ((m_pointsEditionState & POLYLINE) == 0
            || (m_pointsEditionState & RUNNING) == 0)
        return;

    if (m_associatedWin)
    {
        m_associatedWin->releaseMouse();
    }

    assert(m_segmentationPoly);
    unsigned vertCount = m_segmentationPoly->size();
    if (vertCount < 4)
    {
        m_segmentationPoly->clear();
        m_polyVertices->clear();
    }
    else
    {
        //remove last point!
        m_segmentationPoly->resize(vertCount-1); //can't fail --> smaller
        m_segmentationPoly->setClosed(true);
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

void ccToolsManualClassificationDlg::closeRectangle()
{
    //only for rectangle selection in RUNNING mode
    if ((m_pointsEditionState & RECTANGLE) == 0
            || (m_pointsEditionState & RUNNING) == 0)
        return;

    assert(m_segmentationPoly);
    unsigned vertCount = m_segmentationPoly->size();
    if (vertCount < 4)
    {
        //first point only? we keep the real time update mechanism
        if (rbEditRectangle->isChecked())
            return;
        m_segmentationPoly->clear();
        m_polyVertices->clear();
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

void ccToolsManualClassificationDlg::updatePolyLine(int x, int y, Qt::MouseButtons buttons)
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

    assert(m_polyVertices);
    assert(m_segmentationPoly);

    unsigned vertCount = m_polyVertices->size();

    //new point (expressed relatively to the screen center)
    QPointF pos2D = m_associatedWin->toCenteredGLCoordinates(x, y);
    CCVector3 P(static_cast<PointCoordinateType>(pos2D.x()),
                static_cast<PointCoordinateType>(pos2D.y()),
                0);

    if (m_pointsEditionState & RECTANGLE)
    {
        //we need 4 points for the rectangle!
        if (vertCount != 4)
            m_polyVertices->resize(4);

        const CCVector3 *A = m_polyVertices->getPointPersistentPtr(0);
        CCVector3 *B = const_cast<CCVector3 *>(m_polyVertices->getPointPersistentPtr(1));
        CCVector3 *C = const_cast<CCVector3 *>(m_polyVertices->getPointPersistentPtr(2));
        CCVector3 *D = const_cast<CCVector3 *>(m_polyVertices->getPointPersistentPtr(3));
        *B = CCVector3(A->x, P.y, 0);
        *C = P;
        *D = CCVector3(P.x, A->y, 0);

        if (vertCount != 4)
        {
            m_segmentationPoly->clear();
            if (!m_segmentationPoly->addPointIndex(0, 4))
            {
                ccLog::Error("Out of memory!");
//                allowPolylineExport(false);
                return;
            }
            m_segmentationPoly->setClosed(true);
        }
    }
    else if (m_pointsEditionState & POLYLINE)
    {
        if (vertCount < 2)
            return;
        //we replace last point by the current one
        CCVector3 *lastP = const_cast<CCVector3 *>(m_polyVertices->getPointPersistentPtr(vertCount - 1));
        *lastP = P;
    }

    m_associatedWin->redraw(true, false);
}

void ccToolsManualClassificationDlg::run()
{
    m_pointsEditionState |= RUNNING;
    //	buttonsFrame->setEnabled(false); // we disable the buttons when running
}

void ccToolsManualClassificationDlg::stopRunning()
{
    m_pointsEditionState &= (~RUNNING);
//	buttonsFrame->setEnabled(true); // we restore the buttons when running is stopped
}

bool ccToolsManualClassificationDlg::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ev = static_cast<QKeyEvent*>(event);
		if (ev->key() == Qt::Key::Key_Alt)
		{
            m_ptrMouseCircle->setAllowScroll(false);
		}
	}
	else if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ev = static_cast<QKeyEvent*>(event);
		if (ev->key() == Qt::Key::Key_Alt)
		{
            m_ptrMouseCircle->setAllowScroll(true);
		}
	}

	return false;
}

void ccToolsManualClassificationDlg::scalarFieldIndexChanged(int index)
{
    if (m_ptrHelper)
	{
        m_ptrHelper->setScalarFieldIndex(index);
        m_ptrHelper->apply(m_ptrClassificationModel->getData());
        m_ptrHelper->saveState();
	}

	// refresh point count
    m_ptrClassificationModel->refreshData();
}

void ccToolsManualClassificationDlg::selectedPointsToolsChanged(int index)
{
    cbSelectedPointsScalarValues->clear();
    cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    QString tool=cbSelectedPointsTools->currentText();

    QMap<int,QString> dataTags = m_ptrClassificationModel->getDataTags();
    if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->setEnabled(false);
        pbSelectedPointsProcess->setEnabled(false);
        return;
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS,
                    Qt::CaseInsensitive)==0)
    {
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    cbSelectedPointsScalarValues->setEnabled(true);
}

void ccToolsManualClassificationDlg::selectedPointsScalarValuesChanged(int index)
{
    QString targetClassTag=cbSelectedPointsScalarValues->currentText();
    if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        pbSelectedPointsProcess->setEnabled(false);
        return;
    }
    else
    {
        pbSelectedPointsProcess->setEnabled(true);
    }
}

void ccToolsManualClassificationDlg::codeChanged(ccClassificationModel::Item& item, int oldCode)
{
    if (m_ptrHelper)
	{
        m_ptrHelper->changeCode(item, static_cast<ScalarType>(oldCode));
	}
}

void ccToolsManualClassificationDlg::colorChanged(ccClassificationModel::Item& item)
{
    if (!m_ptrHelper)
	{
		return;
	}

    item.count = m_ptrHelper->apply(item, true);

	// refresh point count
    m_ptrClassificationModel->refreshData();
}

void ccToolsManualClassificationDlg::setAllVisible()
{
    m_ptrClassificationModel->setAllVisible();
    m_ptrHelper->apply(m_ptrClassificationModel->getData());
    m_ptrClassificationModel->refreshData();
}

void ccToolsManualClassificationDlg::setAllInvisible()
{
    m_ptrClassificationModel->setAllInvisible();
    m_ptrHelper->apply(m_ptrClassificationModel->getData());
    m_ptrClassificationModel->refreshData();
}

void ccToolsManualClassificationDlg::setAllLocked()
{
    m_ptrClassificationModel->setAllLocked();
    m_ptrClassificationModel->refreshData();
}

void ccToolsManualClassificationDlg::setAllUnlocked()
{
    m_ptrClassificationModel->setAllUnlocked();
    m_ptrClassificationModel->refreshData();
}

void ccToolsManualClassificationDlg::selectedPointsProcess()
{
    ccClassificationModel::Item* selectedItem = m_ptrClassificationModel->getSelectedItem();
    ccClassificationModel::Item* removedItem = m_ptrClassificationModel->getRemovedItem();
    if (!m_ptrHelper)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                  "Invalid helper");
        return;
    }
    QString tool=cbSelectedPointsTools->currentText();
    if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                  "Select a tool");
        return;
    }
    QString targetClassTag=cbSelectedPointsScalarValues->currentText();
    if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                  "Select target class");
        return;
    }
    bool allClasses=false;
    bool allVisibleClasses=false;
    ccToolsClassificationHelper::Parameters& params = m_ptrHelper->getParameters();
    params.anyPoints = false;
    params.visiblePoints = false;
    params.ptrInput = nullptr;
    params.ptrOutput = nullptr;
    std::map<ScalarType, int> affected;
    if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                if(item.visible) targetItemsCode.append(item.code);
            }
        }
        else
        {
            ccClassificationModel::Item* targetItem=nullptr;
            QString targetClassName;
            targetItem=m_ptrClassificationModel->findByTag(targetClassTag);
            if(targetItem==nullptr) return;
            targetItemsCode.append(targetItem->code);
        }
        m_ptrHelper->toolSelectOnly(affected,targetItemsCode,m_ptrClassificationModel);
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS,
                         Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0
                ||targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
                                         Qt::CaseInsensitive)==0)
        {
            return;
        }
        ccClassificationModel::Item* targetItem=nullptr;
        QString targetClassName;
        targetItem=m_ptrClassificationModel->findByTag(targetClassTag);
        if(targetItem==nullptr) return;
        params.ptrOutput=targetItem;
//        if(targetItem->locked) return; // not change to locked classes?
        m_ptrHelper->toolChangeClass(affected,m_ptrClassificationModel);
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS,
                         Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        m_ptrHelper->toolRecoverClass(affected,m_ptrClassificationModel);
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                if(item.visible) targetItemsCode.append(item.code);
            }
        }
        else
        {
            ccClassificationModel::Item* targetItem=nullptr;
            QString targetClassName;
            targetItem=m_ptrClassificationModel->findByTag(targetClassTag);
            if(targetItem==nullptr) return;
            targetItemsCode.append(targetItem->code);
        }
        m_ptrHelper->toolUnselect(affected,targetItemsCode,m_ptrClassificationModel);
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                if(item.visible) targetItemsCode.append(item.code);
            }
        }
        else
        {
            ccClassificationModel::Item* targetItem=nullptr;
            QString targetClassName;
            targetItem=m_ptrClassificationModel->findByTag(targetClassTag);
            if(targetItem==nullptr) return;
            targetItemsCode.append(targetItem->code);
        }
        ccClassificationModel::Item* removedItem = m_ptrClassificationModel->getRemovedItem();
        if(removedItem==nullptr)
        {
            return;
        }
        params.ptrOutput=removedItem;
        m_ptrHelper->toolRemove(affected,targetItemsCode,m_ptrClassificationModel);
    }
    else if(tool.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER,
                    Qt::CaseInsensitive)==0)
    {
        if(removedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points removed");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                if(item.visible) targetItemsCode.append(item.code);
            }
        }
        else
        {
            ccClassificationModel::Item* targetItem=nullptr;
            QString targetClassName;
            targetItem=m_ptrClassificationModel->findByTag(targetClassTag);
            if(targetItem==nullptr) return;
            targetItemsCode.append(targetItem->code);
        }
        m_ptrHelper->toolRecover(affected,targetItemsCode,m_ptrClassificationModel);
    }
    // update point counts
    for (const auto& kv : affected)
    {
        auto item = m_ptrClassificationModel->find(kv.first);
        if (item)
            item->count += kv.second;
    }
    m_ptrClassificationModel->refreshData();
}

void ccToolsManualClassificationDlg::tableViewDoubleClicked(const QModelIndex& index)
{
    /*
    if(index.column() == ccClassificationModel::VISIBLE
            || index.column() == ccClassificationModel::LOCKED)
    {
//        QString itemName=m_asprsModel.ge
//                index.model()->data(ccClassificationModel::NAME, Qt::DisplayRole).value<QString>();
//        ccClassificationModel::Item& item = m_asprsModel.getData()[index.row()];
        QModelIndex sourceIndex = static_cast<QSortFilterProxyModel*>(tableView->model())->mapToSource(index);
        ccClassificationModel::Item& item = m_asprsModel.getData()[sourceIndex.row()];
        QString itemName=item.name;
        if(itemName.compare(CC_CLASSIFICATION_MODEL_ASPRS_REMOVED_NAME,
                            Qt::CaseInsensitive)==0
                ||itemName.compare(CC_CLASSIFICATION_MODEL_ASPRS_SELECTED_NAME,
                                   Qt::CaseInsensitive)==0)
        {
            item.visible=true;
            item.locked=false;
        }
        return;
    }
    */
    if (index.column() != ccClassificationModel::COLOR)
		return;

	QColor currColor = index.model()->data(index, Qt::DisplayRole).value<QColor>();
	QColor color = QColorDialog::getColor(currColor, this, "Pick a color", QColorDialog::DontUseNativeDialog);

	if (color.isValid() && color != currColor)
        tableView->model()->setData(index, color, Qt::EditRole);
}

void ccToolsManualClassificationDlg::tableViewClicked(const QModelIndex &index)
{
    /*
    if(index.column() == ccClassificationModel::VISIBLE
            || index.column() == ccClassificationModel::LOCKED)
    {
//        QString itemName=m_asprsModel.ge
//                index.model()->data(ccClassificationModel::NAME, Qt::DisplayRole).value<QString>();
//        ccClassificationModel::Item& item = m_asprsModel.getData()[index.row()];
        QModelIndex sourceIndex = static_cast<QSortFilterProxyModel*>(tableView->model())->mapToSource(index);
        ccClassificationModel::Item& item = m_asprsModel.getData()[sourceIndex.row()];
        QString itemName=item.name;
        if(itemName.compare(CC_CLASSIFICATION_MODEL_ASPRS_REMOVED_NAME,
                            Qt::CaseInsensitive)==0
                ||itemName.compare(CC_CLASSIFICATION_MODEL_ASPRS_SELECTED_NAME,
                                   Qt::CaseInsensitive)==0)
        {
//            emit(tableView->clicked(index));
            item.visible=true;
            item.locked=false;
        }
        return;
    }
    */
}
/*
void ccTidopToolsManualClassificationDlg::updateInputOutput()
{
	auto data = m_asprsModel.getData();

	cbInput->clear();
	cbInput->addItems(m_presets);
	for (int i = 0; i < data.size(); ++i)
		cbInput->addItem(data[i].name);

	cbOutput->clear();
	for (int i = 0; i < data.size(); ++i)
        cbOutput->addItem(data[i].name);
}
*/
void ccToolsManualClassificationDlg::uptdateSelectedPointsTools()
{
    auto data = m_ptrClassificationModel->getData();

    cbSelectedPointsTools->clear();
    cbSelectedPointsTools->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbSelectedPointsTools->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS);
    cbSelectedPointsTools->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY);
    cbSelectedPointsTools->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT);
    cbSelectedPointsTools->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE);
    cbSelectedPointsTools->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS);
    cbSelectedPointsTools->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER);
//    cbSelectedPointsTools->addItem();

    cbSelectedPointsScalarValues->clear();
    cbSelectedPointsScalarValues->addItem(TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbSelectedPointsScalarValues->setEnabled(false);
    pbSelectedPointsProcess->setEnabled(false);
}
