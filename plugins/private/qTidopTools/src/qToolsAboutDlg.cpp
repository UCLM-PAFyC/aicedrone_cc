#include "../include/qToolsDefinitions.h"
#include "../include/qToolsAboutDlg.h"

ToolsAboutDlg::ToolsAboutDlg(QWidget* parent)
    : QDialog(parent)
    , Ui::ToolsAboutDlg()
{
    setupUi(this);
    initialize();
}

void ToolsAboutDlg::initialize()
{
    classificationModelComboBox->addItem(CC_CLASSIFICATION_MODEL_ASPRS_NAME);
    classificationModelComboBox->addItem(CC_CLASSIFICATION_MODEL_ARCHDATASET_NAME);
    classificationModelComboBox->addItem(CC_CLASSIFICATION_MODEL_IBERDROLA_NAME);
//    classificationModelComboBox->addItem(CC_CLASSIFICATION_MODEL_RAILWAY_NAME);
    addClassificationModelPushButton->setEnabled(false);
}

