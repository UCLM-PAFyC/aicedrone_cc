#pragma once

//##########################################################################
//#                                                                        #
//#            CLOUDCOMPARE PLUGIN: ColorimetricSegmenter                  #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#    COPYRIGHT:	Tri-Thien TRUONG, Ronan COLLIER, Mathieu LETRONE       #
//#                                                                        #
//##########################################################################

#include <ui_RgbDialog.h>
#include "ccPickingListener.h"

//Qt
#include <QDialog>

class ccPickingHub;
class ccGLWindow;
/*
	Get the values of the RGB interface, and interactions
*/
class RgbDialog : public QDialog, public ccPickingListener, public Ui::RgbDialog
{
	Q_OBJECT
public:
	explicit RgbDialog(ccPickingHub* pickingHub, QWidget* parent = nullptr);

	//! Inherited from ccPickingListener
	virtual void onItemPicked(const PickedItem& pi);

public slots:
	//! Method for the first picking point functionnality
	void pickPoint_first(bool);
	//! Method for the second picking point functionnality
	void pickPoint_second(bool);
protected slots:
	//! Store semi-persistent parameters for the next run
	void storeParameters();

protected: //methods
	void updateFirstColorButton();
	void updateSecondColorButton();

protected: //members

	//! Picking hub
	ccPickingHub* m_pickingHub;
};
