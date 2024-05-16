#include "../include/ccTidopToolsManualClassificationDlg.h"

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

ccTidopToolsManualClassificationDlg::ccTidopToolsManualClassificationDlg(QString classificationModelName,
                                                                         ccMainAppInterface* app,
                                                                         QWidget* parent)
	: ccOverlayDialog(parent)
	, Ui::ccTidopToolsManualClassificationDlg()
    , m_ptrApp(app)
    , m_ptrHelper(nullptr)
    , m_ptrMouseCircle(nullptr)
    , m_ptrClassificationModel(nullptr)
{
	setupUi(this);

    setWindowTitle(QString(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE));

	// allow resize and move window
	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint/* | Qt::WindowTitleHint*/);

    m_ptrClassificationModel = new ccClassificationModel(classificationModelName);
	// set model to tableView
	initTableView();

	// connect buttons
	connect(pbAdd, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::addClicked);
	connect(pbDelete, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::deleteClicked);
	connect(pbStart, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::startClicked);
	connect(pbPause, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::pauseClicked);
	connect(pbApply, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::applyClicked);
	connect(pbClose, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::closeClicked);
    connect(pbSelectedPointsProcess, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::selectedPointsProcess);
    connect(pbSetAllVisible, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::setAllVisible);
    connect(pbSetAllInvisible, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::setAllInvisible);
    connect(pbSetAllLocked, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::setAllLocked);
    connect(pbSetAllUnlocked, &QPushButton::clicked, this, &ccTidopToolsManualClassificationDlg::setAllUnlocked);

	// connect comboboxes
	connect(cbScalarField, qOverload<int>(&QComboBox::currentIndexChanged), this, &ccTidopToolsManualClassificationDlg::scalarFieldIndexChanged);
    connect(cbSelectedPointsTools, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccTidopToolsManualClassificationDlg::selectedPointsToolsChanged);
    connect(cbSelectedPointsScalarValues, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccTidopToolsManualClassificationDlg::selectedPointsScalarValuesChanged);

	// color picker
	connect(tableView, &QTableView::doubleClicked, this, &ccTidopToolsManualClassificationDlg::tableViewDoubleClicked);
    connect(tableView, &QTableView::clicked, this, &ccTidopToolsManualClassificationDlg::tableViewClicked);

	// asprs model changed signals
    connect(m_ptrClassificationModel, &ccClassificationModel::codeChanged, this, &ccTidopToolsManualClassificationDlg::codeChanged);
    connect(m_ptrClassificationModel, &ccClassificationModel::colorChanged, this, &ccTidopToolsManualClassificationDlg::colorChanged);

    m_ptrMouseCircle = new ccMouseCircle(m_ptrApp, m_ptrApp->getActiveGLWindow());
    m_ptrMouseCircle->setVisible(false);
    rbSelect->setChecked(true);
}

ccTidopToolsManualClassificationDlg::~ccTidopToolsManualClassificationDlg()
{
	setPointCloud(nullptr);

    if (m_ptrMouseCircle)
	{
        delete m_ptrMouseCircle;
        m_ptrMouseCircle = nullptr;
	}
}

void ccTidopToolsManualClassificationDlg::reject()
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

bool ccTidopToolsManualClassificationDlg::start()
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

	connect(m_associatedWin, &ccGLWindow::mouseMoved, this, &ccTidopToolsManualClassificationDlg::mouseMoved);

	return ccOverlayDialog::start();
}

void ccTidopToolsManualClassificationDlg::stop(bool accepted)
{
    if (m_ptrMouseCircle && m_ptrMouseCircle->isVisible())
		pauseClicked();

    if (accepted && m_ptrHelper)
	{
        m_ptrHelper->keepCurrentSFVisible();
	}

	setPointCloud(nullptr);

    if (m_ptrApp)
	{
        m_ptrApp->freezeUI(false);
	}

	ccOverlayDialog::stop(accepted);
}

void ccTidopToolsManualClassificationDlg::setPointCloud(ccPointCloud* cloud)
{
    if (m_ptrHelper)
	{
        delete m_ptrHelper;
        m_ptrHelper = nullptr;
	}

	cbScalarField->clear();

	if (cloud)
	{
        m_ptrHelper = new ccTidopToolsClassificationHelper(m_ptrApp, cloud);

        cbScalarField->addItems(m_ptrHelper->getScalarFields());
	}
}

void ccTidopToolsManualClassificationDlg::resetUI()
{
	pbStart->setEnabled(true);
	pbPause->setEnabled(false);
}

void ccTidopToolsManualClassificationDlg::initTableView()
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
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::COUNT, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::LOCKED, QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSectionResizeMode(ccClassificationModel::TRAIN, QHeaderView::ResizeToContents);
    tableView->setColumnHidden(m_ptrClassificationModel->getTrainColumn(),true);
}

void ccTidopToolsManualClassificationDlg::saveSettings()
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

void ccTidopToolsManualClassificationDlg::loadSettings()
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

void ccTidopToolsManualClassificationDlg::addClicked()
{
    QModelIndex index = m_ptrClassificationModel->createNewItem();
	tableView->selectRow(index.row());
	tableView->setCurrentIndex(index);

    uptdateSelectedPointsTools();
}

void ccTidopToolsManualClassificationDlg::deleteClicked()
{
	QItemSelectionModel* select = tableView->selectionModel();
	if (!select->hasSelection())
		return;

    if (QMessageBox::question(m_associatedWin->asWidget(),
                              TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
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
                                      TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
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

void ccTidopToolsManualClassificationDlg::startClicked()
{
    if (nullptr == m_ptrApp->getActiveGLWindow())
	{
		return;
	}

    m_ptrApp->getActiveGLWindow()->setPickingMode(ccGLWindow::PICKING_MODE::NO_PICKING);

	//set orthographic view (as this tool doesn't work in perspective mode)
    m_ptrApp->getActiveGLWindow()->setPerspectiveState(false, true);
    m_ptrApp->getActiveGLWindow()->setInteractionMode(ccGLWindow::INTERACT_SEND_ALL_SIGNALS);
    m_ptrMouseCircle->setVisible(true);

	pbStart->setEnabled(false);
	pbPause->setEnabled(true);
}

void ccTidopToolsManualClassificationDlg::pauseClicked()
{
    if (nullptr == m_ptrApp->getActiveGLWindow())
	{
		return;
	}

    m_ptrMouseCircle->setVisible(false);
    m_ptrApp->getActiveGLWindow()->setPickingMode(ccGLWindow::PICKING_MODE::DEFAULT_PICKING);
    m_ptrApp->getActiveGLWindow()->setInteractionMode(ccGLWindow::MODE_TRANSFORM_CAMERA);
    m_ptrApp->getActiveGLWindow()->redraw(true, false);

	pbStart->setEnabled(true);
	pbPause->setEnabled(false);
}

void ccTidopToolsManualClassificationDlg::applyClicked()
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

void ccTidopToolsManualClassificationDlg::closeClicked()
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

void ccTidopToolsManualClassificationDlg::mouseMoved(int x, int y, Qt::MouseButtons buttons)
{
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

    ccTidopToolsClassificationHelper::Parameters& params = m_ptrHelper->getParameters();
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

bool ccTidopToolsManualClassificationDlg::eventFilter(QObject* obj, QEvent* event)
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

void ccTidopToolsManualClassificationDlg::scalarFieldIndexChanged(int index)
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

void ccTidopToolsManualClassificationDlg::selectedPointsToolsChanged(int index)
{
    cbSelectedPointsScalarValues->clear();
    cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    QString tool=cbSelectedPointsTools->currentText();

    QMap<int,QString> dataTags = m_ptrClassificationModel->getDataTags();
    if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->setEnabled(false);
        pbSelectedPointsProcess->setEnabled(false);
        return;
    }
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS,
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
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
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

void ccTidopToolsManualClassificationDlg::selectedPointsScalarValuesChanged(int index)
{
    QString targetClassTag=cbSelectedPointsScalarValues->currentText();
    if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        pbSelectedPointsProcess->setEnabled(false);
        return;
    }
    else
    {
        pbSelectedPointsProcess->setEnabled(true);
    }
}

void ccTidopToolsManualClassificationDlg::codeChanged(ccClassificationModel::Item& item, int oldCode)
{
    if (m_ptrHelper)
	{
        m_ptrHelper->changeCode(item, static_cast<ScalarType>(oldCode));
	}
}

void ccTidopToolsManualClassificationDlg::colorChanged(ccClassificationModel::Item& item)
{
    if (!m_ptrHelper)
	{
		return;
	}

    item.count = m_ptrHelper->apply(item, true);

	// refresh point count
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsManualClassificationDlg::setAllVisible()
{
    m_ptrClassificationModel->setAllVisible();
    m_ptrHelper->apply(m_ptrClassificationModel->getData());
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsManualClassificationDlg::setAllInvisible()
{
    m_ptrClassificationModel->setAllInvisible();
    m_ptrHelper->apply(m_ptrClassificationModel->getData());
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsManualClassificationDlg::setAllLocked()
{
    m_ptrClassificationModel->setAllLocked();
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsManualClassificationDlg::setAllUnlocked()
{
    m_ptrClassificationModel->setAllUnlocked();
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsManualClassificationDlg::selectedPointsProcess()
{
    ccClassificationModel::Item* selectedItem = m_ptrClassificationModel->getSelectedItem();
    ccClassificationModel::Item* removedItem = m_ptrClassificationModel->getRemovedItem();
    if (!m_ptrHelper)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                  "Invalid helper");
        return;
    }
    QString tool=cbSelectedPointsTools->currentText();
    if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                  "Select a tool");
        return;
    }
    QString targetClassTag=cbSelectedPointsScalarValues->currentText();
    if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                  "Select target class");
        return;
    }
    bool allClasses=false;
    bool allVisibleClasses=false;
    ccTidopToolsClassificationHelper::Parameters& params = m_ptrHelper->getParameters();
    params.anyPoints = false;
    params.visiblePoints = false;
    params.ptrInput = nullptr;
    params.ptrOutput = nullptr;
    std::map<ScalarType, int> affected;
    if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS,
                         Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0
                ||targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS,
                         Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        m_ptrHelper->toolRecoverClass(affected,m_ptrClassificationModel);
    }
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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
    else if(tool.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER,
                    Qt::CaseInsensitive)==0)
    {
        if(removedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_TITLE,
                                      "No points removed");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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

void ccTidopToolsManualClassificationDlg::tableViewDoubleClicked(const QModelIndex& index)
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

void ccTidopToolsManualClassificationDlg::tableViewClicked(const QModelIndex &index)
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
void ccTidopToolsManualClassificationDlg::uptdateSelectedPointsTools()
{
    auto data = m_ptrClassificationModel->getData();

    cbSelectedPointsTools->clear();
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER);
//    cbSelectedPointsTools->addItem();

    cbSelectedPointsScalarValues->clear();
    cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_MANUAL_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbSelectedPointsScalarValues->setEnabled(false);
    pbSelectedPointsProcess->setEnabled(false);
}
