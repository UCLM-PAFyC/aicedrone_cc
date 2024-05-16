#pragma once

//##########################################################################
//#                                                                        #
//#                   CLOUDCOMPARE PLUGIN: qCloudLayers                    #
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
//#                     COPYRIGHT: WigginsTech 2022                        #
//#                                                                        #
//##########################################################################

#include "ccClassificationModel.h"

//CC
#include <CCTypes.h>
#include <CCGeom.h>
#include <ccColorTypes.h>
#include <ccGenericGLDisplay.h>
#include <ccPointCloud.h>

//QT
#include <QColor>

//std
#include <vector>

//class ccPointCloud;
class QStringList;
class ccMainAppInterface;
class RGBAColorsTableType;

class ccPointCloudTools:public ccPointCloud{
public:
    ccPointCloudTools();
    void swapPoints(unsigned firstIndex, unsigned secondIndex)
    {
        ccPointCloud::swapPoints(firstIndex,secondIndex);
    };
};

class ccToolsClassificationHelper
{
public:
    ccToolsClassificationHelper(ccMainAppInterface* app, ccPointCloud* cloud);
    ~ccToolsClassificationHelper();

	QStringList getScalarFields();
    bool getScalarFieldValues(QString scalarFieldName,
                              QVector<int>& values,
                              QString& strError);
    bool setScalarFieldValues(QString scalarFieldName,
                              QVector<int>& values,
                              QString& strError);
    void setScalarFieldIndex(int index);

	// set colors alpha to MAX
	void setVisible(bool value);

	// apply visibility and colors
	void apply(QList<ccClassificationModel::Item>& items);

	// apply visibility and color return affected count
	int apply(ccClassificationModel::Item& item, bool redrawDisplay = false);

	// asprs item code changed
	void changeCode(const ccClassificationModel::Item& item, ScalarType oldCode);

	// set scalar code to zero return affected count
	int moveItem(const ccClassificationModel::Item& from, const ccClassificationModel::Item* to, bool redrawDisplay = false);

	// save color and codes
	void saveState();

    void redrawDisplay();

	// restore initial colors and codes
	void restoreState();

	void mouseMove(const CCVector2& center, float squareDist, std::map<ScalarType, int>& affected);
    void mouseMove(const CCVector2& center,
                   float squareDist,
                   std::map<ScalarType, int>& affected,
                   ccClassificationModel* ptrClassificationModel,
                   bool select,bool unselect,bool remove,bool recover);
    void pointsInPolygon(ccPolyline *m_segmentationPoly,
                   std::map<ScalarType, int>& affected,
                   ccClassificationModel* ptrClassificationModel,
                   bool select, bool unselect, bool remove, bool recover);
    void removePoints(ccClassificationModel *ptrClassificationModel);
    void toolChangeClass(std::map<ScalarType, int>& affected,
                    ccClassificationModel* ptrClassificationModel);
    void toolRecoverClass(std::map<ScalarType, int>& affected,
                    ccClassificationModel* ptrClassificationModel);
    void toolSelectOnly(std::map<ScalarType, int>& affected,
                    QList<int> &targetItemsCode,
                    ccClassificationModel* ptrClassificationModel);
    void toolUnselect(std::map<ScalarType, int>& affected,
                  QList<int> &targetItemsCode,
                  ccClassificationModel* ptrClassificationModel);
    void toolRecover(std::map<ScalarType, int>& affected,
                    QList<int> &targetItemsCode,
                    ccClassificationModel* ptrClassificationModel);
    void toolRemove(std::map<ScalarType, int>& affected,
                    QList<int> &targetItemsCode,
                    ccClassificationModel* ptrClassificationModel);
    void projectCloud(const ccGLCameraParameters& camera);
	bool hasChanges() const { return m_modified; }

	struct Parameters
	{
		bool anyPoints = false;
		bool visiblePoints = false;
        ccClassificationModel::Item* ptrInput = nullptr;
        ccClassificationModel::Item* ptrOutput = nullptr;
	};

	Parameters& getParameters();

    inline ccPointCloud* cloud() { return m_ptrCloud; }

	void keepCurrentSFVisible();

private: // methods
	void project(ccGLCameraParameters camera, unsigned start, unsigned end);
	static PointCoordinateType ComputeSquaredEuclideanDistance(const CCVector2& a, const CCVector2& b);

private: // variables
    ccMainAppInterface* m_ptrApp;
    ccPointCloudTools* m_ptrCloudTools;
    ccPointCloud* m_ptrCloud;
    RGBAColorsTableType* m_ptrFormerCloudColors;
	bool m_formerCloudColorsWereShown;
	bool m_formerCloudSFWasShown;
	Parameters m_parameters;

	unsigned m_scalarFieldIndex;
	bool m_modified;
    bool m_selected;
    bool m_removed;

	ccGLCameraParameters m_cameraParameters;
	std::vector<CCVector2> m_projectedPoints;
	std::vector<bool> m_pointInFrustum;

	struct CloudState
	{
	public:
		CloudState() {}

		void update(ScalarType code, ccColor::Rgb color)
		{
			this->code = code;
			this->color = color;
		}

		ScalarType code;
		ccColor::Rgb color;
	};

    std::vector<CloudState> m_cloudStateOld;
    std::vector<CloudState> m_cloudStateNew;

};

