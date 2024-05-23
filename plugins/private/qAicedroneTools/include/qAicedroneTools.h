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

#ifndef Q_AICEDRONETOOLS_PLUGIN_HEADER
#define Q_AICEDRONETOOLS_PLUGIN_HEADER

//qCC
#include <ccStdPluginInterface.h>

//qCC_db
#include <ccHObject.h>

class ToolsAboutDlg;
class ccToolsManualClassificationDlg;
class ccToolsRandomForestClassificationDlg;
class ccPointCloudTilesFileDlg;

//! AICEDRONE Tools plugin
/** Input references here
**/
class qAicedroneToolsPlugin : public QObject, public ccStdPluginInterface
{
	Q_OBJECT
	Q_INTERFACES( ccPluginInterface ccStdPluginInterface )

    Q_PLUGIN_METADATA( IID "cccorp.cloudcompare.plugin.qAicedroneTools" FILE "../info.json" )

public:

	//! Default constructor
	qAicedroneToolsPlugin(QObject* parent = nullptr);

	//inherited from ccStdPluginInterface
	void onNewSelection(const ccHObject::Container& selectedEntities) override;
	virtual QList<QAction*> getActions() override;
	// virtual void registerCommands(ccCommandLineInterface* cmd) override;

protected:

    void doAboutAction();
    void doManualClassificationAction();
    void doRandomForestClassificationAction();
    void doPointCloudTilesFileAction();

protected:

	//! About action
    QAction* m_ptrAboutAction;
    QAction* m_ptrManualClassificationAction;
	//! Train action
    QAction* m_ptrRandomForestClassificationAction;
    QAction* m_ptrPointCloudTilesFileAction;

	//! Currently selected entities
	ccHObject::Container m_selectedEntities;

    ToolsAboutDlg* m_ptrAboutDlg;
    ccToolsManualClassificationDlg* m_ptrManualClassificationDlg;
    ccToolsRandomForestClassificationDlg* m_ptrRandomForestClassificationDlg;
    ccPointCloudTilesFileDlg* m_ptrPointCloudTilesFileDlg;

    bool m_accepted;
};

#endif //Q_AICEDRONETOOLS_PLUGIN_HEADER
