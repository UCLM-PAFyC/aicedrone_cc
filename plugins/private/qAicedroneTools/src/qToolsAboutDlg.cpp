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
    classificationModelComboBox->addItem(CC_CLASSIFICATION_MODEL_BREAKWATER_CUBES_NAME);
    classificationModelComboBox->addItem(CC_CLASSIFICATION_MODEL_RAILWAY_NAME);
    addClassificationModelPushButton->setEnabled(false);
}

