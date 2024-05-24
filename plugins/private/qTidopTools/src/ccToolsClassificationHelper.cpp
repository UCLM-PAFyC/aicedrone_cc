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

#include "../include/ccToolsClassificationHelper.h"
#include "../include/ccMouseCircle.h"

//CC
#include <ccPointCloud.h>
#include <ccScalarField.h>
#include <ccMainAppInterface.h>
#include <ccPolyline.h>
#include <ManualSegmentationTools.h>

//QT
#include <QStringList>

//System
#include <thread>

ccToolsClassificationHelper::ccToolsClassificationHelper(ccMainAppInterface* app, ccPointCloud* cloud)
    : m_ptrApp ( app )
    , m_ptrCloud( cloud )
    , m_ptrFormerCloudColors( nullptr )
	, m_formerCloudColorsWereShown( false )
	, m_formerCloudSFWasShown( false )
	, m_scalarFieldIndex( 0 )
	, m_modified( false )
    , m_selected(false)
    , m_removed(false)
	, m_parameters{}
{
    m_ptrCloudTools=(ccPointCloudTools*)m_ptrCloud;
    m_projectedPoints.resize(m_ptrCloud->size());
    m_pointInFrustum.resize(m_ptrCloud->size());

    if (m_ptrCloud)
	{
        m_formerCloudColorsWereShown = m_ptrCloud->colorsShown();
        m_formerCloudSFWasShown = m_ptrCloud->sfShown();

        if (m_ptrCloud->hasColors())
		{
			// store the original colors
            m_ptrFormerCloudColors = m_ptrCloud->rgbaColors()->clone();
            if (!m_ptrFormerCloudColors)
			{
				ccLog::Error("Not enough memory to backup previous colors");
			}
            unsigned cloudSize = m_ptrCloud->size();
            m_cloudOriginalRGB.resize(cloudSize);
            for (unsigned i = 0; i < cloudSize; ++i)
            {
                m_cloudOriginalRGB[i]= m_ptrCloud->getPointColor(i);
            }
		}
		else
		{
			// check memory for rgb colors
            if (!m_ptrCloud->resizeTheRGBTable())
			{
				ccLog::Error("Not enough memory to show colors");
			}
		}

        m_ptrCloud->showColors(true);
        m_ptrCloud->showSF(false);
	}
}

ccToolsClassificationHelper::~ccToolsClassificationHelper()
{
    if (m_ptrCloud)
	{
        if (m_ptrFormerCloudColors)
		{
            if (m_ptrCloud->rgbaColors())
			{
				// restore original colors
                m_ptrFormerCloudColors->copy(*m_ptrCloud->rgbaColors());
			}

            delete m_ptrFormerCloudColors;
            m_ptrFormerCloudColors = nullptr;
		}
		else
		{
            m_ptrCloud->unallocateColors();
		}
		
        m_ptrCloud->showColors(m_formerCloudColorsWereShown);
        m_ptrCloud->showSF(m_formerCloudSFWasShown);
        m_ptrCloud->redrawDisplay();
	}
    m_ptrCloud = nullptr;
}

QStringList ccToolsClassificationHelper::getScalarFields()
{
    unsigned sfCount = m_ptrCloud->getNumberOfScalarFields();
	QStringList scalarFields;
    if (m_ptrCloud->hasScalarFields())
	{
		for (unsigned i = 0; i < sfCount; ++i)
		{
            scalarFields.append(QString(m_ptrCloud->getScalarFieldName(i)));
		}
	}
    return scalarFields;
}

bool ccToolsClassificationHelper::getScalarFieldValues(QString scalarFieldName,
                                                            QVector<int> &values,
                                                            QString &strError)
{
    QString functionName="ccToolsClassificationHelper::getScalarFieldValues";
    unsigned scalarFieldIndex=-1;
    QStringList scalarFields=getScalarFields();
    for(int i=0;i<scalarFields.size();i++)
    {
        if(scalarFields[i].compare(scalarFieldName,Qt::CaseInsensitive)==0)
        {
            scalarFieldIndex=i;
            break;
        }
    }
    if(scalarFieldIndex==-1)
    {
        QString strError=functionName;
        strError+=QObject::tr("\nNot found scalar field: %1").arg(scalarFieldIndex);
        return(false);
    }
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(scalarFieldIndex);
    if (!sf)
    {
        QString strError=functionName;
        strError+=QObject::tr("\nNot found scalar field: %1").arg(scalarFieldIndex);
                return(false);
    }
    values.clear();
    int numberOfPoints=m_ptrCloud->size();
    values.resize(numberOfPoints);
    int counter = 0;
    for (auto it = sf->begin(); it != sf->end(); ++it)
    {
        int value=qRound(sf->getValue(counter));
        values[counter]=value;
        counter++;
    }
    return(true);
}

bool ccToolsClassificationHelper::setScalarFieldValues(QString scalarFieldName,
                                                            QVector<int> &values,
                                                            QString &strError)
{
    QString functionName="ccToolsClassificationHelper::setScalarFieldValues";
    unsigned scalarFieldIndex=-1;
    QStringList scalarFields=getScalarFields();
    for(int i=0;i<scalarFields.size();i++)
    {
        if(scalarFields[i].compare(scalarFieldName,Qt::CaseInsensitive)==0)
        {
            scalarFieldIndex=i;
            break;
        }
    }
    if(scalarFieldIndex==-1)
    {
        QString strError=functionName;
        strError+=QObject::tr("\nNot found scalar field: %1").arg(scalarFieldIndex);
        return(false);
    }
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(scalarFieldIndex);
    if (!sf)
    {
        QString strError=functionName;
        strError+=QObject::tr("\nNot found scalar field: %1").arg(scalarFieldIndex);
                return(false);
    }
    // DHL 20221216
    if(values.size()!=m_ptrCloud->getPointSize())
    {
        QString strError=functionName;
        strError+=QObject::tr("\nInvalid values size for set in scalar field: %1").arg(scalarFieldIndex);
                return(false);
    }
//    values.clear();
//    values.resize(m_ptrCloud->getPointSize());
    // DHL 20221216
    int counter = 0;
    for (auto it = sf->begin(); it != sf->end(); ++it, ++counter)
    {
        ScalarType value=static_cast<ScalarType>(values[counter]);
        sf->setValue(counter,value);
    }
    return(true);
}

void ccToolsClassificationHelper::setScalarFieldIndex(int index)
{
	m_scalarFieldIndex = index;
}

void ccToolsClassificationHelper::keepCurrentSFVisible()
{
	m_formerCloudSFWasShown = true;
    m_ptrCloud->setCurrentDisplayedScalarField(m_scalarFieldIndex);
}

void ccToolsClassificationHelper::setVisible(bool value)
{
    unsigned pointCount = m_ptrCloud->size();
	for (unsigned i = 0; i < pointCount; ++i)
	{
        ccColor::Rgba color = m_ptrCloud->getPointColor(i);
		color.a = value ? ccColor::MAX : 0;
        m_ptrCloud->setPointColor(i, color);
	}

    m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::apply(QList<ccClassificationModel::Item>& items)
{
    if (m_scalarFieldIndex >= m_ptrCloud->getNumberOfScalarFields())
		return;

    m_ptrCloud->setColor(ccColor::black);

	for (int i = 0; i < items.size(); ++i)
	{
		items[i].count = apply(items[i]);
	}
	
    m_ptrCloud->redrawDisplay();
}

int ccToolsClassificationHelper::apply(ccClassificationModel::Item& item, bool redrawDisplay)
{
	ccColor::Rgba ccColor = ccColor::FromQColora(item.color);
	ccColor.a = item.visible ? ccColor::MAX : 0;
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
	if (!sf)
		return 0;

	ScalarType code = static_cast<ScalarType>(item.code);
	int affected = 0;
	int counter = 0;
	for (auto it = sf->begin(); it != sf->end(); ++it, ++counter)
	{
		if ((*it) == code)
		{
            if(item.rgb&&item.visible
                    &&counter<m_cloudOriginalRGB.size())
                ccColor=m_cloudOriginalRGB[counter];
            m_ptrCloud->setPointColor(counter, ccColor);
			++affected;
		}
	}

	if (redrawDisplay)
        m_ptrCloud->redrawDisplay();

	return affected;
}

void ccToolsClassificationHelper::changeCode(const ccClassificationModel::Item& item, ScalarType oldCode)
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
	if (!sf)
		return;

	ScalarType code = static_cast<ScalarType>(item.code);
	int counter = 0;
	for (auto it = sf->begin(); it != sf->end(); ++it, ++counter)
	{
		if ((*it) == oldCode)
		{
			sf->setValue(counter, code);
		}
	}
}

int ccToolsClassificationHelper::moveItem(const ccClassificationModel::Item& from, const ccClassificationModel::Item* to, bool redrawDisplay)
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
	if (!sf)
		return 0;

	ScalarType code = static_cast<ScalarType>(from.code);
	ScalarType emptyCode = to != nullptr ? static_cast<ScalarType>(to->code) : static_cast<ScalarType>(0);
	const ccColor::Rgba color = to != nullptr ? ccColor::FromQColora(to->color) : ccColor::black;

	int affected = 0;
	int counter = 0;
	for (auto it = sf->begin(); it != sf->end(); ++it, ++counter)
	{
		if ((*it) == code)
		{
			sf->setValue(counter, emptyCode);
            m_ptrCloud->setPointColor(counter, color);
			++affected;
		}
	}

	if (redrawDisplay)
        m_ptrCloud->redrawDisplay();

	return affected;
}

void ccToolsClassificationHelper::saveState()
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
	if (!sf)
		return;

    unsigned cloudSize = m_ptrCloud->size();
    m_cloudStateOld.resize(cloudSize);
    m_cloudStateNew.resize(cloudSize);
    for (unsigned i = 0; i < cloudSize; ++i)
	{
        m_cloudStateOld[i].update(sf->getValue(i), m_ptrCloud->getPointColor(i));
        m_cloudStateNew[i].update(sf->getValue(i), m_ptrCloud->getPointColor(i));
    }
}

void ccToolsClassificationHelper::redrawDisplay()
{
    m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::restoreState()
{
    CCCoreLib::ScalarField* sf = (m_ptrCloud ? m_ptrCloud->getScalarField(m_scalarFieldIndex) : nullptr);
	if (!sf)
	{
		assert(false);
		return;
	}

    unsigned cloudSize = m_ptrCloud->size();
    if (m_cloudStateOld.size() != cloudSize)
	{
		assert(false);
		return;
	}

	for (unsigned i = 0; i < cloudSize; ++i)
	{
        const CloudState& state = m_cloudStateOld[i];
		sf->setValue(i, state.code);
        m_ptrCloud->setPointColor(i, state.color);
	}
}

void ccToolsClassificationHelper::project(ccGLCameraParameters camera, unsigned start, unsigned end)
{
	const double half_w = camera.viewport[2] / 2.0;
	const double half_h = camera.viewport[3] / 2.0;

	CCVector3d Q2D;
	bool pointInFrustum = false;
	for (unsigned i = start; i < end; ++i)
	{
        const CCVector3* P3D = m_ptrCloud->getPoint(i);
		camera.project(*P3D, Q2D, &pointInFrustum);
		m_projectedPoints[i] = CCVector2(static_cast<PointCoordinateType>(Q2D.x - half_w), static_cast<PointCoordinateType>(Q2D.y - half_h));
		m_pointInFrustum[i] = pointInFrustum;
	}
}

PointCoordinateType ccToolsClassificationHelper::ComputeSquaredEuclideanDistance(const CCVector2& a, const CCVector2& b)
{
	return (b - a).norm2();
}

void ccToolsClassificationHelper::mouseMove(const CCVector2& center, float squareDist, std::map<ScalarType, int>& affected)
{
    if (m_parameters.ptrOutput == nullptr ||
        ((!m_parameters.anyPoints && !m_parameters.visiblePoints) && m_parameters.ptrInput == nullptr))
	{
		return;
	}

    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
	if (!sf)
		return;

    ScalarType inputCode = m_parameters.ptrInput != nullptr ? static_cast<ScalarType>(m_parameters.ptrInput->code) : 0;
    ScalarType outputCode = static_cast<ScalarType>(m_parameters.ptrOutput->code);

    unsigned char alpha = m_parameters.ptrOutput->visible ? ccColor::MAX : 0;
    ccColor::Rgba outputColor = ccColor::Rgba(ccColor::FromQColor(m_parameters.ptrOutput->color), alpha);

    unsigned cloudSize = m_ptrCloud->size();
	for (unsigned i = 0; i < cloudSize; ++i)
	{
		// skip camera outside point
		if (!m_pointInFrustum[i])
			continue;

        const auto& color = m_ptrCloud->getPointColor(i);

		// skip invisible points
		if (m_parameters.visiblePoints && color.a != ccColor::MAX)
			continue;

		ScalarType code = sf->getValue(i);

		// skip other codes
        if (m_parameters.ptrInput && code != inputCode)
			continue;

		// skip circle outside point
		if (ComputeSquaredEuclideanDistance(center, m_projectedPoints[i]) > squareDist)
			continue;
		
		if (code != outputCode)
		{
			sf->setValue(i, outputCode);
            m_ptrCloud->setPointColor(i, outputColor);

			--affected[code];
			++affected[outputCode];
		}
	
		m_modified = true;
	}
	
    m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::mouseMove(const CCVector2& center,
                                                       float squareDist,
                                                       std::map<ScalarType, int>& affected,
                                                       ccClassificationModel *ptrClassificationModel,
                                                       bool select,bool unselect, bool remove, bool recover)
{
//    if (m_parameters.output == nullptr ||
//        ((!m_parameters.anyPoints && !m_parameters.visiblePoints) && m_parameters.input == nullptr))
//    {
//        return;
//    }
    if (m_parameters.ptrOutput == nullptr)
    {
        return;
    }

    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;

//    ScalarType inputCode = m_parameters.input != nullptr ? static_cast<ScalarType>(m_parameters.input->code) : 0;
    ScalarType outputCode = static_cast<ScalarType>(m_parameters.ptrOutput->code);

    unsigned char outputAlpha = m_parameters.ptrOutput->visible ? ccColor::MAX : 0;
    ccColor::Rgba outputColor = ccColor::Rgba(ccColor::FromQColor(m_parameters.ptrOutput->color), outputAlpha);

//    ccClassificationModel::Item* ptrOutputItem=ptrClassificationModel->find(qRound(outputCode));
//    if(!ptrOutputItem) return;
    ScalarType selectedCode = static_cast<ScalarType>(ptrClassificationModel->getSelectedItem()->code);
    ScalarType removedCode = static_cast<ScalarType>(ptrClassificationModel->getRemovedItem()->code);

    unsigned cloudSize = m_ptrCloud->size();
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        // skip camera outside point
        if (!m_pointInFrustum[i])
            continue;

        const auto& color = m_ptrCloud->getPointColor(i);

        // No se pueden seleccionar puntos no visibles
        // skip invisible points
        if (m_parameters.visiblePoints && color.a != ccColor::MAX)
            continue;

        ScalarType code = sf->getValue(i);
        int intCode=qRound(code);
        ccClassificationModel::Item* ptrPointItem=ptrClassificationModel->find(intCode);
        if(ptrPointItem->locked)
        {
            continue;
        }

//        // skip other codes
//        if (m_parameters.input && code != inputCode)
//            continue;

        // skip circle outside point
        if (ComputeSquaredEuclideanDistance(center, m_projectedPoints[i]) > squareDist)
            continue;
        if(select||remove)
        {
            // removed points cant be selected
            if(select&&(qRound(code)==qRound(removedCode))) continue;
            // selected points cant be removed
            if(remove&&(qRound(code)==qRound(selectedCode))) continue;
            if (qRound(code) != qRound(outputCode))
            {
                sf->setValue(i, outputCode);
                m_ptrCloud->setPointColor(i, outputColor);
                --affected[code];
                ++affected[outputCode];
                if(select&&!m_selected) m_selected=true;
                if(remove&&!m_removed)
                {
                    m_removed=true;
                }
            }
        }
        else if(unselect||recover)
        {
            if(qRound(code) == qRound(outputCode))
            {
                const CloudState& state = m_cloudStateNew[i];
                sf->setValue(i, state.code);
                int stateCodeInt=qRound(state.code);
                ccClassificationModel::Item* item=ptrClassificationModel->find(stateCodeInt);
                if(item==nullptr) continue;
                sf->setValue(i, state.code);
                if(item->rgb&&i<m_cloudOriginalRGB.size())
                {
                    ccColor::Rgba ccColor=m_cloudOriginalRGB[i];
                    ccColor.a = item->visible ? ccColor::MAX : 0;
                    m_ptrCloud->setPointColor(i, ccColor);
                }
                else
                {
                    ccColor::Rgba ccColor=state.color;
                    ccColor.a = item->visible ? ccColor::MAX : 0;
                    m_ptrCloud->setPointColor(i, ccColor);
                }
//                m_ptrCloud->setPointColor(i, state.color);
                --affected[outputCode];
                ++affected[state.code];
                if(unselect&&affected[outputCode]==0) m_selected=false;
                if(recover&&affected[outputCode]==0) m_removed=false;
            }
        }
//        m_modified = true;
    }
    m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::pointsInPolygon(ccPolyline* m_segmentationPoly,
                                                      std::map<ScalarType, int> &affected,
                                                      ccClassificationModel *ptrClassificationModel,
                                                      bool select, bool unselect, bool remove, bool recover)
{
    if (m_parameters.ptrOutput == nullptr)
    {
        return;
    }

    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;

//    ScalarType inputCode = m_parameters.input != nullptr ? static_cast<ScalarType>(m_parameters.input->code) : 0;
    ScalarType outputCode = static_cast<ScalarType>(m_parameters.ptrOutput->code);

    unsigned char outputAlpha = m_parameters.ptrOutput->visible ? ccColor::MAX : 0;
    ccColor::Rgba outputColor = ccColor::Rgba(ccColor::FromQColor(m_parameters.ptrOutput->color), outputAlpha);

//    ccClassificationModel::Item* ptrOutputItem=ptrClassificationModel->find(qRound(outputCode));
//    if(!ptrOutputItem) return;
    ScalarType selectedCode = static_cast<ScalarType>(ptrClassificationModel->getSelectedItem()->code);
    ScalarType removedCode = static_cast<ScalarType>(ptrClassificationModel->getRemovedItem()->code);

    unsigned cloudSize = m_ptrCloud->size();
    const double half_w = m_cameraParameters.viewport[2] / 2.0;
    const double half_h = m_cameraParameters.viewport[3] / 2.0;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        // skip camera outside point
        if (!m_pointInFrustum[i])
            continue;

        const auto& color = m_ptrCloud->getPointColor(i);

        // No se pueden seleccionar puntos no visibles
        // skip invisible points
        if (m_parameters.visiblePoints && color.a != ccColor::MAX)
            continue;

        ScalarType code = sf->getValue(i);
        int intCode=qRound(code);
        ccClassificationModel::Item* ptrPointItem=ptrClassificationModel->find(intCode);
        if(ptrPointItem->locked)
        {
            continue;
        }

//        // skip other codes
//        if (m_parameters.input && code != inputCode)
//            continue;
        const CCVector3 *P3D = m_ptrCloud->getPoint(i);

        CCVector3d Q2D;
        bool pointInFrustum = false;
        m_cameraParameters.project(*P3D, Q2D, &pointInFrustum);
        CCVector2 P2D(	static_cast<PointCoordinateType>(Q2D.x - half_w),
                        static_cast<PointCoordinateType>(Q2D.y - half_h));
        bool pointInside=false;
        pointInside = CCCoreLib::ManualSegmentationTools::isPointInsidePoly(P2D, m_segmentationPoly);

        // skip circle outside point
        if (!pointInside)
            continue;
        if(select||remove)
        {
            // removed points cant be selected
            if(select&&(qRound(code)==qRound(removedCode))) continue;
            // selected points cant be removed
            if(remove&&(qRound(code)==qRound(selectedCode))) continue;
            if (qRound(code) != qRound(outputCode))
            {
                sf->setValue(i, outputCode);
                m_ptrCloud->setPointColor(i, outputColor);
                --affected[code];
                ++affected[outputCode];
                if(select&&!m_selected) m_selected=true;
                if(remove&&!m_removed)
                {
                    m_removed=true;
                }
            }
        }
        else if(unselect||recover)
        {
            if(qRound(code) == qRound(outputCode))
            {
                const CloudState& state = m_cloudStateNew[i];
                int stateCodeInt=qRound(state.code);
                ccClassificationModel::Item* item=ptrClassificationModel->find(stateCodeInt);
                if(item==nullptr) continue;
                sf->setValue(i, state.code);
                if(item->rgb&&i<m_cloudOriginalRGB.size())
                {
                    ccColor::Rgba ccColor=m_cloudOriginalRGB[i];
                    ccColor.a = item->visible ? ccColor::MAX : 0;
                    m_ptrCloud->setPointColor(i, ccColor);
                }
                else
                {
                    ccColor::Rgba ccColor=state.color;
                    ccColor.a = item->visible ? ccColor::MAX : 0;
                    m_ptrCloud->setPointColor(i, ccColor);
                }
//                m_ptrCloud->setPointColor(i, state.color);
                --affected[outputCode];
                ++affected[state.code];
                if(unselect&&affected[outputCode]==0) m_selected=false;
                if(recover&&affected[outputCode]==0) m_removed=false;
            }
        }
//        m_modified = true;
    }
    m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::removePoints(ccClassificationModel *ptrClassificationModel)
{
    if(!m_removed) return;
    ccClassificationModel::Item* removedItem = ptrClassificationModel->getRemovedItem();
    ScalarType removedCode = static_cast<ScalarType>(ptrClassificationModel->getRemovedItem()->code);
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;
    unsigned cloudSize = m_ptrCloud->size();
    unsigned numberOfPointsToRemove=removedItem->count;
    unsigned numberOfRemovedPoints=0;
    unsigned lastPointIndexNoRemoved=cloudSize-1;
    for (unsigned i = 0; i <=lastPointIndexNoRemoved; i++)
    {
        ScalarType code = sf->getValue(i);
        if(qRound(code)==qRound(removedCode))
        {
            bool control=false;
            while(!control)
            {
                ScalarType codeAux = sf->getValue(lastPointIndexNoRemoved);
                if(qRound(codeAux)==qRound(removedCode))
                {
                    lastPointIndexNoRemoved--;
                    numberOfRemovedPoints++;
                }
                else control=true;
            }
            numberOfRemovedPoints++;
            if(i==lastPointIndexNoRemoved) break;
            m_ptrCloudTools->swapPoints(i,lastPointIndexNoRemoved);
            lastPointIndexNoRemoved--;
        }
    }
    m_ptrCloud->unallocateVisibilityArray();
    m_ptrCloud->resize(cloudSize-numberOfRemovedPoints);
    m_ptrCloud->redrawDisplay();
    return;
}

void ccToolsClassificationHelper::toolChangeClass(std::map<ScalarType, int> &affected,
                                                             ccClassificationModel *ptrClassificationModel)
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;
    ScalarType outputCode = static_cast<ScalarType>(m_parameters.ptrOutput->code);
    unsigned char outputAlpha = m_parameters.ptrOutput->visible ? ccColor::MAX : 0;
    ccColor::Rgba outputColor = ccColor::Rgba(ccColor::FromQColor(m_parameters.ptrOutput->color), outputAlpha);
    ScalarType selectedCode = static_cast<ScalarType>(ptrClassificationModel->getSelectedItem()->code);
    unsigned cloudSize = m_ptrCloud->size();
    bool changes=false;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        // skip camera outside point
        if (!m_pointInFrustum[i])
            continue;

        const auto& color = m_ptrCloud->getPointColor(i);

//		// skip invisible points
//		if (m_parameters.visiblePoints && color.a != ccColor::MAX)
//			continue;

        ScalarType code = sf->getValue(i);
        if(qRound(code)!=qRound(selectedCode)) continue; // only change selected points

        // locked classes are not selected
        int intCode=qRound(code);
//        ccClassificationModel::Item* ptrPointItem=ptrClassificationModel->find(intCode);
//        if(ptrPointItem->locked)
//        {
//            continue;
//        }

        const CloudState& state = m_cloudStateNew[i];
        if(qRound(state.code)==qRound(outputCode)) continue; // point in target class
        sf->setValue(i, outputCode);

        int outputCodeInt=qRound(outputCode);
        ccClassificationModel::Item* item=ptrClassificationModel->find(outputCodeInt);
        if(item==nullptr) continue;
        if(item->rgb&&i<m_cloudOriginalRGB.size())
        {
            ccColor::Rgba ccColor=m_cloudOriginalRGB[i];
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
        else
        {
            ccColor::Rgba ccColor=outputColor;
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
//        m_ptrCloud->setPointColor(i, outputColor);

        m_cloudStateNew[i].code=outputCode;
        m_cloudStateNew[i].color=outputColor;
        --affected[selectedCode];
        ++affected[outputCode];
        if(affected[selectedCode]==0) m_selected=false;
        changes=true;
    }
    if(changes) m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::toolRecoverClass(std::map<ScalarType, int> &affected,
                                                              ccClassificationModel *ptrClassificationModel)
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;
    ScalarType selectedCode = static_cast<ScalarType>(ptrClassificationModel->getSelectedItem()->code);
    unsigned cloudSize = m_ptrCloud->size();
    bool changes=false;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        // skip camera outside point
        if (!m_pointInFrustum[i])
            continue;

        const auto& color = m_ptrCloud->getPointColor(i);

//		// skip invisible points
//		if (m_parameters.visiblePoints && color.a != ccColor::MAX)
//			continue;

        ScalarType code = sf->getValue(i);
        if(qRound(code)!=qRound(selectedCode)) continue; // only change selected points
        int stateCodeOldInt=qRound(m_cloudStateOld[i].code);
        ccClassificationModel::Item* item=ptrClassificationModel->find(stateCodeOldInt);
        if(item==nullptr) continue;
        sf->setValue(i, m_cloudStateOld[i].code);
        if(item->rgb&&i<m_cloudOriginalRGB.size())
        {
            ccColor::Rgba ccColor=m_cloudOriginalRGB[i];
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
        else
        {
            ccColor::Rgba ccColor= m_cloudStateOld[i].color;
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
//        m_ptrCloud->setPointColor(i, m_cloudStateOld[i].color);
        m_cloudStateNew[i].code=m_cloudStateOld[i].code;
        m_cloudStateNew[i].color=m_cloudStateOld[i].color;
        --affected[selectedCode];
        ++affected[m_cloudStateOld[i].code];
        if(affected[selectedCode]==0) m_selected=false;
        changes=true;
    }
    if(changes) m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::toolSelectOnly(std::map<ScalarType, int> &affected,
                                                        QList<int> &targetItemsCode,
                                                        ccClassificationModel *ptrClassificationModel)
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;
    ScalarType selectedCode = static_cast<ScalarType>(ptrClassificationModel->getSelectedItem()->code);
    unsigned cloudSize = m_ptrCloud->size();
    bool changes=false;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        // skip camera outside point
        if (!m_pointInFrustum[i])
            continue;

        const auto& color = m_ptrCloud->getPointColor(i);

//		// skip invisible points
//		if (m_parameters.visiblePoints && color.a != ccColor::MAX)
//			continue;

        ScalarType code = sf->getValue(i);
        if(qRound(code)!=qRound(selectedCode)) continue;
        const CloudState& state = m_cloudStateNew[i];

//        if(targetItemsCode.contains(qRound(state.code))) continue;
//        else
//        {
//            sf->setValue(i, state.code);
//            m_ptrCloud->setPointColor(i, state.color);
//            --affected[selectedCode];
//            ++affected[state.code];
//            if(affected[selectedCode]==0) m_selected=false;
//            changes=true;
//        }

        int stateCodeInt=qRound(state.code);
        if(!targetItemsCode.contains(stateCodeInt)) continue;
        ccClassificationModel::Item* item=ptrClassificationModel->find(stateCodeInt);
        if(item==nullptr) continue;
        sf->setValue(i, state.code);
        if(item->rgb&&i<m_cloudOriginalRGB.size())
        {
            ccColor::Rgba ccColor=m_cloudOriginalRGB[i];
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
        else
        {
            ccColor::Rgba ccColor=state.color;
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
        --affected[selectedCode];
        ++affected[state.code];
        if(affected[selectedCode]==0) m_selected=false;
        changes=true;

    }
    if(changes) m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::toolUnselect(std::map<ScalarType, int> &affected,
                                                      QList<int> &targetItemsCode,
                                                      ccClassificationModel *ptrClassificationModel)
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;
    ScalarType selectedCode = static_cast<ScalarType>(ptrClassificationModel->getSelectedItem()->code);
    unsigned cloudSize = m_ptrCloud->size();
    bool changes=false;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        // skip camera outside point
        if (!m_pointInFrustum[i])
            continue;

        const auto& color = m_ptrCloud->getPointColor(i);

//		// skip invisible points
//		if (m_parameters.visiblePoints && color.a != ccColor::MAX)
//			continue;

        ScalarType code = sf->getValue(i);
        if(qRound(code)!=qRound(selectedCode)) continue;
        const CloudState& state = m_cloudStateNew[i];
        int stateCodeInt=qRound(state.code);
        if(!targetItemsCode.contains(stateCodeInt)) continue;
        ccClassificationModel::Item* item=ptrClassificationModel->find(stateCodeInt);
        if(item==nullptr) continue;
        sf->setValue(i, state.code);
        if(item->rgb&&i<m_cloudOriginalRGB.size())
        {
            ccColor::Rgba ccColor=m_cloudOriginalRGB[i];
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
        else
        {
            ccColor::Rgba ccColor=state.color;
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
        --affected[selectedCode];
        ++affected[state.code];
        if(affected[selectedCode]==0) m_selected=false;
        changes=true;
    }
    if(changes) m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::toolRecover(std::map<ScalarType, int> &affected,
                                                         QList<int> &targetItemsCode,
                                                         ccClassificationModel *ptrClassificationModel)
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;
    ScalarType removedCode = static_cast<ScalarType>(ptrClassificationModel->getRemovedItem()->code);
    unsigned cloudSize = m_ptrCloud->size();
    bool changes=false;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        // skip camera outside point
        if (!m_pointInFrustum[i])
            continue;

        const auto& color = m_ptrCloud->getPointColor(i);

//		// skip invisible points
//		if (m_parameters.visiblePoints && color.a != ccColor::MAX)
//			continue;

        ScalarType code = sf->getValue(i);
        if(qRound(code)!=qRound(removedCode)) continue;
        const CloudState& state = m_cloudStateNew[i];
//        if(!targetItemsCode.contains(qRound(state.code))) continue;
//        else
//        {
//            sf->setValue(i, state.code);
//            m_ptrCloud->setPointColor(i, state.color);
//            --affected[removedCode];
//            ++affected[state.code];
//            if(affected[removedCode]==0) m_removed=false;
//            changes=true;
//        }
        int stateCodeInt=qRound(state.code);
        if(!targetItemsCode.contains(stateCodeInt)) continue;
        ccClassificationModel::Item* item=ptrClassificationModel->find(stateCodeInt);
        if(item==nullptr) continue;
        sf->setValue(i, state.code);
        if(item->rgb&&i<m_cloudOriginalRGB.size())
        {
            ccColor::Rgba ccColor=m_cloudOriginalRGB[i];
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
        else
        {
            ccColor::Rgba ccColor=state.color;
            ccColor.a = item->visible ? ccColor::MAX : 0;
            m_ptrCloud->setPointColor(i, ccColor);
        }
        --affected[removedCode];
        ++affected[state.code];
        if(affected[removedCode]==0) m_removed=false;
        changes=true;

    }
    if(changes) m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::toolRemove(std::map<ScalarType, int> &affected,
                                                        QList<int> &targetItemsCode,
                                                        ccClassificationModel *ptrClassificationModel)
{
    CCCoreLib::ScalarField* sf = m_ptrCloud->getScalarField(m_scalarFieldIndex);
    if (!sf)
        return;
    ScalarType selectedCode = static_cast<ScalarType>(ptrClassificationModel->getSelectedItem()->code);
    ScalarType removedCode = static_cast<ScalarType>(ptrClassificationModel->getRemovedItem()->code);
    ScalarType outputCode = static_cast<ScalarType>(m_parameters.ptrOutput->code);// == removedCode
    unsigned char outputAlpha = m_parameters.ptrOutput->visible ? ccColor::MAX : 0;
    ccColor::Rgba outputColor = ccColor::Rgba(ccColor::FromQColor(m_parameters.ptrOutput->color), outputAlpha);
    unsigned cloudSize = m_ptrCloud->size();
    bool changes=false;
    for (unsigned i = 0; i < cloudSize; ++i)
    {
        // skip camera outside point
        if (!m_pointInFrustum[i])
            continue;

        const auto& color = m_ptrCloud->getPointColor(i);

//		// skip invisible points
//		if (m_parameters.visiblePoints && color.a != ccColor::MAX)
//			continue;
        ScalarType code = sf->getValue(i);
        if(qRound(code)!=qRound(selectedCode)) continue;
        const CloudState& state = m_cloudStateNew[i];

        if(!targetItemsCode.contains(qRound(state.code))) continue;
        else
        {
            sf->setValue(i, outputCode);
            m_ptrCloud->setPointColor(i, outputColor);
            --affected[selectedCode];
            ++affected[outputCode];
            if(affected[selectedCode]==0) m_selected=false;
            if(affected[outputCode]==0) m_removed=false;
            changes=true;
        }

//        int stateCodeInt=qRound(state.code);
//        if(!targetItemsCode.contains(stateCodeInt)) continue;
//        ccClassificationModel::Item* item=ptrClassificationModel->find(stateCodeInt);
//        if(item==nullptr) continue;
//        sf->setValue(i, outputCode);
//        if(item->rgb&&i<m_cloudOriginalRGB.size())
//        {
//            ccColor::Rgba ccColor=m_cloudOriginalRGB[i];
//            ccColor.a = item->visible ? ccColor::MAX : 0;
//            m_ptrCloud->setPointColor(i, ccColor);
//        }
//        else
//        {
//            ccColor::Rgba ccColor=outputColor;
//            ccColor.a = item->visible ? ccColor::MAX : 0;
//            m_ptrCloud->setPointColor(i, ccColor);
//        }
//        --affected[selectedCode];
//        ++affected[outputColor];
//        if(affected[outputColor]==0) m_removed=false;
//        changes=true;

    }
    if(changes) m_ptrCloud->redrawDisplay();
}

void ccToolsClassificationHelper::projectCloud(const ccGLCameraParameters& camera)
{
	// check camera parameters changes
	bool hasChanges = false;
	auto a = m_cameraParameters.modelViewMat.data();
	auto b = camera.modelViewMat.data();
	for (int i = 0; i < OPENGL_MATRIX_SIZE; ++i)
	{
		if (std::abs(a[i] - b[i]) > 1e-6)
		{
			hasChanges = true;
			break;
		}
	}

	if (!hasChanges)
		return;

	m_cameraParameters = camera;
    unsigned cloudSize = m_ptrCloud->size();

	unsigned processorCount = std::thread::hardware_concurrency();
	if (processorCount == 0)
		processorCount = 1;

	const size_t part_size = cloudSize / processorCount;
	std::vector<std::thread*> threads;
	threads.resize(processorCount, nullptr);
	for (unsigned i = 0; i < processorCount; ++i)
	{
		size_t start = i * part_size;
		size_t end = start + part_size;

		if (i == processorCount - 1)
			end = cloudSize;

		threads[i] = new std::thread(&ccToolsClassificationHelper::project, this, camera, start, end);
	}

	for (auto it = threads.begin(); it != threads.end(); ++it)
		(*it)->join();

	for (auto it = threads.begin(); it != threads.end(); ++it)
		delete (*it);
}

ccToolsClassificationHelper::Parameters& ccToolsClassificationHelper::getParameters()
{
	return m_parameters;
}
