/*
 * JsonDataModel.hpp
 *
 *  Created on: 07/11/2014
 *      Author: Victor
 */

#ifndef JSONDATAMODEL_HPP_
#define JSONDATAMODEL_HPP_

#include <bb/cascades/QListDataModel>
#include <bb/data/JsonDataAccess>
#include <bb/system/InvokeManager>
#include <QtNetwork/QNetworkReply>

using namespace bb::data;
typedef bb::cascades::QListDataModel<QVariantMap> JsonListDataModel;
typedef QList<QVariantMap> QVariantMapList;

class JsonDataModel: public JsonListDataModel
{
Q_OBJECT

Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

public:

    JsonDataModel(QObject *parent = 0);
    virtual ~JsonDataModel();

    QString url();
    void setUrl(QString url);
    QString filter();
    void setFilter(QString filter);

    void setJsonData(QVariant data);
    void setIsFavorite(const QVariantList selectionList, const bool isFavorite);
    void deleteJsonItems(const QVariantList selectionList);
    void insertList(const QList<QVariantMap> &values);

Q_SIGNALS:
    void urlChanged(QString url);
    void filterChanged(QString filter);
    void replyCompleted(QString reply);

private Q_SLOTS:

    void onReplyFinished();

private:

    bool jsonToDataFolder();
    bool saveData();
    void applyFilter();
    void updateItemIsFavoriteAtIndex(QVariantList indexPath, const bool isFavorite);
    void deleteItemAtIndex(QVariantList indexPath);
    void getRequest();
    void findSettings();
    void parseJsonData(QVariant jsonData);

    QString getLocalTimeFromStation(QString time, QString mode="hh:mm");

    QString m_filter;
    QString m_url;
    QVariantMap m_settings;
    QVariantMapList m_originalList;
};


#endif /* JSONDATAMODEL_HPP_ */
