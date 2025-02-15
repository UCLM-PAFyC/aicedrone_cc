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

#pragma once

/**
This is a custom 2DViewportLabel which takes up the entire viewport but is entirely transparent,
except for a circle with radius r around the mouse. 
*/

#include <ccStdPluginInterface.h>
#include <ccGLWindow.h>
#include <cc2DViewportObject.h>

//Qt
#include <QEvent>
#include <QPoint>
#include <QObject>

class ccMouseCircle : public cc2DViewportObject, public QObject
{
public:
	//constructor
	explicit ccMouseCircle(ccMainAppInterface* appInterface, ccGLWindow* owner, QString name = QString("MouseCircle"));

	//deconstructor
	~ccMouseCircle();

	//get the circle radius in px
	inline int getRadiusPx() const { return m_radius; }

	//get the circle radius in world coordinates
	float getRadiusWorld();

	//removes the link with the owner (no cleanup)
    inline void ownerIsDead() { m_ptrOwner = nullptr; }

	//sets whether scroll is allowed or not
	inline void setAllowScroll(bool state) { m_allowScroll = state; }
	
protected:
	//draws a circle of radius r around the mouse
	void draw(CC_DRAW_CONTEXT& context) override;

private:
    ccMainAppInterface* m_ptrApp;

	//ccGLWindow this overlay is attached to -> used to get mouse position & events
    ccGLWindow* m_ptrOwner;
	float m_pixelSize;

	//event to get mouse-move updates & trigger repaint
	bool eventFilter(QObject* obj, QEvent* event) override;
	
	int m_radius;
	int m_radiusStep;
	bool m_allowScroll;
};

