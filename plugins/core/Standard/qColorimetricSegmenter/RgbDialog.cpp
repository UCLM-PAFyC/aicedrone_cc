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

#include "RgbDialog.h"

//common
#include <ccPickingHub.h>
#include <ccGenericPointCloud.h>
#include <ccQtHelpers.h>

//qCC_gl
#include <ccGLWindow.h>

//Qt
#include <QCheckBox>

//Semi-persistent parameters
static int s_lastFirstR = 0;
static int s_lastFirstG = 0;
static int s_lastFirstB = 0;
static int s_lastSecondR = 0;
static int s_lastSecondG = 0;
static int s_lastSecondB = 0;
static int s_lastMargin = 0;

RgbDialog::RgbDialog(ccPickingHub* pickingHub, QWidget* parent)
	: QDialog(parent)
	, Ui::RgbDialog()
	, m_pickingHub(pickingHub)
{
	assert(pickingHub);

	setupUi(this);
	setModal(false);

	red_first->setValue(s_lastFirstR);
	green_first->setValue(s_lastFirstG);
	blue_first->setValue(s_lastFirstB);
	red_second->setValue(s_lastSecondR);
	green_second->setValue(s_lastSecondG);
	blue_second->setValue(s_lastSecondB);
	margin->setValue(s_lastMargin);

	updateFirstColorButton();
	updateSecondColorButton();

	//Link between Ui and actions
	connect(pointPickingButton_first, &QCheckBox::toggled, this, &RgbDialog::pickPoint_first);
	connect(pointPickingButton_second, &QCheckBox::toggled, this, &RgbDialog::pickPoint_second);
	connect(red_first,    qOverload<int>(&QSpinBox::valueChanged), this, &RgbDialog::updateFirstColorButton);
	connect(green_first,  qOverload<int>(&QSpinBox::valueChanged), this, &RgbDialog::updateFirstColorButton);
	connect(blue_first,   qOverload<int>(&QSpinBox::valueChanged), this, &RgbDialog::updateFirstColorButton);
	connect(red_second,   qOverload<int>(&QSpinBox::valueChanged), this, &RgbDialog::updateSecondColorButton);
	connect(green_second, qOverload<int>(&QSpinBox::valueChanged), this, &RgbDialog::updateSecondColorButton);
	connect(blue_second,  qOverload<int>(&QSpinBox::valueChanged), this, &RgbDialog::updateSecondColorButton);
	connect(this, &QDialog::accepted, this, &RgbDialog::storeParameters);

	//auto disable picking mode on quit
	connect(this, &QDialog::finished, [&]()
		{
			if (pointPickingButton_first->isChecked()) pointPickingButton_first->setChecked(false);
			if (pointPickingButton_second->isChecked()) pointPickingButton_second->setChecked(false);
		}
	);
}

void RgbDialog::storeParameters()
{
	s_lastFirstR = red_first->value();
	s_lastFirstG = green_first->value();
	s_lastFirstB = blue_first->value();
	s_lastSecondR = red_second->value();
	s_lastSecondG = green_second->value();
	s_lastSecondB = blue_second->value();
	s_lastMargin = margin->value();
}

void RgbDialog::updateFirstColorButton()
{
	ccQtHelpers::SetButtonColor(firstColorToolButton, QColor(red_first->value(), green_first->value(), blue_first->value()));
}

void RgbDialog::updateSecondColorButton()
{
	ccQtHelpers::SetButtonColor(secondColorToolButton, QColor(red_second->value(), green_second->value(), blue_second->value()));
}

void RgbDialog::pickPoint_first(bool state)
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

void RgbDialog::pickPoint_second(bool state)
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

void RgbDialog::onItemPicked(const PickedItem& pi)
{
	if (!pi.entity || !m_pickingHub)
	{
		return;
	}

	if (pi.entity->isKindOf(CC_TYPES::POINT_CLOUD))
	{
		ccGenericPointCloud* cloud = static_cast<ccGenericPointCloud*>(pi.entity);
		if (cloud->hasColors())
		{
			//Get RGB values of the picked point
			const ccColor::Rgb& rgb = cloud->getPointColor(pi.itemIndex);
			ccLog::Print(QString("%0 point picked: %1 - color: R=%2 G=%3 B=%4")
				.arg(pointPickingButton_first->isChecked() ? "First" : "Second")
				.arg(pi.itemIndex)
				.arg(rgb.r)
				.arg(rgb.g)
				.arg(rgb.b)
			);

			if (pointPickingButton_first->isChecked())
			{
				red_first->setValue(rgb.r);
				green_first->setValue(rgb.g);
				blue_first->setValue(rgb.b);

				pointPickingButton_first->setChecked(false);
			}
			else
			{
				red_second->setValue(rgb.r);
				green_second->setValue(rgb.g);
				blue_second->setValue(rgb.b);

				pointPickingButton_second->setChecked(false);
			}
		}
		else
		{
			ccLog::Print("This point cloud doesn't have RGB colors");
		}
	}
}
