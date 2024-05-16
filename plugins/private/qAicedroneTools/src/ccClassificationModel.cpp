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

#include "../include/ccClassificationModel.h"

//QT
#include <QSettings>

ccClassificationModel::ccClassificationModel(QString classificationModelName,
                                             QObject* parent)
	: QAbstractTableModel(parent)
    , m_modelName(classificationModelName)
{
	load();

    // to change values
    createDefaultItems();

	if (m_data.size() == 0)
	{
		createDefaultItems();
	}

    m_removedIndexCode=CC_CLASSIFICATION_MODEL_REMOVED_CODE;
    m_selectedIndexCode=CC_CLASSIFICATION_MODEL_SELECTED_CODE;
    m_noiseCode=CC_CLASSIFICATION_MODEL_ASPRS_NOISE_CODE;
}

int ccClassificationModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return m_data.size();
}

int ccClassificationModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return LAST;
}

QVariant ccClassificationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return {};

	if (orientation == Qt::Vertical)
		return section;

	switch (section)
	{
	case VISIBLE:
		return "Visible";
	case NAME:
		return "Name";
	case CODE:
		return "Code";
	case COLOR:
		return "Color";
	case COUNT:
		return "Count";
    case LOCKED:
        return "Locked";
    case TRAIN:
        return "Train";
    default:
		assert(false);
		break;
	}

	return {};
}

QVariant ccClassificationModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return {};

    const Item& item = m_data[index.row()];
    QString itemName = item.name;

	// specific case for the VISIBLE column
    if (index.column() == VISIBLE)
	{
		// we only provide the value for the 'CheckStateRole' role
		if (role == Qt::CheckStateRole)
		{
			return item.visible ? Qt::Checked : Qt::Unchecked;
		}
		else
		{
			return {};
		}
	}
    if (index.column() == LOCKED)
    {
        // we only provide the value for the 'CheckStateRole' role
        if (role == Qt::CheckStateRole)
        {
            return item.locked ? Qt::Checked : Qt::Unchecked;
        }
        else
        {
            return {};
        }
    }
    if (index.column() == TRAIN)
    {
        // we only provide the value for the 'CheckStateRole' role
        if (role == Qt::CheckStateRole)
        {
            return item.train ? Qt::Checked : Qt::Unchecked;
        }
        else
        {
            return {};
        }
    }

	// for the others, we only provide the values for the 'Display' and 'Edit' roles
	if (role != Qt::DisplayRole && role != Qt::EditRole)
	{
		return {};
	}

	switch (index.column())
	{
    case VISIBLE:
        return item.visible;
    case NAME:
		return item.name;
	case CODE:
		return item.code;
	case COLOR:
		return item.color;
	case COUNT:
		return item.count;
    case LOCKED:
        return item.locked;
    case TRAIN:
        return item.train;
    default:
		assert(false);
		break;
	}

	return {};
}

bool ccClassificationModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	Q_UNUSED(role)

	if (!index.isValid())
	{
		return false;
	}

    Item& item = m_data[index.row()];
    QString name=item.name;

	switch (index.column())
	{
	case VISIBLE:
	{
        if(name.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,Qt::CaseInsensitive)==0
                ||name.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,Qt::CaseInsensitive)==0)
        {
            return false;
        }
		if (role == Qt::CheckStateRole)
		{
			item.visible = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
			emit colorChanged(item);
		}
		else
		{
			return false;
		}
	}
	break;

	case NAME:
	{
        if(item.name.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,Qt::CaseInsensitive)==0)
        {
            return false;
        }
        if(item.name.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,Qt::CaseInsensitive)==0)
        {
            return false;
        }
        QString name = value.toString();
		if (!isNameExist(name))
		{
			item.name = name;
			break;
		}
		else
		{
			return false;
		}
	}

	case CODE:
	{
		int code = value.toInt();
		if (!isCodeExist(code))
		{
			int oldCode = item.code;
			item.code = code;
            if(oldCode == m_removedIndexCode) m_removedIndexCode=code;
            if(oldCode == m_selectedIndexCode) m_selectedIndexCode=code;
            emit codeChanged(item, oldCode);
			break;
		}
		else
		{
			return false;
		}
	}

	case COLOR:
	{
		item.color = value.value<QColor>();
		emit colorChanged(item);
	}
	break;

	case COUNT:
	{
		item.count = value.toInt();
	}
    break;

    case LOCKED:
    {
        if(name.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,Qt::CaseInsensitive)==0
                ||name.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,Qt::CaseInsensitive)==0)
        {
            return false;
        }
        if (role == Qt::CheckStateRole)
        {
            item.locked = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
            emit colorChanged(item);
        }
        else
        {
            return false;
        }
    }
    break;

    case TRAIN:
    {
        if(name.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,Qt::CaseInsensitive)==0
                ||name.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,Qt::CaseInsensitive)==0)
        {
            return false;
        }
        if (role == Qt::CheckStateRole)
        {
            item.train = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
            emit colorChanged(item);
        }
        else
        {
            return false;
        }
    }
    break;
    }

	emit dataChanged(index, index);

	return true;
}

Qt::ItemFlags ccClassificationModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return Qt::ItemIsEnabled;
	}

	Qt::ItemFlags f = QAbstractItemModel::flags(index);
	if (index.column() == NAME || index.column() == CODE)
	{
		f |= (Qt::ItemIsEditable);
	}
	else if (index.column() == VISIBLE)
	{
		f |= (Qt::ItemIsUserCheckable);
	}
    else if (index.column() == LOCKED)
    {
        f |= (Qt::ItemIsUserCheckable);
    }
    else if (index.column() == TRAIN)
    {
        f |= (Qt::ItemIsUserCheckable);
    }

	return f;
}

QModelIndex ccClassificationModel::createNewItem()
{
	const int rowNumber = m_data.size();
	beginInsertRows(QModelIndex(), rowNumber, rowNumber);
	m_data.append({ false, "UNNAMED", getUnusedCode(), Qt::GlobalColor::black, 0 });
	endInsertRows();
	return createIndex(rowNumber, NAME);
}

bool ccClassificationModel::isNameExist(const QString& name) const
{
    auto item = std::find_if(m_data.begin(), m_data.end(), [name](const Item& item) { return item.name == name; });
	return item != m_data.end();
}

bool ccClassificationModel::isCodeExist(int code) const
{
    auto item = std::find_if(m_data.begin(), m_data.end(), [code](const Item& item) { return item.code == code; });
	return item != m_data.end();
}

static void AddClass(QSettings& settings, const QString& className, int classValue, QColor classColor, bool visible = true)
{
	QString cleanClassName = className;
	cleanClassName.replace(QChar('/'), QChar('@'));

	settings.beginGroup(cleanClassName);
	{
		settings.setValue("class", classValue);
		settings.setValue("color", classColor.rgb());
		settings.setValue("visible", visible);
	}
	settings.endGroup();
}

static void ReadClass(const QSettings& settings, const QString& className, ccClassificationModel::Item& item)
{
	QString cleanClassName = className;
	cleanClassName.replace(QChar('/'), QChar('@'));

	QString readableClassName = className;
	readableClassName.replace(QChar('@'), QChar('/'));

	item.name = readableClassName;
	item.code = settings.value(cleanClassName + "/class", 0).toInt();
	item.color = QColor(settings.value(cleanClassName + "/color", 0).toUInt());
	item.visible = settings.value(cleanClassName + "/visible", true).toBool();
	item.count = 0;
}

void ccClassificationModel::createDefaultItems()
{
	QSettings settings;
    if(m_modelName.compare(CC_CLASSIFICATION_MODEL_ASPRS_NAME,
                         Qt::CaseInsensitive)==0)
    {
        settings.beginGroup(CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_ASPRS);
        {
            AddClass(settings, "Removed",				        CC_CLASSIFICATION_MODEL_REMOVED_CODE, qRgb(255, 255, 255));
            AddClass(settings, "Selected",				        CC_CLASSIFICATION_MODEL_SELECTED_CODE, qRgb(255, 255, 0));
            AddClass(settings, "Not classified",				 0, qRgb(210, 210, 210));
            AddClass(settings, "Unclassified",					 1, qRgb(170, 170, 170));
            AddClass(settings, "Ground",						 2, qRgb(166, 116, 4));
            AddClass(settings, "Low vegetation",				 3, qRgb(204, 240, 123));
            AddClass(settings, "Medium vegetation",				 4, qRgb(38, 114, 0));
            AddClass(settings, "High vegetation",				 5, qRgb(69, 229, 0));
            AddClass(settings, "Building",						 6, qRgb(255, 0, 0));
            AddClass(settings, "Low Noise",						 7, qRgb(120, 0, 120));
            AddClass(settings, "Model Keypoint",				 8, qRgb(255, 0, 255));
            AddClass(settings, "Water",							 9, qRgb(0, 0, 255));
            AddClass(settings, "Rail",							10, qRgb(80, 80, 80));
            AddClass(settings, "Road surface",					11, qRgb(120, 120, 120));
            AddClass(settings, "Overlap",						12, qRgb(255, 170, 255));
            AddClass(settings, "Wire Shield/Neutral/Com",		13, qRgb(191, 231, 205));
            AddClass(settings, "Wire Conductors/Phases",		14, qRgb(193, 230, 125));
            AddClass(settings, "Transmission Tower",			15, Qt::darkBlue);
            AddClass(settings, "Wire Insulators",				16, Qt::darkYellow);
            AddClass(settings, "Bridge Deck",					17, Qt::darkCyan);
            AddClass(settings, "High Noise",					18, Qt::darkRed);

            AddClass(settings, "Conductor Attachment Points",	64, qRgb(25, 0, 51));
            AddClass(settings, "Shield Attachment Points",		65, qRgb(51, 0, 102));
            AddClass(settings, "Midspan Points",				66, qRgb(76, 0, 153));
            AddClass(settings, "Structure Top Points",			67, qRgb(102, 0, 204));
            AddClass(settings, "Structure Bottom Points",		68, qRgb(127, 0, 255));

            AddClass(settings, "Guy Wire",						70, qRgb(153, 51, 255));
            AddClass(settings, "Substation",					75, qRgb(178, 102, 255));

            AddClass(settings, "Misc Temporary",				81, qRgb(204, 153, 255));
            AddClass(settings, "Misc Permanent",				82, qRgb(229, 204, 255));
            AddClass(settings, "Misc Fences",					83, qRgb(204, 204, 255));
        }
        settings.endGroup();
        m_noiseCode=CC_CLASSIFICATION_MODEL_ASPRS_NOISE_CODE;
    }
//    if(m_modelName.compare(CC_CLASSIFICATION_MODEL_ARCHDATASET_NAME,
//                         Qt::CaseInsensitive)==0)
//    {
//        settings.beginGroup(CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_ARCHDATASET);
//        {
//            AddClass(settings, "Removed",				        CC_CLASSIFICATION_MODEL_REMOVED_CODE, qRgb(255, 255, 255));
//            AddClass(settings, "Selected",				        CC_CLASSIFICATION_MODEL_SELECTED_CODE, qRgb(255, 255, 0));
//            AddClass(settings, "Not classified",				 CC_CLASSIFICATION_MODEL_ARCHDATASET_NOT_CLASSIFIED_CODE, qRgb(210, 210, 210));
//            AddClass(settings, "Unclassified",					 CC_CLASSIFICATION_MODEL_ARCHDATASET_UNCLASSIFIED_CODE, qRgb(170, 170, 170));
//            AddClass(settings, "Arch",				             0, qRgb(17,70,244));
//            AddClass(settings, "Column",					     1, qRgb(255,0,0));
//            AddClass(settings, "Moldings",						 2, qRgb(187,20,206));
//            AddClass(settings, "Floor",				             3, qRgb(90, 44, 51));
//            AddClass(settings, "Window/Door",				     4, qRgb(180, 180, 0));
//            AddClass(settings, "Wall",				             5, qRgb(184, 163, 153));
//            AddClass(settings, "Stair",						     6, qRgb(27,226,31));
//            AddClass(settings, "Vault",						     7, qRgb(255,123,0));
//            AddClass(settings, "Roof",				             8, qRgb(172, 30, 32));
//            AddClass(settings, "Other",							 9, qRgb(9,245,241));
//        }
//        settings.endGroup();
//        m_noiseCode=CC_CLASSIFICATION_MODEL_ARCHDATASET_NOISE_CODE;
//    }
    if(m_modelName.compare(CC_CLASSIFICATION_MODEL_BREAKWATER_CUBES_NAME,
                           Qt::CaseInsensitive)==0)
    {
        settings.beginGroup(CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_BREAKWATER_CUBES);
        {
            AddClass(settings, "Removed",				        CC_CLASSIFICATION_MODEL_REMOVED_CODE, qRgb(255, 255, 255));
            AddClass(settings, "Selected",				        CC_CLASSIFICATION_MODEL_SELECTED_CODE, qRgb(255, 255, 0));
            AddClass(settings, "Not classified",				 CC_CLASSIFICATION_MODEL_NO_ASPRS_NOT_CLASSIFIED_CODE, qRgb(210, 210, 210));
            AddClass(settings, "Unclassified",					 CC_CLASSIFICATION_MODEL_NO_ASPRS_UNCLASSIFIED_CODE, qRgb(170, 170, 170));
            AddClass(settings, "CubeCorner",				     0, qRgb(0,255,0));
            AddClass(settings, "CubeEdge",					     1, qRgb(255,0,0));
            AddClass(settings, "CubeFace",						 2, qRgb(166, 116, 4));
            AddClass(settings, "Other",							 4, qRgb(9,245,241));
        }
        settings.endGroup();
        m_noiseCode=CC_CLASSIFICATION_MODEL_NO_ASPRS_NOISE_CODE;
    }
    if(m_modelName.compare(CC_CLASSIFICATION_MODEL_RAILWAY_NAME,
                         Qt::CaseInsensitive)==0)
    {
        settings.beginGroup(CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_RAILWAY);
        {
            AddClass(settings, "Removed",				        CC_CLASSIFICATION_MODEL_REMOVED_CODE, qRgb(255, 255, 255));
            AddClass(settings, "Selected",				        CC_CLASSIFICATION_MODEL_SELECTED_CODE, qRgb(0, 255, 255));
            AddClass(settings, "Not classified",				 CC_CLASSIFICATION_MODEL_NO_ASPRS_NOT_CLASSIFIED_CODE, qRgb(255, 0, 255));
            AddClass(settings, "Unclassified",					 CC_CLASSIFICATION_MODEL_NO_ASPRS_UNCLASSIFIED_CODE, qRgb(170, 170, 170));
//            AddClass(settings, "Floor",				             0, qRgb(200,130,55));
            AddClass(settings, "Ground",					     1, qRgb(200,150,150));
            AddClass(settings, "Railway",					     2, qRgb(200,115,50));
            AddClass(settings, "Rail",				             3, qRgb(250,0,0));
            AddClass(settings, "Wire",				             4, qRgb(0,0,255));
            AddClass(settings, "Tower",				             5, qRgb(255,255,50));
            AddClass(settings, "TowerCatenary",				     6, qRgb(0,0,255));
            AddClass(settings, "Building",						 7, qRgb(255,150,0));
            AddClass(settings, "Ballast",				         8, qRgb(150,150,150));
            AddClass(settings, "Tree",							 9, qRgb(0,255,0));
        }
        settings.endGroup();
        m_noiseCode=CC_CLASSIFICATION_MODEL_NO_ASPRS_NOISE_CODE;
    }
    settings.sync();
	load();
}

void ccClassificationModel::load()
{
	QSettings settings;
    QString persistenceTag;
    if(m_modelName.compare(CC_CLASSIFICATION_MODEL_ASPRS_NAME,
                           Qt::CaseInsensitive)==0)
    {
        persistenceTag=CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_ASPRS;
    }
    else if(m_modelName.compare(CC_CLASSIFICATION_MODEL_BREAKWATER_CUBES_NAME,
                                Qt::CaseInsensitive)==0)
    {
        persistenceTag=CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_BREAKWATER_CUBES;
    }
    else if(m_modelName.compare(CC_CLASSIFICATION_MODEL_RAILWAY_NAME,
                                Qt::CaseInsensitive)==0)
    {
        persistenceTag=CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_RAILWAY;
    }
    settings.beginGroup(persistenceTag);

	QStringList classes = settings.childGroups();

	m_data.clear();
	m_data.reserve(classes.size());
	for (int i = 0; i < classes.length(); ++i)
	{
        ccClassificationModel::Item item;
		ReadClass(settings, classes[i], item);
        item.visible=true;
        item.locked=false;
        item.train=false;
		m_data.append(item);
        if(item.name.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,Qt::CaseInsensitive)==0)
        {
            m_removedIndexCode=item.code;
        }
        if(item.name.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,Qt::CaseInsensitive)==0)
        {
            m_selectedIndexCode=item.code;
        }
    }
}

void ccClassificationModel::save() const
{
	QSettings settings;
    QString persistenceTag;
    if(m_modelName.compare(CC_CLASSIFICATION_MODEL_ASPRS_NAME,
                         Qt::CaseInsensitive)==0)
    {
        persistenceTag=CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_ASPRS;
    }
    else if(m_modelName.compare(CC_CLASSIFICATION_MODEL_BREAKWATER_CUBES_NAME,
                         Qt::CaseInsensitive)==0)
    {
        persistenceTag=CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_BREAKWATER_CUBES;
    }
    else if(m_modelName.compare(CC_CLASSIFICATION_MODEL_RAILWAY_NAME,
                         Qt::CaseInsensitive)==0)
    {
        persistenceTag=CC_CLASSIFICATION_MODEL_PERSISTENCE_TAG_RAILWAY;
    }
    settings.remove(persistenceTag);
    settings.beginGroup(persistenceTag);
	{
		for (int i = 0; i < m_data.length(); ++i)
		{
            const Item& item = m_data[i];
			AddClass(settings, item.name, item.code, item.color, item.visible);
		}
	}
	settings.endGroup();
	settings.sync();
}

void ccClassificationModel::refreshData()
{
	QModelIndex a = createIndex(0, COUNT);
	QModelIndex b = createIndex(m_data.count() - 1, COUNT);
    emit dataChanged(a, b);
}

QMap<int,QString> &ccClassificationModel::getDataTags()
{
    updateTags();
    return(m_tagByCode);
}

bool ccClassificationModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	Q_UNUSED(parent);

	beginRemoveRows(QModelIndex(), position, position + rows - 1);
	for (int i = 0; i < rows; ++i)
	{
		m_data.removeAt(position);
	}
	endRemoveRows();

    return true;
}

bool ccClassificationModel::updateTags()
{
    m_tagByCode.clear(); // "1 - Unclassified"
    for (int i = 0; i < m_data.size(); ++i)
    {
        QString itemName=m_data[i].name;
        if(itemName.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,Qt::CaseInsensitive)!=0
                &&itemName.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,Qt::CaseInsensitive)!=0)
        {
            int code=m_data[i].code;
            QString tag=QString::number(code)+CC_CLASSIFICATION_MODEL_TAG_STRING_SEPARATOR+itemName;
            m_tagByCode[code]=tag;
        }
    }
    return(true);
}

ccClassificationModel::Item* ccClassificationModel::find(QString name)
{
    auto it = std::find_if(m_data.begin(), m_data.end(), [name](const ccClassificationModel::Item &item) { return item.name == name; });
	return it != m_data.end() ? &(*it) : nullptr;
}

ccClassificationModel::Item* ccClassificationModel::find(int code)
{
    auto it = std::find_if(m_data.begin(), m_data.end(), [code](const ccClassificationModel::Item &item) { return item.code == code; });
	return it != m_data.end() ? &(*it) : nullptr;
}

ccClassificationModel::Item* ccClassificationModel::findByTag(QString tag)
{
    QStringList values=tag.split(CC_CLASSIFICATION_MODEL_TAG_STRING_SEPARATOR);
    if(values.size()!=2) return(NULL);
    bool okToInt=false;
    int code=values[0].trimmed().toInt(&okToInt);
    if(!okToInt) return(NULL);
    return(find(code));
}

ccClassificationModel::Item* ccClassificationModel::getSelectedItem()
{
    return(find(m_selectedIndexCode));
}

ccClassificationModel::Item* ccClassificationModel::getRemovedItem()
{
    return(find(m_removedIndexCode));
}

int ccClassificationModel::indexOf(QString name) const
{
    auto it = std::find_if(m_data.begin(), m_data.end(), [name](const ccClassificationModel::Item &item) { return item.name == name; });
    return it != m_data.end() ? it - m_data.begin() : -1;
}

void ccClassificationModel::setAllVisible()
{
    for (int i = 0; i < m_data.length(); ++i)
    {
        Item& item = m_data[i];
        item.visible=true;
//        m_ptrHelper->apply(item, true);
//        emit colorChanged(item);
    }
//    refreshData();
}

void ccClassificationModel::setAllInvisible()
{
    for (int i = 0; i < m_data.length(); ++i)
    {
        Item& item = m_data[i];
        QString itemName = item.name;
        if(itemName.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,
                            Qt::CaseInsensitive)==0
                ||itemName.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,
                                   Qt::CaseInsensitive)==0)
        {
            item.visible=true;
        }
        else item.visible=false;
    }
}

void ccClassificationModel::setAllLocked()
{
    for (int i = 0; i < m_data.length(); ++i)
    {
        Item& item = m_data[i];
        QString itemName = item.name;
        if(itemName.compare(CC_CLASSIFICATION_MODEL_REMOVED_NAME,
                            Qt::CaseInsensitive)==0
                ||itemName.compare(CC_CLASSIFICATION_MODEL_SELECTED_NAME,
                                   Qt::CaseInsensitive)==0)
        {
            item.locked=false;
        }
        else item.locked=true;
    }
}

void ccClassificationModel::setAllUnlocked()
{
    for (int i = 0; i < m_data.length(); ++i)
    {
        Item& item = m_data[i];
        item.locked=false;
    }
}

int ccClassificationModel::getUnusedCode() const
{
    auto it = std::max_element(m_data.cbegin(), m_data.cend(), [](const Item &a, const Item &b) { return a.code < b.code; });
	return it != m_data.end() ? (*it).code + 1 : 0;
}
