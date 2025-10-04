/*
 * JsonItemProvider.cpp
 *
 *  Created on: 09/11/2014
 *      Author: Victor
 */

#include "JsonItem.hpp"
#include "JsonItemProvider.hpp"

JsonItemProvider::JsonItemProvider(QObject *parent){
    setParent(parent);
}
VisualNode * JsonItemProvider::createItem(ListView* list, const QString &type){
    Q_UNUSED(type);
    Q_UNUSED(list);
    qDebug() << type;
    JsonItem *item = new JsonItem();
    return item;
}

void JsonItemProvider::updateItem(ListView* list, VisualNode *listItem, const QString &type,
            const QVariantList &indexPath, const QVariant &data){
    Q_UNUSED(list);
    Q_UNUSED(indexPath);
    Q_UNUSED(type);

    // Update the control with correct data
    QVariantMap map = data.value<QVariantMap>();
    JsonItem *item = static_cast<JsonItem *>(listItem);
    QString s(map["name"].toString());
    item->updateItem(s);


}
JsonItemProvider::~JsonItemProvider()
{
}

