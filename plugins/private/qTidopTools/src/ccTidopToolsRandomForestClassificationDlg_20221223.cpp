#include "../include/ccTidopToolsRandomForestClassificationDlg.h"
#include "../include/ccTidopToolsRandomForestClassificationDefinitions.h"
#include "libParameters/ParametersManager.h"
#include "libParameters/ParametersManagerDialog.h"

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
#include <QInputDialog>
#include <QLineEdit>
#include <QDir>
#include <QProgressDialog>
#include <QThread>
#include <qtconcurrentrun.h>
#include <QFuture>

//CC
#include <ccGLWindow.h>
#include <ccPointCloud.h>


ccClassificationToolCGAL* ccTidopToolsRandomForestClassificationDlg::m_ptrClassificationToolCGAL=ccClassificationToolCGAL::getInstance();
ccTidopToolsETHZRandomForest* ccTidopToolsRandomForestClassificationDlg::m_ptrTidopToolsETHZRandomForest=ccTidopToolsETHZRandomForest::getInstance();
QString ccTidopToolsRandomForestClassificationDlg::m_strError="";
QString ccTidopToolsRandomForestClassificationDlg::m_functionName="";
QString ccTidopToolsRandomForestClassificationDlg::m_strReport="";
QString ccTidopToolsRandomForestClassificationDlg::m_strTime="";
QString ccTidopToolsRandomForestClassificationDlg::m_classificationProcessCommand="";
bool ccTidopToolsRandomForestClassificationDlg::m_useCGAL=true;

ccTidopToolsRandomForestClassificationDlg::ccTidopToolsRandomForestClassificationDlg(QString classificationModelName,
                                                                                     ccMainAppInterface* app,
                                                                                     QWidget* parent)
	: ccOverlayDialog(parent)
	, Ui::ccTidopToolsRandomForestClassificationDlg()
    , m_ptrApp(app)
    , m_ptrHelper(nullptr)
    , m_ptrMouseCircle(nullptr)
    , m_ptrParametersManager(nullptr)
    , m_ptrClassificationModel(nullptr)
{
	setupUi(this);

    setWindowTitle(QString(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE));

	// allow resize and move window
	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint/* | Qt::WindowTitleHint*/);

    m_ptrClassificationModel = new ccClassificationModel(classificationModelName);
	// set model to tableView
	initTableView();

	// connect buttons
	connect(pbAdd, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::addClicked);
	connect(pbDelete, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::deleteClicked);
	connect(pbStart, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::startClicked);
	connect(pbPause, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::pauseClicked);
	connect(pbApply, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::applyClicked);
	connect(pbClose, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::closeClicked);
    connect(pbSelectedPointsProcess, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::selectedPointsProcess);
    connect(pbSetAllVisible, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::setAllVisible);
    connect(pbSetAllInvisible, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::setAllInvisible);
    connect(pbSetAllLocked, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::setAllLocked);
    connect(pbSetAllUnlocked, &QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::setAllUnlocked);
    connect(pbNewScalarField,&QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::newScalarFieldClicked);
    connect(pbDeleteScalarField,&QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::deleteScalarFieldClicked);
    connect(pbCloneScalarField,&QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::cloneScalarFieldClicked);
    connect(pbClassificationStepParameters,&QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::selectParameters);
    connect(pbClassificationStepProcess,&QPushButton::clicked, this, &ccTidopToolsRandomForestClassificationDlg::selectClassificationProcess);

    connect(ckbUseCGAL,&QCheckBox::clicked,this,&ccTidopToolsRandomForestClassificationDlg::useCGALClicked);
    ckbUseCGAL->setChecked(true);

	// connect comboboxes
    connect(cbScalarField, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccTidopToolsRandomForestClassificationDlg::scalarFieldIndexChanged);
    connect(cbSelectedPointsTools, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccTidopToolsRandomForestClassificationDlg::selectedPointsToolsChanged);
    connect(cbSelectedPointsScalarValues, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccTidopToolsRandomForestClassificationDlg::selectedPointsScalarValuesChanged);
    connect(cbClassificationTrainingScalarField, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccTidopToolsRandomForestClassificationDlg::classificationTrainingScalarFieldChanged);
    connect(cbClassificationStep, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ccTidopToolsRandomForestClassificationDlg::classificationStepChanged);

	// color picker
	connect(tableView, &QTableView::doubleClicked, this, &ccTidopToolsRandomForestClassificationDlg::tableViewDoubleClicked);
    connect(tableView, &QTableView::clicked, this, &ccTidopToolsRandomForestClassificationDlg::tableViewClicked);

	// asprs model changed signals
    connect(m_ptrClassificationModel, &ccClassificationModel::codeChanged, this, &ccTidopToolsRandomForestClassificationDlg::codeChanged);
    connect(m_ptrClassificationModel, &ccClassificationModel::colorChanged, this, &ccTidopToolsRandomForestClassificationDlg::colorChanged);

    m_ptrMouseCircle = new ccMouseCircle(m_ptrApp, m_ptrApp->getActiveGLWindow());
    m_ptrMouseCircle->setVisible(false);
    rbSelect->setChecked(true);

    cbClassificationStep->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbClassificationStep->addItem(RFC_PROCESS_COMPUTE_FEATURES);
    cbClassificationStep->addItem(RFC_PROCESS_TRAINING);
    cbClassificationStep->addItem(RFC_PROCESS_CLASSIFICATION);
    cbClassificationStep->addItem(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING);
    cbClassificationStep->addItem(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT);
    cbClassificationTrainingScalarField->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbClassificationTargetScalarField->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbClassificationTargetScalarField->setEnabled(false);
//    cbClassificationStep->setEnabled(false);
    pbClassificationStepParameters->setEnabled(false);
    pbClassificationStepProcess->setEnabled(false);

    QString strError;
    if(!initialize(strError))
    {

        if(parent==nullptr)
        {
            QMessageBox::information(new QWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      strError);
            stop(false);
        }
        QMessageBox::information(parent,
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  strError);
        close();
    }
    m_ptrProcessesDialog=new QProgressDialog(this);
    m_ptrProcessesDialog->close();
//    scalarFieldIndexChanged(0);
}

ccTidopToolsRandomForestClassificationDlg::~ccTidopToolsRandomForestClassificationDlg()
{
	setPointCloud(nullptr);

    if (m_ptrMouseCircle)
	{
        delete m_ptrMouseCircle;
        m_ptrMouseCircle = nullptr;
	}
}

void ccTidopToolsRandomForestClassificationDlg::reject()
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

void ccTidopToolsRandomForestClassificationDlg::updateCbScalarFields()
{
    if (!m_ptrHelper)
    {
        return;
    }
    cbScalarField->clear();
    cbScalarField->addItems(m_ptrHelper->getScalarFields());
    cbClassificationTrainingScalarField->clear();
    cbClassificationTrainingScalarField->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbClassificationTrainingScalarField->addItems(m_ptrHelper->getScalarFields());
    cbClassificationTargetScalarField->clear();
    cbClassificationTargetScalarField->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbClassificationTargetScalarField->addItems(m_ptrHelper->getScalarFields());
    cbClassificationTargetScalarField->setEnabled(false);
    cbClassificationStep->setCurrentIndex(0);
//    cbClassificationStep->setEnabled(false);
    pbClassificationStepParameters->setEnabled(false);
    pbClassificationStepProcess->setEnabled(false);
}

bool ccTidopToolsRandomForestClassificationDlg::start()
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

	connect(m_associatedWin, &ccGLWindow::mouseMoved, this, &ccTidopToolsRandomForestClassificationDlg::mouseMoved);

	return ccOverlayDialog::start();
}

void ccTidopToolsRandomForestClassificationDlg::stop(bool accepted)
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

void ccTidopToolsRandomForestClassificationDlg::setPointCloud(ccPointCloud* cloud)
{
    if (m_ptrHelper)
	{
        delete m_ptrHelper;
        m_ptrHelper = nullptr;
	}

	cbScalarField->clear();

//    if(m_ptrClassificationToolCGAL)
//    {
//        delete(m_ptrClassificationToolCGAL);
//    }

	if (cloud)
	{
        m_ptrHelper = new ccTidopToolsClassificationHelper(m_ptrApp, cloud);
        updateCbScalarFields();
	}
}

void ccTidopToolsRandomForestClassificationDlg::resetUI()
{
	pbStart->setEnabled(true);
	pbPause->setEnabled(false);
}

void ccTidopToolsRandomForestClassificationDlg::initTableView()
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
}

void ccTidopToolsRandomForestClassificationDlg::saveSettings()
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

void ccTidopToolsRandomForestClassificationDlg::loadSettings()
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

void ccTidopToolsRandomForestClassificationDlg::addClicked()
{
    QModelIndex index = m_ptrClassificationModel->createNewItem();
	tableView->selectRow(index.row());
	tableView->setCurrentIndex(index);

    uptdateSelectedPointsTools();
}

void ccTidopToolsRandomForestClassificationDlg::deleteClicked()
{
	QItemSelectionModel* select = tableView->selectionModel();
	if (!select->hasSelection())
		return;

    if (QMessageBox::question(m_associatedWin->asWidget(),
                              TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
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
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
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

void ccTidopToolsRandomForestClassificationDlg::startClicked()
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

void ccTidopToolsRandomForestClassificationDlg::pauseClicked()
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

void ccTidopToolsRandomForestClassificationDlg::applyClicked()
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

void ccTidopToolsRandomForestClassificationDlg::closeClicked()
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

void ccTidopToolsRandomForestClassificationDlg::newScalarFieldClicked()
{
    if (!m_ptrHelper)
    {
        return;
    }
    //ask for a name
    bool ok;
    QString defaultName;
    QString currentFieldName=cbScalarField->currentText();
    if(currentFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                Qt::CaseInsensitive)!=0)
    {
        defaultName=currentFieldName+TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_DEFAULT_NEW_FIELD_SUFFIX;
    }
    else
    {
        defaultName=TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_DEFAULT_NEW_FIELD;
    }
    QString sfName = QInputDialog::getText(this, tr("New SF name"), tr("SF name (must be unique)"), QLineEdit::Normal, defaultName, &ok);
    if (!ok)
        return;
    sfName=sfName.trimmed();
    if(sfName.isEmpty())
    {
        return;
    }
    if (m_ptrHelper->cloud()->getScalarFieldIndexByName(qPrintable(sfName)) >= 0)
    {
        QString msg="Exists a field with name: "+sfName;
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  msg);
         return;
    }
    ScalarType sfValue = 0;
    ok = false;
    static double s_constantDoubleSFValue = 0.0;
    double dValue = static_cast<ScalarType>(QInputDialog::getDouble(this, QT_TR_NOOP("Add constant value"), QT_TR_NOOP("value"), s_constantDoubleSFValue, -1.0e9, 1.0e9, 8, &ok));
    if (ok)
    {
        s_constantDoubleSFValue = dValue;
        sfValue = static_cast<ScalarType>(dValue);
    }
    if (!ok)
    {
        // cancelled by the user
        return;
    }
    int sfIdx = m_ptrHelper->cloud()->getScalarFieldIndexByName(qPrintable(sfName));
    if (sfIdx < 0)
        sfIdx = m_ptrHelper->cloud()->addScalarField(qPrintable(sfName));
    if (sfIdx < 0)
    {
        QString msg="An error occurred! (see console)";
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  msg);
         return;
    }

    CCCoreLib::ScalarField* sf = m_ptrHelper->cloud()->getScalarField(sfIdx);
    assert(sf);
    if (!sf)
    {
        assert(false);
        return;
    }

    sf->fill(sfValue);
    sf->computeMinAndMax();
    updateCbScalarFields();
    int cbScalarFieldPosition=cbScalarField->findText(sfName);
    cbScalarField->setCurrentIndex(cbScalarFieldPosition);
//    m_ptrHelper->cloud()->setCurrentDisplayedScalarField(sfIdx);
//    m_ptrHelper->cloud()->showSF(true);
//    updateUI();

//    m_ptrHelper->cloud()->redrawDisplay();

//	if (ccEntityAction::sfAddConstant(m_ptrHelper->cloud, sfName, false, this))
//	{
//		updateUI();
//		cloud->redrawDisplay();
//	}

}

void ccTidopToolsRandomForestClassificationDlg::deleteScalarFieldClicked()
{
    if (!m_ptrHelper)
    {
        return;
    }
    QString currentFieldName=cbScalarField->currentText();
    if(currentFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                Qt::CaseInsensitive)==0)
    {
        return;
    }
    int numberOfScalarFields=m_ptrHelper->cloud()->getNumberOfScalarFields();
    if(numberOfScalarFields<2)
    {
        QString msg="There is only one scalar field";
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  msg);
         return;
    }
    int sfIdx = m_ptrHelper->cloud()->getScalarFieldIndexByName(qPrintable(currentFieldName));
    if (sfIdx < 0)
    {
        QString msg="An error occurred! (see console)";
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  msg);
         return;
    }
    m_ptrHelper->cloud()->deleteScalarField( sfIdx);
    updateCbScalarFields();
    int cbScalarFieldPosition=0;//cbScalarField->findText(sfName);
    cbScalarField->setCurrentIndex(cbScalarFieldPosition);
}

void ccTidopToolsRandomForestClassificationDlg::cloneScalarFieldClicked()
{
    if (!m_ptrHelper)
    {
        return;
    }
    QString currentFieldName=cbScalarField->currentText();
    if(currentFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                Qt::CaseInsensitive)==0)
    {
        return;
    }
    //ask for a name
    bool ok;
    QString defaultName;
    if(currentFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                Qt::CaseInsensitive)!=0)
    {
        defaultName=currentFieldName+TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_DEFAULT_NEW_FIELD_SUFFIX;
    }
    else
    {
        defaultName=TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_DEFAULT_NEW_FIELD;
    }
    QString clonedFieldName = QInputDialog::getText(this, tr("New SF name"), tr("SF name (must be unique)"), QLineEdit::Normal, defaultName, &ok);
    if (!ok)
        return;
    clonedFieldName=clonedFieldName.trimmed();
    if(clonedFieldName.isEmpty())
    {
        return;
    }
    if (m_ptrHelper->cloud()->getScalarFieldIndexByName(qPrintable(clonedFieldName)) >= 0)
    {
        QString msg="Exists a field with name: "+clonedFieldName;
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  msg);
         return;
    }

    int sfIdx = m_ptrHelper->cloud()->getScalarFieldIndexByName(qPrintable(currentFieldName));
    if (sfIdx < 0)
    {
        QString msg="An error occurred! (see console)";
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  msg);
         return;
    }
    int sfClonedIdx = m_ptrHelper->cloud()->getScalarFieldIndexByName(qPrintable(clonedFieldName));
    if (sfClonedIdx < 0)
        sfClonedIdx = m_ptrHelper->cloud()->addScalarField(qPrintable(clonedFieldName));
    if (sfClonedIdx < 0)
    {
        QString msg="An error occurred! (see console)";
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  msg);
         return;
    }

    CCCoreLib::ScalarField* sf = m_ptrHelper->cloud()->getScalarField(sfIdx);
    assert(sf);
    if (!sf)
    {
        assert(false);
        return;
    }

    CCCoreLib::ScalarField* sfCloned = m_ptrHelper->cloud()->getScalarField(sfClonedIdx);
    assert(sfClonedIdx);
    if (!sfClonedIdx)
    {
        assert(false);
        return;
    }
    for (unsigned i = 0; i < m_ptrHelper->cloud()->size(); ++i)
    {
//        // skip camera outside point
//        if (!m_pointInFrustum[i])
//            continue;

        ScalarType code = sf->getValue(i);
        sfCloned->setValue(i, code);
    }
    sfCloned->computeMinAndMax();
    updateCbScalarFields();
    int cbClonedScalarFieldPosition=cbScalarField->findText(clonedFieldName);
    cbScalarField->setCurrentIndex(cbClonedScalarFieldPosition);
}

void ccTidopToolsRandomForestClassificationDlg::mouseMoved(int x, int y, Qt::MouseButtons buttons)
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

void ccTidopToolsRandomForestClassificationDlg::classificationTrainingScalarFieldChanged()
{
    cbClassificationTargetScalarField->setEnabled(false);
    cbClassificationStep->setCurrentIndex(0);
//    cbClassificationStep->setEnabled(false);
    pbClassificationStepParameters->setEnabled(false);
    pbClassificationStepProcess->setEnabled(false);
    QString classificationTrainingScalarFieldName=cbClassificationTrainingScalarField->currentText();
    if(classificationTrainingScalarFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                                     Qt::CaseInsensitive)==0)
    {
        return;
    }
    cbClassificationTargetScalarField->setEnabled(true);
}

void ccTidopToolsRandomForestClassificationDlg::classificationTargetScalarFieldChanged()
{
    cbClassificationStep->setCurrentIndex(0);
//    cbClassificationStep->setEnabled(false);
    pbClassificationStepParameters->setEnabled(false);
    pbClassificationStepProcess->setEnabled(false);
    QString classificationTargetScalarFieldName=cbClassificationTargetScalarField->currentText();
    if(classificationTargetScalarFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                                     Qt::CaseInsensitive)==0)
    {
        return;
    }
//    cbClassificationStep->setEnabled(true);
}

void ccTidopToolsRandomForestClassificationDlg::classificationStepChanged()
{
//    if(!cbClassificationTargetScalarField->setEnabled(false)))
    pbClassificationStepParameters->setEnabled(false);
    pbClassificationStepProcess->setEnabled(false);
    QString classificationStep=cbClassificationStep->currentText();
    if(classificationStep.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                                     Qt::CaseInsensitive)==0)
    {
        if(cbClassificationTargetScalarField->isEnabled())
        {
            QString classificationTrainingScalarField=cbClassificationTrainingScalarField->currentText();
            if(classificationTrainingScalarField.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                                             Qt::CaseInsensitive)==0)
            {
                cbClassificationTargetScalarField->setCurrentIndex(0);
                cbClassificationTargetScalarField->setEnabled(false);
            }
        }
        return;
    }
    else if(classificationStep.compare(RFC_PROCESS_COMPUTE_FEATURES,
                                                     Qt::CaseInsensitive)==0)
    {
        if(cbClassificationTargetScalarField->isEnabled())
        {
            QString classificationTrainingScalarField=cbClassificationTrainingScalarField->currentText();
            if(classificationTrainingScalarField.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                                             Qt::CaseInsensitive)==0)
            {
                cbClassificationTargetScalarField->setCurrentIndex(0);
                cbClassificationTargetScalarField->setEnabled(false);
            }
        }
    }
    else if(classificationStep.compare(RFC_PROCESS_CLASSIFICATION,
                                       Qt::CaseInsensitive)==0
            ||classificationStep.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,
                                         Qt::CaseInsensitive)==0
            ||classificationStep.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,
                                         Qt::CaseInsensitive)==0)
    {
        if(!cbClassificationTargetScalarField->isEnabled())
        {
            cbClassificationTargetScalarField->setEnabled(true);
        }
    }
    pbClassificationStepParameters->setEnabled(true);
    pbClassificationStepProcess->setEnabled(true);
}

void ccTidopToolsRandomForestClassificationDlg::selectParameters()
{
    QString command=cbClassificationStep->currentText();
    if(command.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        return;
    }
    if(command.compare(RFC_PROCESS_COMPUTE_FEATURES,Qt::CaseInsensitive)==0
            &&!m_useCGAL)
    {
        command=RFC_PROCESS_COMPUTE_FEATURES_NO_CGAL;
    }

    ParametersManagerDialog parameterDialog(m_ptrParametersManager,
                                            command);

}

void ccTidopToolsRandomForestClassificationDlg::selectClassificationProcess()
{
    QString functionName="ccTidopToolsRandomForestClassificationDlg::selectClassificationProcess";
    m_functionName=functionName;
    m_classificationProcessCommand=cbClassificationStep->currentText();
    QString strAuxError;

    if(m_useCGAL)
    {
        if(!m_ptrClassificationToolCGAL->initializeFromCCPointCloud(m_ptrHelper->cloud(),
                                                                    m_ptrClassificationModel,
                                                                    strAuxError))
        {
            QString strError=functionName;
            strError+=QObject::tr("\nIn process:\n%1").arg(m_classificationProcessCommand);
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      strError);
             return;
        }
    }
    else
    {
        if(!m_ptrTidopToolsETHZRandomForest->initializeFromCCPointCloud(m_ptrHelper->cloud(),
                                                                    m_ptrClassificationModel,
                                                                    strAuxError))
        {
            QString strError=functionName;
            strError+=QObject::tr("\nIn process:\n%1").arg(m_classificationProcessCommand);
            strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      strError);
             return;
        }
    }

    if(m_classificationProcessCommand.compare(RFC_PROCESS_COMPUTE_FEATURES,Qt::CaseInsensitive)==0
            &&m_useCGAL)
    {
        m_ptrProcessesDialog->setModal(true);
        m_ptrProcessesDialog->setAutoReset(false);
        m_ptrProcessesDialog->setCancelButton(nullptr);
//        ptrDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_ptrProcessesDialog->setRange(0,0);
        QString text=QObject::tr("Computing features ... ");
        text+=QObject::tr("\nWait for finished ... ");
        m_ptrProcessesDialog->setLabelText(text);
        connect(&m_futureWatcher,&QFutureWatcher<bool>::finished,
                this,&ccTidopToolsRandomForestClassificationDlg::on_process_finished);
        m_future=QtConcurrent::run(computeFeatures,
                                   m_ptrParametersManager);
        m_futureWatcher.setFuture(m_future);
        m_ptrProcessesDialog->show();
        return;
    }
    else if(m_classificationProcessCommand.compare(RFC_PROCESS_COMPUTE_FEATURES,Qt::CaseInsensitive)==0
            &&!m_useCGAL)
    {
        if(!computeFeatures(m_ptrParametersManager))
        {
            QString strError=functionName;
            strError+=QObject::tr("\nComputing features, error:\n%1").arg(m_strError);
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      strError);
            return;
        }
        QMessageBox msgBox;
        msgBox.setWindowTitle("Process: "+m_classificationProcessCommand);
        QString msgText=QObject::tr("Processing time (seconds): %1\n").arg(m_strTime);
        if(!m_strReport.isEmpty())
        {
            msgText+=QObject::tr("Processing report:\n%1").arg(m_strReport);
        }
        msgBox.setText(msgText);
        msgBox.exec();
    }
    else if(m_classificationProcessCommand.compare(RFC_PROCESS_TRAINING,Qt::CaseInsensitive)==0)
    {
        QString trainingScalarFieldName=cbClassificationTrainingScalarField->currentText();
        if(trainingScalarFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                       Qt::CaseInsensitive)==0)
        {
            QString strError=functionName;
            strError+=QObject::tr("\nSelect training scalar field");
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      strError);
             return;
        }
        QString targetScalarFieldName=cbClassificationTargetScalarField->currentText();
        if(targetScalarFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                       Qt::CaseInsensitive)==0)
        {
            QString strError=functionName;
            strError+=QObject::tr("\nSelect target scalar field");
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      strError);
             return;
        }
        bool getPreviousValues=false;
        if(!updatedScalarFields.contains(targetScalarFieldName))
        {
            QVector<int> aux;
            m_previousClassificationFieldValues[targetScalarFieldName]=aux;
            updatedScalarFields[targetScalarFieldName]=false;
            getPreviousValues=true;
        }
        else
        {
            if(updatedScalarFields[targetScalarFieldName])
            {
                getPreviousValues=true;
            }
        }
        if(getPreviousValues)
        {
            QVector<int> previousValues;
            if(!m_ptrHelper->getScalarFieldValues(targetScalarFieldName,
                                                  previousValues,
                                                  strAuxError))
            {
                QString strError=functionName;
                strError+=QObject::tr("\nGetting previous values for scalar field: %1").arg(targetScalarFieldName);
                strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
                QMessageBox::information(m_associatedWin->asWidget(),
                                          TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                          strError);
                 return;
            }
            m_previousClassificationFieldValues[targetScalarFieldName]=previousValues;
            updatedScalarFields[targetScalarFieldName]=false;
        }
        if(m_useCGAL)
        {
            m_ptrProcessesDialog->setModal(true);
            m_ptrProcessesDialog->setAutoReset(false);
            m_ptrProcessesDialog->setCancelButton(nullptr);
    //        ptrDialog->setAttribute(Qt::WA_DeleteOnClose);
            m_ptrProcessesDialog->setRange(0,0);
            QString text=QObject::tr("Training ... ");
            text+=QObject::tr("\nWait for finished ... ");
            m_ptrProcessesDialog->setLabelText(text);
            connect(&m_futureWatcher,&QFutureWatcher<bool>::finished,
                    this,&ccTidopToolsRandomForestClassificationDlg::on_process_finished);
            m_future=QtConcurrent::run(train,
                                       m_ptrParametersManager,
                                       trainingScalarFieldName,
                                       targetScalarFieldName);
            m_futureWatcher.setFuture(m_future);
            m_ptrProcessesDialog->show();
        }
        else
        {
            if(!train(m_ptrParametersManager,
                      trainingScalarFieldName,
                      targetScalarFieldName))
            {
                QString strError=functionName;
                strError+=QObject::tr("\nTraining, error:\n%1").arg(m_strError);
                QMessageBox::information(m_associatedWin->asWidget(),
                                          TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                          strError);
                return;
            }
            QString trainingScalarFieldName=cbClassificationTrainingScalarField->currentText();
            QString targetScalarFieldName=cbClassificationTargetScalarField->currentText();
            QString scalarFieldName=cbScalarField->currentText();
            if(targetScalarFieldName.compare(scalarFieldName,Qt::CaseInsensitive)!=0)
            {
                int targetScalarFieldPosition=cbScalarField->findText(targetScalarFieldName);
                if(targetScalarFieldPosition!=-1)
                {
                    cbScalarField->setCurrentIndex(targetScalarFieldPosition);
                }
            }
            else
            {
    //            m_ptrHelper->redrawDisplay();
    //            cbScalarField->setCurrentIndex(targetScalarFieldPosition);
                scalarFieldIndexChanged(cbScalarField->currentIndex());
            }
            QMessageBox msgBox;
            msgBox.setWindowTitle("Process: "+m_classificationProcessCommand);
            QString msgText=QObject::tr("Processing time (seconds): %1\n").arg(m_strTime);
            if(!m_strReport.isEmpty())
            {
                msgText+=QObject::tr("Processing report:\n%1").arg(m_strReport);
            }
            msgBox.setText(msgText);
            msgBox.exec();
        }
        return;
    }
    else if(m_classificationProcessCommand.compare(RFC_PROCESS_CLASSIFICATION,Qt::CaseInsensitive)==0
            ||m_classificationProcessCommand.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0
            ||m_classificationProcessCommand.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
    {
        QString targetScalarFieldName=cbClassificationTargetScalarField->currentText();
        if(targetScalarFieldName.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,
                                       Qt::CaseInsensitive)==0)
        {
            QString strError=functionName;
            strError+=QObject::tr("\nSelect target scalar field");
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      strError);
             return;
        }
        if(m_useCGAL)
        {
            m_ptrProcessesDialog->setModal(true);
            m_ptrProcessesDialog->setAutoReset(false);
            m_ptrProcessesDialog->setCancelButton(nullptr);
    //        ptrDialog->setAttribute(Qt::WA_DeleteOnClose);
            m_ptrProcessesDialog->setRange(0,0);
            QString text=QObject::tr("Classifiying ... ");
            text+=QObject::tr("\nWait for finished ... ");
            m_ptrProcessesDialog->setLabelText(text);
            connect(&m_futureWatcher,&QFutureWatcher<bool>::finished,
                    this,&ccTidopToolsRandomForestClassificationDlg::on_process_finished);
            m_future=QtConcurrent::run(classify,
                                       m_ptrParametersManager,
                                       targetScalarFieldName);
            m_futureWatcher.setFuture(m_future);
            m_ptrProcessesDialog->show();
        }
        else
        {
            if(m_classificationProcessCommand.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0
                    ||m_classificationProcessCommand.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
            {
                m_classificationProcessCommand=RFC_PROCESS_CLASSIFICATION;
                QString strError=functionName;
                strError+=QObject::tr("\nWithout using the GCAL library the only");
                strError+=QObject::tr("\nclassification method implemented is:\n%1").arg(RFC_PROCESS_CLASSIFICATION);
                QMessageBox::information(m_associatedWin->asWidget(),
                                          TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                          strError);
            }
            if(!classify(m_ptrParametersManager,
                         targetScalarFieldName))
            {
                QString strError=functionName;
                strError+=QObject::tr("\nClassifiying, error:\n%1").arg(m_strError);
                QMessageBox::information(m_associatedWin->asWidget(),
                                          TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                          strError);
                return;
            }
            QString targetScalarFieldName=cbClassificationTargetScalarField->currentText();
            QString scalarFieldName=cbScalarField->currentText();
            if(targetScalarFieldName.compare(scalarFieldName,Qt::CaseInsensitive)!=0)
            {
                int targetScalarFieldPosition=cbScalarField->findText(targetScalarFieldName);
                if(targetScalarFieldPosition!=-1)
                {
                    cbScalarField->setCurrentIndex(targetScalarFieldPosition);
                }
            }
            else
            {
    //            m_ptrHelper->redrawDisplay();
    //            cbScalarField->setCurrentIndex(targetScalarFieldPosition);
                scalarFieldIndexChanged(cbScalarField->currentIndex());
            }
            QMessageBox msgBox;
            msgBox.setWindowTitle("Process: "+m_classificationProcessCommand);
            QString msgText=QObject::tr("Processing time (seconds): %1\n").arg(m_strTime);
            if(!m_strReport.isEmpty())
            {
                msgText+=QObject::tr("Processing report:\n%1").arg(m_strReport);
            }
            msgBox.setText(msgText);
            msgBox.exec();
        }
        return;
    }
    m_ptrHelper->redrawDisplay();
}

void ccTidopToolsRandomForestClassificationDlg::useCGALClicked()
{
    m_useCGAL=ckbUseCGAL->isChecked();
}

void ccTidopToolsRandomForestClassificationDlg::on_process_finished()
{
    disconnect(&m_futureWatcher,&QFutureWatcher<bool>::finished,
               this,&ccTidopToolsRandomForestClassificationDlg::on_process_finished);
    m_ptrProcessesDialog->setLabelText("");
    m_ptrProcessesDialog->close();
    if(!m_future.result())
    {
        QString strError=m_functionName;
        strError+=QObject::tr("\nIn process:\n%1").arg(RFC_PROCESS_COMPUTE_FEATURES);
        strError+=QObject::tr("\nError:\n%1").arg(m_strError);
        QMessageBox::information(m_associatedWin->asWidget(),
                                 TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                 strError);
        return;
    }
    if(m_classificationProcessCommand.compare(RFC_PROCESS_TRAINING,Qt::CaseInsensitive)==0
            ||m_classificationProcessCommand.compare(RFC_PROCESS_CLASSIFICATION,Qt::CaseInsensitive)==0
            ||m_classificationProcessCommand.compare(RFC_PROCESS_CLASSIFICATION_WITH_LOCAL_SMOOTHING,Qt::CaseInsensitive)==0
            ||m_classificationProcessCommand.compare(RFC_PROCESS_CLASSIFICATION_WITH_GRAPH_CUT,Qt::CaseInsensitive)==0)
    {
        QString trainingScalarFieldName=cbClassificationTrainingScalarField->currentText();
        QString targetScalarFieldName=cbClassificationTargetScalarField->currentText();
        QString scalarFieldName=cbScalarField->currentText();
        if(targetScalarFieldName.compare(scalarFieldName,Qt::CaseInsensitive)!=0)
        {
            int targetScalarFieldPosition=cbScalarField->findText(targetScalarFieldName);
            if(targetScalarFieldPosition!=-1)
            {
                cbScalarField->setCurrentIndex(targetScalarFieldPosition);
            }
        }
        else
        {
//            m_ptrHelper->redrawDisplay();
//            cbScalarField->setCurrentIndex(targetScalarFieldPosition);
            scalarFieldIndexChanged(cbScalarField->currentIndex());
        }
        QMessageBox msgBox;
        msgBox.setWindowTitle("Process: "+m_classificationProcessCommand);
        QString msgText=QObject::tr("Processing time (seconds): %1\n").arg(m_strTime);
        if(!m_strReport.isEmpty())
        {
            msgText+=QObject::tr("Processing report:\n%1").arg(m_strReport);
        }
        msgBox.setText(msgText);
        msgBox.exec();
    }
    else if(m_classificationProcessCommand.compare(RFC_PROCESS_COMPUTE_FEATURES,Qt::CaseInsensitive)==0)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Process: "+m_classificationProcessCommand);
        QString msgText=QObject::tr("Processing time (seconds): %1\n").arg(m_strTime);
//        if(!m_strReport.isEmpty())
//        {
//            msgText+=QObject::tr("Processing report:\n%1").arg(m_strReport);
//        }
        msgBox.setText(msgText);
        msgBox.exec();
    }
}

bool ccTidopToolsRandomForestClassificationDlg::initialize(QString &strError)
{
    strError.clear();
    QString functionName="ccTidopToolsRandomForestClassificationDlg::initialize";
    m_basePath=QDir::currentPath();
    QString parametersFileName=m_basePath+RFC_PARAMETERS_FILE_PATH;
    if(!QFile::exists(parametersFileName))
    {
        strError=tr("Parameters file not found:ºn%1").arg(parametersFileName);
    }
    m_ptrParametersManager=new ParametersManager();
    QString strAuxError;
    if(!m_ptrParametersManager->loadFromXml(parametersFileName,strAuxError))
    {
        strError=functionName;
        strError+=QObject::tr("\nLoading parameters manager from file:\n%1").arg(parametersFileName);
        strError+=QObject::tr("\nError:\n%1").arg(strAuxError);
        delete(m_ptrParametersManager);
        m_ptrParametersManager=nullptr;
        return(false);
    }
    return(true);
}

bool ccTidopToolsRandomForestClassificationDlg::classify(ParametersManager *ptrParametersManager,
                                                         QString targetFieldName)
{
    m_strError.clear();
    m_strReport.clear();
    m_strTime.clear();
    QString method=m_classificationProcessCommand;
    if(m_useCGAL)
    {
        return(m_ptrClassificationToolCGAL->classify(ptrParametersManager,
                                                     method,
                                                     targetFieldName,
                                                     m_strTime,
                                                     m_strReport,
                                                     m_strError));
    }
    else
    {
        return(m_ptrTidopToolsETHZRandomForest->classify(ptrParametersManager,
                                                         method,
                                                         targetFieldName,
                                                         m_strTime,
                                                         m_strReport,
                                                         m_strError));
    }
}

bool ccTidopToolsRandomForestClassificationDlg::computeFeatures(ParametersManager *ptrParametersManager)
{
    m_strError.clear();
    m_strReport.clear();
    m_strTime.clear();
    if(m_useCGAL)
    {
        QStringList featuresNamesFromTrainingFile;
        return(m_ptrClassificationToolCGAL->computeFeatures(ptrParametersManager,
                                                            featuresNamesFromTrainingFile,
                                                            m_strTime,
                                                            m_strReport,
                                                            m_strError));
    }
    else
    {
//        ccProgressDialog* pDlg=new ccProgressDialog(true, this);
//        pDlg->setAutoClose(false);
        QStringList featuresStringsFromTrainingFile;
        return(m_ptrTidopToolsETHZRandomForest->computeFeatures(ptrParametersManager,
                                                                featuresStringsFromTrainingFile,
                                                                m_strTime,
                                                                m_strReport,
                                                                m_strError));
//        delete(pDlg);
    }
}
/*
bool ccTidopToolsRandomForestClassificationDlg::computeFeatures(ParametersManager *ptrParametersManager,
                                                                QString& strError)
{
    m_strError.clear();
    m_strReport.clear();
    m_strTime.clear();
    if(m_useCGAL)
    {
        return(m_ptrClassificationToolCGAL->computeFeatures(ptrParametersManager,
                                                            m_strTime,
                                                            m_strReport));
    }
    else
    {
        return(m_ptrTidopToolsETHZRandomForest->computeFeatures(ptrParametersManager,
                                                                m_strTime,
                                                                m_strReport,
                                                                m_pDlg));
    }
}
*/
bool ccTidopToolsRandomForestClassificationDlg::train(ParametersManager *ptrParametersManager,
                                                      QString trainFieldName,
                                                      QString targetFieldName)
{
    m_strError.clear();
    m_strReport.clear();
    m_strTime.clear();
    if(m_useCGAL)
    {
        return(m_ptrClassificationToolCGAL->train(ptrParametersManager,
                                                  trainFieldName,
                                                  targetFieldName,
                                                  m_strTime,
                                                  m_strReport,
                                                  m_strError));
    }
    else
    {
        return(m_ptrTidopToolsETHZRandomForest->train(ptrParametersManager,
                                                      trainFieldName,
                                                      targetFieldName,
                                                      m_strTime,
                                                      m_strReport,
                                                      m_strError));
    }
}

bool ccTidopToolsRandomForestClassificationDlg::eventFilter(QObject* obj, QEvent* event)
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

void ccTidopToolsRandomForestClassificationDlg::scalarFieldIndexChanged(int index)
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

void ccTidopToolsRandomForestClassificationDlg::selectedPointsToolsChanged(int index)
{
    cbSelectedPointsScalarValues->clear();
    cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    QString tool=cbSelectedPointsTools->currentText();

    QMap<int,QString> dataTags = m_ptrClassificationModel->getDataTags();
    if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->setEnabled(false);
        pbSelectedPointsProcess->setEnabled(false);
        return;
    }
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS,
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
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
        QMap<int,QString>::const_iterator iterDataTags=dataTags.begin();
        while(iterDataTags!=dataTags.end())
        {
            QString itemTag=iterDataTags.value();
            cbSelectedPointsScalarValues->addItem(itemTag);
            iterDataTags++;
        }
    }
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER,
                    Qt::CaseInsensitive)==0)
    {
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES);
        cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES);
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

void ccTidopToolsRandomForestClassificationDlg::selectedPointsScalarValuesChanged(int index)
{
    QString targetClassTag=cbSelectedPointsScalarValues->currentText();
    if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        pbSelectedPointsProcess->setEnabled(false);
        return;
    }
    else
    {
        pbSelectedPointsProcess->setEnabled(true);
    }
}

void ccTidopToolsRandomForestClassificationDlg::codeChanged(ccClassificationModel::Item& item, int oldCode)
{
    if (m_ptrHelper)
	{
        m_ptrHelper->changeCode(item, static_cast<ScalarType>(oldCode));
	}
}

void ccTidopToolsRandomForestClassificationDlg::colorChanged(ccClassificationModel::Item& item)
{
    if (!m_ptrHelper)
	{
		return;
	}

    item.count = m_ptrHelper->apply(item, true);

	// refresh point count
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsRandomForestClassificationDlg::setAllVisible()
{
    m_ptrClassificationModel->setAllVisible();
    m_ptrHelper->apply(m_ptrClassificationModel->getData());
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsRandomForestClassificationDlg::setAllInvisible()
{
    m_ptrClassificationModel->setAllInvisible();
    m_ptrHelper->apply(m_ptrClassificationModel->getData());
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsRandomForestClassificationDlg::setAllLocked()
{
    m_ptrClassificationModel->setAllLocked();
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsRandomForestClassificationDlg::setAllUnlocked()
{
    m_ptrClassificationModel->setAllUnlocked();
    m_ptrClassificationModel->refreshData();
}

void ccTidopToolsRandomForestClassificationDlg::selectedPointsProcess()
{
    ccClassificationModel::Item* selectedItem = m_ptrClassificationModel->getSelectedItem();
    ccClassificationModel::Item* removedItem = m_ptrClassificationModel->getRemovedItem();
    if (!m_ptrHelper)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  "Invalid helper");
        return;
    }
    QString tool=cbSelectedPointsTools->currentText();
    if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                  "Select a tool");
        return;
    }
    QString targetClassTag=cbSelectedPointsScalarValues->currentText();
    if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT,Qt::CaseInsensitive)==0)
    {
        QMessageBox::information(m_associatedWin->asWidget(),
                                  TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
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
    if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS,
                         Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0
                ||targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS,
                         Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        m_ptrHelper->toolRecoverClass(affected,m_ptrClassificationModel);
    }
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE,
                    Qt::CaseInsensitive)==0)
    {
        if(selectedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      "No points selected");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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
    else if(tool.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER,
                    Qt::CaseInsensitive)==0)
    {
        if(removedItem->count==0)
        {
            QMessageBox::information(m_associatedWin->asWidget(),
                                      TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_TITLE,
                                      "No points removed");
            return;
        }
        QList<ccClassificationModel::Item> items= m_ptrClassificationModel->getData();
        QList<int> targetItemsCode;
        if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_CLASSES,
                        Qt::CaseInsensitive)==0)
        {
            for (int i = 0; i < items.length(); ++i)
            {
                const ccClassificationModel::Item& item = items[i];
                targetItemsCode.append(item.code);
            }
        }
        else if(targetClassTag.compare(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_ALL_VISIBLE_CLASSES,
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

void ccTidopToolsRandomForestClassificationDlg::tableViewDoubleClicked(const QModelIndex& index)
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

void ccTidopToolsRandomForestClassificationDlg::tableViewClicked(const QModelIndex &index)
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
}
/*
void ccTidopToolsRandomForestClassificationDlg::updateInputOutput()
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
void ccTidopToolsRandomForestClassificationDlg::uptdateSelectedPointsTools()
{
    auto data = m_ptrClassificationModel->getData();

    cbSelectedPointsTools->clear();
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_CHANGE_CLASS);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_SELECT_ONLY);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_UNSELECT);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_REMOVE);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER_ORIGINAL_CLASS);
    cbSelectedPointsTools->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_SELECTION_TOOLS_RECOVER);
//    cbSelectedPointsTools->addItem();

    cbSelectedPointsScalarValues->clear();
    cbSelectedPointsScalarValues->addItem(TIDOP_TOOLS_RANDOM_FOREST_CLASSIFICATION_DIALOG_NO_COMBO_SELECT);
    cbSelectedPointsScalarValues->setEnabled(false);
    pbSelectedPointsProcess->setEnabled(false);
}
