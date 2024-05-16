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

#ifndef TOOLS_ABOUT_DIALOG_HEADER
#define TOOLS_ABOUT_DIALOG_HEADER

#include <ui_qToolsAboutDlg.h>

//qCC_plugins
#include <ccMainAppInterface.h>

//Qt
#include <QMainWindow>

//! Dialog for displaying the CANUPO/UEB disclaimer
class ToolsAboutDlg : public QDialog, public Ui::ToolsAboutDlg
{
public:
	//! Default constructor
    ToolsAboutDlg(QWidget* parent = nullptr);
    QString getClassificationModel(){return(classificationModelComboBox->currentText());};
private:
    void initialize();
};

#endif //CANUPO_DISCLAIMER_DIALOG_HEADER
