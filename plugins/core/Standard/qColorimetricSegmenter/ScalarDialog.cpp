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

#include "ScalarDialog.h"

//common
#include <ccPickingHub.h>
#include <ccGenericPointCloud.h>

//qCC_gl
#include <ccGLWindow.h>

//Qt
#include <QCheckBox>

ScalarDialog::ScalarDialog(ccPickingHub* pickingHub, QWidget* parent)
	: QDialog(parent)
    , Ui::ScalarDialog()
	, m_pickingHub(pickingHub)
{
	assert(pickingHub);

	setModal(false);
	setupUi(this);

	//Link between Ui and actions
    connect(pointPickingButton_first, &QCheckBox::toggled, this, &ScalarDialog::pickPoint_first);
    connect(pointPickingButton_second, &QCheckBox::toggled, this, &ScalarDialog::pickPoint_second);
	
		
	//auto disable picking mode on quit
	connect(this, &QDialog::finished, [&]()
	{
		if (pointPickingButton_first->isChecked()) pointPickingButton_first->setChecked(false);
		if (pointPickingButton_second->isChecked()) pointPickingButton_second->setChecked(false);
	}
	);
}

/*
	Method for the first picking point functionnality
*/
void ScalarDialog::pickPoint_first(bool state)
{
	if (!m_pickingHub)
	{
		return;
	}
	if (state)
	{
		if (!m_pickingHub->addListener(this, true))
		{
			ccLog::Error("Can't start the picking process (another tool is using it)");
			state = false;
		}
	}
	else
	{
		m_pickingHub->removeListener(this);
	}
	pointPickingButton_first->blockSignals(true);
	pointPickingButton_first->setChecked(state);
	pointPickingButton_first->blockSignals(false);
}

/*
	Method for the second picking point functionnality
*/
void ScalarDialog::pickPoint_second(bool state)
{
	if (!m_pickingHub)
	{
		return;
	}
	if (state)
	{
		if (!m_pickingHub->addListener(this, true))
		{
			ccLog::Error("Can't start the picking process (another tool is using it)");
			state = false;
		}
	}
	else
	{
		m_pickingHub->removeListener(this);
	}
	pointPickingButton_second->blockSignals(true);
	pointPickingButton_second->setChecked(state);
	pointPickingButton_second->blockSignals(false);
}

/*
	Method applied after a point is picked by picking point functionnality
*/
void ScalarDialog::onItemPicked(const PickedItem& pi)
{
	if (!pi.entity || !m_pickingHub)
	{
		return;
	}

	if (pi.entity->isKindOf(CC_TYPES::POINT_CLOUD))
	{
		ccGenericPointCloud* cloud = static_cast<ccGenericPointCloud*>(pi.entity);
		if (cloud->hasDisplayedScalarField())
		{
			//Get the scalar value of the picked point
			const ScalarType scalarValue = cloud->getPointScalarValue(pi.itemIndex);
			ccLog::Print(QString("%0 point picked: %1 - SF value = %2")
				.arg(pointPickingButton_first->isChecked() ? "First" : "Second")
				.arg(pi.itemIndex)
				.arg(scalarValue)
			);

			if (pointPickingButton_first->isChecked())
			{
				first->setValue(scalarValue);

				pointPickingButton_first->setChecked(false);
			}
			else
			{
				second->setValue(scalarValue);

				pointPickingButton_second->setChecked(false);
			}
		}
		else
		{
			ccLog::Print("This point cloud doesn't have an active scalar field");
		}
	}
}
