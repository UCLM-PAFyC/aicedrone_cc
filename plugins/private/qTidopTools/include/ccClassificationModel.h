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

//Qt
#include <QAbstractTableModel>
#include <QColor>
#include <QMap>
#include "qToolsDefinitions.h"

class ccClassificationModel : public QAbstractTableModel
{
	Q_OBJECT

public:
    ccClassificationModel(QString classificationModelName,
                          QObject* parent = nullptr);
    int getNoiseCode(){return(m_noiseCode);};
    int getTrainColumn(){return(6);}; // linked to enum Column and item

	int rowCount(const QModelIndex& parent) const;
	int columnCount(const QModelIndex& parent) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	Qt::ItemFlags flags(const QModelIndex& index) const;

	QModelIndex createNewItem();

	// load asprs items data from qsettings
    void load();

	// save asprs items data to qsettings
    void save() const;

	enum Column
	{
		VISIBLE,
		NAME,
		CODE,
		COLOR,
		RGB,
		COUNT,
        LOCKED,
        TRAIN,
		LAST
	};

    struct Item
	{
		bool visible;
		QString name;
		int code;
		QColor color;
		bool rgb;
		int count;
        bool locked;
        bool train;
	};

	void refreshData();
	
    inline const QList<Item>& getData() const { return m_data; }
    inline QList<Item>& getData() { return m_data; }
    QMap<int,QString>& getDataTags();

    Item* find(QString name);
    Item* find(int code);
    Item* findByTag(QString tag);
    Item* getSelectedItem();
    Item* getRemovedItem();
    Item* getNotClassifiedItem();

	int indexOf(QString name) const;

    void setAllVisible();
    void setAllInvisible();
    void setAllLocked();
    void setAllUnlocked();

public Q_SLOTS:
	bool removeRows(int position, int rows, const QModelIndex& parent);
    bool updateTags();

Q_SIGNALS:
    void codeChanged(Item& item, int oldCode);
    void colorChanged(Item& item);

private:
    QList<Item> m_data;
    int m_removedIndexCode;
    int m_selectedIndexCode;
    int m_notClassifiedIndexCode;
    QMap<int,QString> m_tagByCode; // "1 - Unclassified"
    int m_noiseCode;
    QString m_modelName;

private:
	bool isNameExist(const QString& name) const;
	bool isCodeExist(int code) const;

	// default asprs items
    void createDefaultItems();
    int getUnusedCode() const;
};

