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

#include <ui_HSVDialog.h>
#include <ccPickingListener.h>

//Qt
#include <QDialog>

class ccPickingHub;

//! Get the values of the HSV interface, and interactions
class HSVDialog : public QDialog, public ccPickingListener, public Ui::HSVDialog
{
	Q_OBJECT
public:
	//! Constructor
	explicit HSVDialog(ccPickingHub* pickingHub, QWidget* parent = nullptr);

	//! Method applied after a point is picked by picking point functionnality
	/** Inherited from ccPickingListener
	**/
	void onItemPicked(const PickedItem& pi) override;

public slots:
	//! Method for the picking point functionnality
	void pickPoint(bool);
	//! Method applied after entering a value in RGB text fields
	void updateValues();
protected slots:
	//! Store semi-persistent parameters for the next run
	void storeParameters();

protected: //methods
	void updateColorButton();

protected: //members

	//! Picking hub
	ccPickingHub* m_pickingHub;

};
