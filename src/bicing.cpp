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

#include "bicing.hpp"
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/cascades/Container>
#include <bb/cascades/maps/MapView>
#include <bb/cascades/maps/MapData>
#include <bb/cascades/maps/ViewProperties>
#include <bb/cascades/maps/MapImageGenerator>
#include <bb/cascades/maps/DataProvider>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/Image>
#include <bb/cascades/TabbedPane>
#include <bb/cascades/Dialog>
#include <bb/cascades/ListView>
#include <bb/cascades/ListItemProvider>
#include <bb/cascades/QListDataModel>
#include <bb/data/JsonDataAccess>
#include <bb/platform/geo/Point>
#include <bb/platform/geo/GeoLocation>
#include <bb/platform/geo/Marker>

#include <QNetworkAccessManager>
#include <QPoint>

using namespace bb;
using namespace bb::cascades;
using namespace bb::cascades::maps;
using namespace bb::platform::geo;
Bicing::Bicing() :
        QObject()
{
    // prepare the localization
    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
    // This is only available in Debug builds
    Q_ASSERT(res);
    // Since the variable is not used in the app, this is added to avoid a
    // compiler warning
    Q_UNUSED(res);

    m_currentNetwork.insert("default",QVariant("Nothing"));
    m_currentStationsDataModel = new GroupDataModel(this);
    m_currentNearbyStationsDataModel = new GroupDataModel(this);
    m_currentStationsDataModel->setGrouping(bb::cascades::ItemGrouping::None);
    m_currentNearbyStationsDataModel->setGrouping(bb::cascades::ItemGrouping::None);

    // initial load
    onSystemLanguageChanged();

    // Create scene document from main.qml asset, the parent is set
    // to ensure the document gets destroyed properly at shut down.
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    qml->setContextProperty("_bicing", this);


    // Create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();

    QObject* tabbedPaneAsQObject = root->findChild<QObject*>(QString("tabbedPaneObj"));
    if(tabbedPaneAsQObject){
        m_tabbedPane = qobject_cast<bb::cascades::TabbedPane*>(tabbedPaneAsQObject);
    }
    QObject* placeInspectorAsObject = root->findChild<QObject*>(QString("placeInspectorObj"));
        if(placeInspectorAsObject){
        m_placeInspector = qobject_cast<bb::cascades::Sheet*>(placeInspectorAsObject);
    }


    QObject* mapViewAsQObject = root->findChild<QObject*>(QString("mapViewObj"));
     if (mapViewAsQObject) {
         m_mapView = qobject_cast<bb::cascades::maps::MapView*>(mapViewAsQObject);
         if (m_mapView) {
             QObject* nearbyStationsDialogAsQObject = root->findChild<QObject*>(QString("nearbyStationsDialogObj"));
             if(nearbyStationsDialogAsQObject){
                 m_nearbyStationsDialog = qobject_cast<bb::cascades::Dialog*>(nearbyStationsDialogAsQObject);
             }
             // creating a data provider just for the device location object. that way, when the clear function is call, this object is not removed.

             DataProvider* stationsLocDataProv = new DataProvider("stations-data-provider");

             m_mapView->mapData()->addProvider(stationsLocDataProv);
             // create a geolocation just for the device's location
             DataProvider* deviceLocDataProv = new DataProvider("device-location-data-provider");
             m_mapView->mapData()->addProvider(deviceLocDataProv);
             m_deviceLocation = new GeoLocation("device-location-id");
             m_deviceLocation->setName("Current Device Location");
             Marker bullseye = Marker("asset:///images/me.png", QSize(60, 60),
                                 QPoint(29, 29), QPoint(29, 1));
             qDebug()<< bullseye.absoluteIconPath();
             m_deviceLocation->setMarker(bullseye);

             deviceLocDataProv->add(m_deviceLocation);
             QObject* bubbleObject = m_mapView->findChild<QObject*>(QString("bubbleObj"));
             if (bubbleObject) {
                 m_bubble = qobject_cast<bb::cascades::Container*>(bubbleObject);
             }
             //getMapImage(m_deviceLocation->latitude(),m_deviceLocation->longitude());
             updateMap();

         }
     }
    // Set created root object as the application scene
     bb::cascades::Application::instance()->setScene(root);

}

void Bicing::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    QString locale_string = QLocale().name();
    QString file_name = QString("Bicing_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}

void Bicing::locationTapped(QString id){
    if(id=="device-location-id"){
        m_bubble->setProperty("device",true);
    }
    else{
        QVariantList l = m_currentNetwork["stations"].toList();

        for (QList<QVariant>::iterator childIt = l.begin(); childIt != l.end(); childIt++){
            if (childIt->toMap()["id"]==id) {
                setCurrentStation(childIt->toMap());
            }
        }
        m_bubble->setProperty("device",false);
        m_bubble->setProperty("timestamp",getLocalTimeFromStation(m_currentStation));
        m_bubble->setProperty("name",m_currentStation["name"].toString());
        m_bubble->setProperty("status",m_currentStation["extra"].toMap()["status"].toString()=="OPN");
        m_bubble->setProperty("free_bikes",m_currentStation["free_bikes"].toInt());
        m_bubble->setProperty("empty_slots",m_currentStation["empty_slots"].toInt());
        m_bubble->setProperty("latitude",m_currentStation["latitude"].toString());
        m_bubble->setProperty("longitude",m_currentStation["longitude"].toString());
    }
}
void Bicing::updateMap(){
    if (m_mapView) {
        //SystemProgressToast* toast = new SystemProgressToast()
        getRequest("http://api.citybik.es/v2/networks/bicing");

    }
}

void Bicing::updateDeviceLocation(double lat, double lon) {
    /*qDebug() << "updateDeviceLocation( " << lat << ", " << lon
            << " )";*/
    if (m_deviceLocation) {
        m_deviceLocation->setLatitude(lat);
        m_deviceLocation->setLongitude(lon);
    }
}
void Bicing::getRequest(QString url){
    QNetworkAccessManager* m = new QNetworkAccessManager(this);
    QNetworkReply* reply = m->get(QNetworkRequest(url));
    bool ok = connect(reply,SIGNAL(finished()),this,SLOT(onReplyFinished()));
    //qDebug() << ok;
    //qDebug() << reply->errorString();
    Q_ASSERT(ok);
    Q_UNUSED(ok);
}
void Bicing::updateNearbyStations(){

}
void Bicing::onReplyFinished(){
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QVariantMap m;
    QString response;
    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            const int available = reply->bytesAvailable();
            if (available > 0) {
                const QByteArray buffer(reply->readAll());
                bb::data::JsonDataAccess jda;
                m = jda.loadFromBuffer(buffer).toMap();
                //qDebug() << m;
                //qDebug() <<m["networks"].toList()[0].toMap()["name"].toString();
                //qDebug()<<m["network"].toMap()["stations"].toList()[0].toMap()["name"].toString();
                response="Loaded";
            }else{
                response="No bytes available";
                m.insert("ByteError",QVariant(response));
            }

        }
        else{
            response =  tr("Error: %1 status: %2").arg(reply->errorString(), reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
            m.insert("NoReply",QVariant(response));
        }
        reply->deleteLater();
    }
    if (response.trimmed().isEmpty()) {
        response = tr("Unable to retrieve request headers");
        m.insert("NoHeaders",QVariant(response));
    }
    //qDebug() << "RESPONSE : " << response;
    setCurrentNetwork(m);
    emit replyCompleted(response);
}


 void Bicing::getMapImage(double lat, double lon){
     ViewProperties mapProperties;
     mapProperties.setLatitude(lat);
     mapProperties.setLongitude(lon);
     mapProperties.setAltitude(200);
     QRect size(0,0,768,300);
     mapProperties.setWindow(size);
     GeoList data;
     GeoLocation* location = new GeoLocation(lon,lat);
     data.add(location);

     MapImageGenerator *imageGenerator = new MapImageGenerator(mapProperties,data);
     bool ok = QObject::connect(imageGenerator, SIGNAL(finished(bb::cascades::maps::MapImageGenerator*)),this,SLOT(onImageFinished(bb::cascades::maps::MapImageGenerator*)));
     Q_ASSERT(ok);
     Q_UNUSED(ok);
     imageGenerator->start();



}
void Bicing::onImageFinished(MapImageGenerator * reply){
    bb::ImageData imgD = reply->imageData();
    if ( imgD.isValid() ) {
        bb::cascades::Image img(imgD);
        setStaticMapImage(img);
        qDebug() << "Loaded image succesfully";
    } else {
        qDebug() << "Error loading image";
    }
    reply->deleteLater();
}

void Bicing::setStaticMapImage(bb::cascades::Image img){
    m_staticMapImage = img;
    emit staticMapImageChanged(img);
}

void Bicing::setNets(QVariantMap n){
    m_networks = n;
    emit netsChanged(n);
}
QVariantList Bicing::worldToPixelInvokable(QObject* mapObject, double latitude, double longitude) const
{
    MapView* mapview = qobject_cast<MapView*>(mapObject);
    const Point worldCoordinates = Point(latitude, longitude);
    const QPoint pixels = mapview->worldToWindow(worldCoordinates);

    return QVariantList() << pixels.x() << pixels.y();
}

void Bicing::updateMarkers(QObject* mapObject, QObject* containerObject) const
{
    MapView* mapview = qobject_cast<MapView*>(mapObject);
    Container* container = qobject_cast<Container*>(containerObject);

    for (int i = 0; i < container->count(); i++) {
        const QPoint xy = worldToPixel(mapview,
                                       container->at(i)->property("lat").value<double>(),
                                       container->at(i)->property("lon").value<double>());
        container->at(i)->setProperty("x", xy.x());
        container->at(i)->setProperty("y", xy.y());
    }
}
QPoint Bicing::worldToPixel(QObject* mapObject, double latitude, double longitude) const
{
    MapView* mapview = qobject_cast<MapView*>(mapObject);
    const Point worldCoordinates = Point(latitude, longitude);

    return mapview->worldToWindow(worldCoordinates);
}

void Bicing::goToMyLocation(){
    m_mapView->setLocation(Point(m_deviceLocation->latitude(),m_deviceLocation->longitude()));
}
void Bicing::routeToCurrentStation(){
    ;
}
void Bicing::inspectCurrentStation(){
    getMapImage(m_currentStation["latitude"].toDouble(),m_currentStation["longitude"].toDouble());
    m_currentNearbyStationsDataModel->clear();
    m_currentNearbyStationsDataModel->insertList(m_currentNearbyStations);
    m_placeInspector->setProperty("device",false);
    qDebug() << "helo";
    m_placeInspector->setProperty("timestamp",getLocalTimeFromStation(m_currentStation));
    m_placeInspector->setProperty("name",m_currentStation["name"].toString());
    m_placeInspector->setProperty("status",m_currentStation["extra"].toMap()["status"].toString()=="OPN");
    m_placeInspector->setProperty("free_bikes",m_currentStation["free_bikes"].toInt());
    m_placeInspector->setProperty("empty_slots",m_currentStation["empty_slots"].toInt());
    m_placeInspector->setProperty("lat",m_currentStation["latitude"].toString());
    m_placeInspector->setProperty("lon",m_currentStation["longitude"].toString());
}
void Bicing::filterCurrentStationsDataModel(QString filter){
    filter = filter.trimmed();
    QVariantList l = m_currentNetwork["stations"].toList();
    m_currentStationsDataModel->clear();
    if(filter.isEmpty()){
        m_currentStationsDataModel->insertList(l);
    }else{
        for (QList<QVariant>::iterator childIt = l.begin(); childIt != l.end(); childIt++)
        {
            QString str = childIt->toMap()["name"].toString();
            if(str.contains(filter,Qt::CaseInsensitive)){
                m_currentStationsDataModel->insert(childIt->toMap());
            }
        }
    }
}
void Bicing::filterCurrentNearbyStationsDataModel(QString filter){
    filter = filter.trimmed();
    QVariantList l = m_currentNearbyStations;
    m_currentNearbyStationsDataModel->clear();
    if(filter.isEmpty()){
        m_currentNearbyStationsDataModel->insertList(l);
    }else{
        for (QList<QVariant>::iterator childIt = l.begin(); childIt != l.end(); childIt++)
        {
            QString str = childIt->toMap()["name"].toString();
            if(str.contains(filter,Qt::CaseInsensitive)){
                m_currentNearbyStationsDataModel->insert(childIt->toMap());
            }
        }
    }
}

GroupDataModel* Bicing::getNearbyStationsDataModel(QString id){
    GroupDataModel* m = new GroupDataModel(this);
    QVariantList l = m_currentNetwork["stations"].toList();
    QVariantList qvm;

    for (QList<QVariant>::iterator childIt = l.begin(); childIt != l.end(); childIt++){
        if (childIt->toMap()["id"]==id) {
            QVariantList l2 = childIt->toMap()["extra"].toMap()["NearbyStationList"].toList();
            for (QList<QVariant>::iterator childIt2 = l2.begin(); childIt2 != l2.end(); childIt2++){
                for (QList<QVariant>::iterator childIt3 = l.begin(); childIt3 != l.end(); childIt3++){
                    if(childIt3->toMap()["extra"].toMap()["uid"].toInt()==childIt2->toInt()){
                        qvm.prepend(QVariant(childIt3->toMap()));
                    }
                }
            }
        }
    }
    m->insertList(qvm);
    return m;
}

QString Bicing::getLocalTimeFromStation(QVariantMap m,QString mode){
    std::string timestamp = m["timestamp"].toString().toStdString();
    timestamp= timestamp.replace(timestamp.find('.'),5,"Z");
    QDateTime t =QDateTime::fromString(timestamp.c_str(),Qt::ISODate);
    return t.toLocalTime().toString(mode);
}

//SET QPROPERTIES

void Bicing::setCurrentNetwork(QVariantMap cn){
    m_currentNetwork = cn["network"].toMap();
    DataProvider* stationsLocDataProv = m_mapView->mapData()->provider("stations-data-provider");
    stationsLocDataProv->setVisible(false);
    stationsLocDataProv->clear();
    QVariantList l = m_currentNetwork["stations"].toList();

    for (QList<QVariant>::iterator childIt = l.begin(); childIt != l.end(); childIt++)
    {
        QVariantMap childMap = childIt->toMap();

        childMap.insert("timestamp",QVariant(getLocalTimeFromStation(childMap)));

        m_currentNetworkStations.prepend(childMap);

        GeoLocation* newDrop = new GeoLocation(childMap["id"].toString(),childMap["name"].toString(), Point(childMap["latitude"].toDouble(),childMap["longitude"].toDouble()));
        QString desc = QString("Free: "+childMap["free_bikes"].toString()+" - Empty: "+childMap["empty_slots"].toString());
        newDrop->setDescription(desc);
        Marker flag;
        if(childMap["extra"].toMap()["status"].toString()=="OPN"){
            if(childMap["free_bikes"].toInt()>5){
                flag.setIconUri("asset:///images/green_pin.png");
            }else if(childMap["free_bikes"].toInt()==0){
                flag.setIconUri("asset:///images/red_pin.png");
            }else{
                flag.setIconUri("asset:///images/yellow_pin.png");
            }

        }else{
            flag.setIconUri("asset:///images/black_pin.png");
        }
        flag.setIconSize(QSize(60, 60));
        flag.setLocationCoordinate(QPoint(20, 59));
        flag.setCaptionTailCoordinate(QPoint(20, 1));
        newDrop->setMarker(flag);
        stationsLocDataProv->add(newDrop);


    }
    m_currentStationsDataModel->clear();
    m_currentStationsDataModel->insertList(m_currentNetworkStations);
    stationsLocDataProv->setVisible(true);
    emit currentNetworkChanged(cn);

}
void Bicing::setCurrentStation(QVariantMap cs){
    m_currentStation = cs;
    qDebug() << m_currentNetworkStations[0].toMap()["name"].toString();
    qDebug() << m_currentNetwork["stations"].toList()[0].toMap()["name"].toString();
    QVariantList l = m_currentNetworkStations;
    QVariantList l2 = m_currentStation["extra"].toMap()["NearbyStationList"].toList();
    m_currentNearbyStations.clear();
    for (QList<QVariant>::iterator childIt2 = l2.begin(); childIt2 != l2.end(); childIt2++)
    {
        for (QList<QVariant>::iterator childIt3 = l.begin(); childIt3 != l.end(); childIt3++){
            if(childIt3->toMap()["extra"].toMap()["uid"].toInt()==childIt2->toInt()){
                QVariantMap childMap = childIt3->toMap();
                childMap.insert("timestamp",QVariant(getLocalTimeFromStation(childMap)));
                m_currentNearbyStations.prepend(QVariant(childMap));
            }
        }

    }
    emit currentStationChanged(cs);
};
void Bicing::setCurrentNearbyStations(QVariantList cns){
    m_currentNearbyStations = cns;
    emit currentNearbyStationsChanged(cns);
}
void Bicing::setCurrentStationsDataModel(GroupDataModel* model){
    m_currentStationsDataModel= model;
    emit currentStationsDataModelChanged(model);
}
void Bicing::setCurrentNearbyStationsDataModel(GroupDataModel* model){
    m_currentNearbyStationsDataModel= model;
    emit currentNearbyStationsDataModelChanged(model);
}
void Bicing::setCurrentNetworkStations(QVariantList cns){
    m_currentNetworkStations = cns;
    emit currentNetworkStationsChanged(cns);
}
// RETURN QPROPERTIES

QVariantMap Bicing::currentNetwork(){
    return m_currentNetwork;
}
QVariantMap Bicing::nets(){
    return m_networks;
}
QVariantMap Bicing::currentStation(){
    return m_currentStation;
}
QVariantList Bicing::currentNearbyStations(){
    return m_currentNearbyStations;
}
QVariantList Bicing::currentNetworkStations(){
    return m_currentNetworkStations;
}
bb::cascades::Image Bicing::staticMapImage(){
    return m_staticMapImage;
}
GroupDataModel* Bicing::currentStationsDataModel(){
    return m_currentStationsDataModel;
}
GroupDataModel* Bicing::currentNearbyStationsDataModel(){
    return m_currentNearbyStationsDataModel;
}
