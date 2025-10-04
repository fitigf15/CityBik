/*
 * Copyright (c) 2011-2014 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>
#include <QVariant>
#include <bb/cascades/GroupDataModel>
#include <bb/cascades/Image>
#include <bb/cascades/Dialog>
#include <bb/cascades/Sheet>
namespace bb
{
    namespace cascades
    {
        class LocaleHandler;
        class Container;
        class Image;
        class TabbedPane;
        class Dialog;
        class GroupDataModel;
        class Sheet;
        namespace maps {
            class MapView;
            class MapImageGenerator;
        }
    }
    namespace platform {
        namespace geo {
            class GeoLocation;
        }
    }
}

class QTranslator;

/*!
 * @brief Application UI object
 *
 * Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class Bicing : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap currentNetwork READ currentNetwork WRITE setCurrentNetwork NOTIFY currentNetworkChanged)
    Q_PROPERTY(QVariantMap currentStation READ currentStation WRITE setCurrentStation NOTIFY currentStationChanged)
    Q_PROPERTY(QVariantList currentNearbyStations READ currentNearbyStations WRITE setCurrentNearbyStations NOTIFY currentNearbyStationsChanged)
    Q_PROPERTY(QVariantList currentNetworkStations READ currentNetworkStations WRITE setCurrentNetworkStations NOTIFY currentNetworkStationsChanged)
    Q_PROPERTY(QVariantMap nets READ nets WRITE setNets NOTIFY netsChanged)
    Q_PROPERTY(bb::cascades::Image staticMapImage READ staticMapImage WRITE setStaticMapImage NOTIFY staticMapImageChanged)
    Q_PROPERTY(bb::cascades::GroupDataModel* currentStationsDataModel READ currentStationsDataModel WRITE setCurrentStationsDataModel NOTIFY currentStationsDataModelChanged)
    Q_PROPERTY(bb::cascades::GroupDataModel* currentNearbyStationsDataModel READ currentNearbyStationsDataModel WRITE setCurrentNearbyStationsDataModel NOTIFY currentNearbyStationsDataModelChanged)
public:
    Bicing();
    Q_INVOKABLE void updateDeviceLocation(double lat, double lon);
    Q_INVOKABLE void locationTapped(QString id);
    Q_INVOKABLE void updateMap();
    Q_INVOKABLE QVariantList worldToPixelInvokable(QObject* mapObject, double latitude, double longitude) const;
    Q_INVOKABLE void updateMarkers(QObject* mapObject, QObject* containerObject) const;
    Q_INVOKABLE void goToMyLocation();
    Q_INVOKABLE void routeToCurrentStation();
    Q_INVOKABLE void inspectCurrentStation();
    Q_INVOKABLE void filterCurrentStationsDataModel(QString filter);
    Q_INVOKABLE void filterCurrentNearbyStationsDataModel(QString filter);
    Q_INVOKABLE bb::cascades::GroupDataModel* getNearbyStationsDataModel(QString id);


    // RETURN QPROPERTIES

    QVariantMap currentNetwork();
    QVariantMap currentStation();
    QVariantMap nets();
    QVariantList currentNearbyStations();
    QVariantList currentNetworkStations();
    bb::cascades::Image staticMapImage();
    bb::cascades::GroupDataModel* currentStationsDataModel();
    bb::cascades::GroupDataModel* currentNearbyStationsDataModel();

    void updateNearbyStations();


    //SET QPROPERTIES

    void setCurrentNetwork(QVariantMap cn);
    void setCurrentStation(QVariantMap cs);
    void setCurrentNearbyStations(QVariantList cns);
    void setCurrentNetworkStations(QVariantList cns);
    void setNets(QVariantMap n);
    void setStaticMapImage(bb::cascades::Image i);
    void setCurrentStationsDataModel(bb::cascades::GroupDataModel* model);
    void setCurrentNearbyStationsDataModel(bb::cascades::GroupDataModel* model);
    virtual ~Bicing() {}


Q_SIGNALS:
    void currentNetworkChanged(QVariantMap cn);
    void currentStationChanged(QVariantMap cs);
    void currentNearbyStationsChanged(QVariantList cns);
    void currentNetworkStationsChanged(QVariantList cns);
    void netsChanged(QVariantMap n);
    void staticMapImageChanged(bb::cascades::Image i);
    void replyCompleted(QString reply);
    void currentStationsDataModelChanged(bb::cascades::GroupDataModel* model);
    void currentNearbyStationsDataModelChanged(bb::cascades::GroupDataModel* model);
private Q_SLOTS:
    void onReplyFinished();
    void onImageFinished(bb::cascades::maps::MapImageGenerator*);
private slots:
    void onSystemLanguageChanged();


private:
    QTranslator* m_pTranslator;
    bb::cascades::LocaleHandler *m_pLocaleHandler;
    bb::cascades::maps::MapView *m_mapView;
    bb::platform::geo::GeoLocation *m_deviceLocation;
    bb::cascades::Container *m_bubble;
    bb::cascades::Image m_staticMapImage;
    bb::cascades::TabbedPane *m_tabbedPane;
    bb::cascades::Dialog* m_nearbyStationsDialog;
    bb::cascades::GroupDataModel* m_currentStationsDataModel;
    bb::cascades::GroupDataModel* m_currentNearbyStationsDataModel;
    bb::cascades::Sheet* m_placeInspector;
    QVariantMap m_currentNetwork;
    QVariantMap m_networks;
    QVariantMap m_currentStation;
    QVariantList m_currentNearbyStations;
    QVariantList m_currentNetworkStations;

    void getRequest(QString url);
    void getMapImage(double lat,double lon);
    QPoint worldToPixel(QObject* mapObject, double latitude, double longitude) const;
    QString getLocalTimeFromStation(QVariantMap m,QString mode="hh:mm");
};

#endif /* ApplicationUI_HPP_ */
