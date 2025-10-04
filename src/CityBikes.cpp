/*
 * CityBikes.cpp
 *
 *  Created on: 11/11/2014
 *      Author: Victor
 */

#include "CityBikes.hpp"
#include "JsonDataModel.hpp"
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/cascades/GroupDataModel>
#include <bb/cascades/maps/MapView>
#include <bb/cascades/maps/MapData>
#include <bb/cascades/Color>
#include <bb/cascades/maps/DataProvider>
#include <bb/platform/geo/Point>
#include <bb/platform/geo/GeoLocation>
#include <bb/platform/geo/Marker>
#include <bb/platform/geo/GeoList>
#include <bb/platform/geo/BoundingBox>
#include <bb/data/JsonDataAccess>
using namespace bb;
using namespace bb::cascades;
using namespace bb::cascades::maps;
using namespace bb::platform::geo;

CityBikes::CityBikes():
    QObject()
{
    // prepare the localization
    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);
    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));    // This is only available in Debug builds
    Q_ASSERT(res);
    // Since the variable is not used in the app, this is added to avoid a
    // compiler warning
    Q_UNUSED(res);
    // initial load
    m_jsonDataModel = new GroupDataModel(this);
    m_jsonDataModel->setGrouping(bb::cascades::ItemGrouping::None);

    onSystemLanguageChanged();
    //qmlRegisterType<JsonDataModel>("com.jsondatamodel", 1, 0, "JsonDataModel");

    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    qml->setContextProperty("_cityBikes", this);
    AbstractPane *root = qml->createRootObject<AbstractPane>();

    QObject* mapViewAsQObject = root->findChild<QObject*>(QString("mapViewObj"));
    if (mapViewAsQObject) {
        m_mapView = qobject_cast<bb::cascades::maps::MapView*>(mapViewAsQObject);
        if (m_mapView) {
            // creating a data provider just for the device location object. that way, when the clear function is call, this object is not removed.
            //m_mapView->setRenderEngine(TCS::RenderEngine3d);
             DataProvider* stationsLocDataProv = new DataProvider("stations-data-provider");
             stationsLocDataProv->setVisible(false);
             m_mapView->mapData()->addProvider(stationsLocDataProv);
             DataProvider* currentLocDataProv = new DataProvider("current-location-data-provider");
             m_mapView->mapData()->addProvider(currentLocDataProv);
             // create a geolocation just for the device's location
             DataProvider* deviceLocDataProv = new DataProvider("device-location-data-provider");
             m_mapView->mapData()->addProvider(deviceLocDataProv);
             m_deviceLocation = new GeoLocation("device-location-id");
             m_deviceLocation->setName("Current Device Location");
             Marker bullseye = Marker("asset:///images/me.png", QSize(60, 60),
                                 QPoint(29, 29), QPoint(29, 1));
             m_deviceLocation->setMarker(bullseye);
             deviceLocDataProv->add(m_deviceLocation);


             setUrl("http://api.citybik.es/v2/networks/bicing");

        }

    }
    bb::cascades::Application::instance()->setScene(root);

}
void CityBikes::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    QString locale_string = QLocale().name();
    QString file_name = QString("CityBikes_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}

CityBikes::~CityBikes()
{
}

void CityBikes::addGeoLocation(QVariantMap map){
    GeoLocation* newDrop = new GeoLocation(map["id"].toString());
    newDrop->setPoint(Point(map["latitude"].toDouble(),map["longitude"].toDouble()));
    QString desc;

    Marker flag;
    if(map["extra"].toMap()["status"].toString()=="OPN"){
        if(map["free_bikes"].toInt()>5){
            flag.setIconUri("asset:///images/green_pin.png");
            desc = QString("green");
        }else if(map["free_bikes"].toInt()==0){
            flag.setIconUri("asset:///images/red_pin.png");
            desc = QString("red");
        }else{
            flag.setIconUri("asset:///images/yellow_pin.png");
            desc = QString("yellow");
        }

    }else{
        flag.setIconUri("asset:///images/black_pin.png");
        desc = QString("black");
    }
    newDrop->setDescription(desc);
    flag.setIconSize(QSize(60, 60));
    flag.setLocationCoordinate(QPoint(20, 59));
    flag.setCaptionTailCoordinate(QPoint(20, 1));
    newDrop->setMarker(flag);
    m_mapView->mapData()->provider("stations-data-provider")->add(newDrop);

}

void CityBikes::updateDeviceLocation(double lat, double lon) {
    if (m_deviceLocation) {
        m_deviceLocation->setLatitude(lat);
        m_deviceLocation->setLongitude(lon);
    }
}
void CityBikes::goToDeviceLocation(){
    m_mapView->setLocation(Point(m_deviceLocation->latitude(),m_deviceLocation->longitude()));
    refreshBoundingBox();
}
void CityBikes::getMapImage(double lat, double lon){
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
void CityBikes::onImageFinished(MapImageGenerator * reply){
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

void CityBikes::inspectStation(QString id){
    m_jsonDataModel->clear();
    m_generalFIlter= id;
    m_currentStationOriginalList.clear();
    if(id!="device-location-id"){
        QVariantMap stationMap;
        foreach(QVariantMap m, m_originalList){
            if(m["id"].toString()==id) {
                stationMap = m;
                break;
            }
        }

        qDebug() << stationMap;
        if(!stationMap.isEmpty()){
            getMapImage(stationMap["latitude"].toDouble(),stationMap["longitude"].toDouble());
            QVariantList nearbyStationList=stationMap["extra"].toMap()["NearbyStationList"].toList();
            foreach(QVariantMap m, m_originalList){
                foreach(QVariant uid, nearbyStationList){
                    QString s = m["name"].toString();
                    s.truncate(3);
                    if(s.trimmed().toInt()==uid.toInt()){
                        m_currentStationOriginalList.prepend(m);
                        break;
                    }
                }
            }
            m_jsonDataModel->insertList(m_currentStationOriginalList);
        }
    }

}

void CityBikes::selectOriginalList(){
    m_generalFIlter="";
    m_jsonDataModel->clear();
    m_jsonDataModel->insertList(m_originalList);
}

QVariantMap CityBikes::getStationProperties(QString id){
    foreach(QVariantMap m, m_originalList){
        if (m["id"].toString()==id) {
            qDebug() << m["isFavorite"];
            return m;
        }
    }
    return QVariantMap();
}

bb::cascades::Color CityBikes::getStationBubbleColor(QString id){
    QVariantMap station;
    foreach(QVariantMap m, m_originalList){
        if(m["id"].toString()==id){
            station = m;
        }
    }
    if(station["extra"].toMap()["status"].toString()=="OPN"){
        if(station["free_bikes"].toInt()>5){
            return Color::fromARGB(0x7a00ff00);
        }else if(station["free_bikes"].toInt()==0){
            return Color::fromARGB(0x7aff0000);
        }else{
            return Color::fromARGB(0x7affff00);
        }

    }else{
        return Color::fromARGB(0x7a000000);
    }

}

void CityBikes::routeTo(double lat, double lon){
    qDebug() << lat << lon;
}


//TODO: NOTHING, JUST SEPARATING, DOWN FROM THIS THERE IS JSONDATAMODEL CLASS

bool CityBikes::saveData()
{
    bb::data::JsonDataAccess jda;
    QVariantMap m;
    m.insert("settings",m_settings);
    jda.save(m, "app/native/assets/settings.json");
    qDebug() << jda.load("app/native/assets/settings.json");

    if (jda.hasError()) {
        bb::data::DataAccessError error = jda.error();
        qDebug() << "JSON loading error: " << error.errorType() << ": " << error.errorMessage();
        return false;
    }

    return true;
}

void CityBikes::getRequest(QString url){
    QNetworkAccessManager* m = new QNetworkAccessManager(this);
    QNetworkReply* reply = m->get(QNetworkRequest(url));
    bool ok = connect(reply,SIGNAL(finished()),this,SLOT(onReplyFinished()));
    Q_ASSERT(ok);
    Q_UNUSED(ok);
}
void CityBikes::onReplyFinished(){
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QVariant jsonData;
    QString response;
    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            const int available = reply->bytesAvailable();
            if (available > 0) {
                const QByteArray buffer(reply->readAll());
                bb::data::JsonDataAccess jda;
                jsonData = jda.loadFromBuffer(buffer);
                response="Loaded successfully";
            }else{
                response="No bytes available";
            }

        }
        else{
            response =  tr("Error: %1 status: %2").arg(reply->errorString(), reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
        }
        reply->deleteLater();
    }
    if (response.trimmed().isEmpty()) {
        response = tr("Unable to retrieve request headers");
    }
    //qDebug() << "RESPONSE : " << response;
    parseJsonData(jsonData);
    emit replyCompleted(response);
}

void CityBikes::parseJsonData(QVariant jsonData){

    m_jsonDataModel->clear();
    QVariantMapList qvml;
    m_originalList.clear();
    QVariantList l = jsonData.toMap()["network"].toMap()["stations"].toList();
    findSettings();

    foreach(QVariant v, l){
        QVariantMap m  =v.toMap();
        m.insert("localTimestamp",getLocalTimeFromStation(m["timestamp"].toString()));
        m.insert("isFavorite",false);
        foreach(QVariant s, m_settings["favorites"].toList()){
            if(m["id"].toString()==s.toMap()["id"].toString()){
                m.insert("isFavorite",QVariant(true));
                break;
            }
        }
        m_originalList.prepend(m);
        //addGeoLocation(m);
    }
    m_jsonDataModel->insertList(m_originalList);

}

void CityBikes::resetList(){
    m_jsonDataModel->clear();
    m_jsonDataModel->insertList(m_originalList);
}

void CityBikes::findSettings(){
    qDebug() << "ATTEMPTING TO RETRIEVE SETTINGS";
    bb::data::JsonDataAccess jda;
    QString filepath("app/native/assets/settings.json");
    QVariantMap m = jda.load(filepath).toMap();
    if (jda.hasError()) {
        bb::data::DataAccessError error = jda.error();
        qDebug() << "JSON loading error: " << error.errorType() << ": "
                << error.errorMessage();
        return;
    }
    if(m.isEmpty()){
        qDebug() << "List is empty, attempt failed, ignoring";
        return;
    }else{
        m_settings.clear();
        m_settings=m["settings"].toMap();
        qDebug() << "DONE";
    }
}


QString CityBikes::getLocalTimeFromStation(QString time,QString mode){
    time.replace(time.indexOf('.',Qt::CaseInsensitive),5,"Z");
    QDateTime t = QDateTime::fromString(time,Qt::ISODate);
    return t.toLocalTime().toString(mode);
}

void CityBikes::applyFilter(QString filter){
    m_jsonDataModel->clear();
    filter = filter.trimmed();
    QVariantMapList newList;
    qDebug() << "FILTER" << filter;
    if(m_generalFIlter==""){
        if(filter.isEmpty()){
            newList = m_originalList;
        }else{
            /*if(filter.contains(",",Qt::CaseInsensitive)){
                QStringList filters = filter.split(",");
                foreach(QString f, filters){
                    bool ok;
                    if(f.toInt(&ok, 10)>0){
                        if(ok){
                            if(f.size()==1){
                                f.prepend("0");
                            }
                        }
                    }
                }
                foreach(QVariantMap m, m_originalList){
                    QString s = m["name"].toString();
                    s.truncate(3);
                    foreach(QString f, filters){
                        if(s.trimmed()==f){
                            newList.prepend(m);
                            break;
                        }
                    }
                }
            }else{*/
            foreach(QVariantMap m, m_originalList){
                if(m["name"].toString().contains(filter,Qt::CaseInsensitive)){
                    newList.prepend(m);
                }
            }
        }
    }else{
        if(filter.isEmpty()){
            newList = m_currentStationOriginalList;
        }else{
            foreach(QVariantMap m, m_currentStationOriginalList){
                if(m["name"].toString().contains(filter,Qt::CaseInsensitive)){
                    newList.prepend(m);
                }
            }
        }
    }



    m_jsonDataModel->insertList(newList);

}

void CityBikes::updateItemIsFavoriteAtIndex(QVariantList indexPath, const bool isFavorite)
{
    QVariantMap modelItem = m_jsonDataModel->data(indexPath).toMap();
    m_jsonDataModel->clear();
    int itemDataIndex = m_originalList.indexOf(modelItem);
    // Update the item in the list of data.
    modelItem.insert("isFavorite",isFavorite);
    qDebug() << "BEFORE" <<m_originalList.at(itemDataIndex)["name"] <<m_originalList.at(itemDataIndex)["isFavorite"];
    m_originalList.replace(itemDataIndex,modelItem);
    qDebug() << "AFTER" <<m_originalList.at(itemDataIndex)["name"] <<m_originalList.at(itemDataIndex)["isFavorite"];
    QVariantMap m;
    m.insert("id",modelItem["id"].toString());
    QVariantList favorites = m_settings["favorites"].toList();
    if(favorites.contains(m)){
        if(!isFavorite) favorites.removeAt(m_settings["favorites"].toList().indexOf(m));
    }else{
        if(isFavorite)favorites.prepend(m);
    }
    m_settings.insert("favorites",favorites);

    foreach(QVariant v, m_settings["favorites"].toList()){
        qDebug() << v;
    }
    m_jsonDataModel->insertList(m_originalList);

    // Since the item status was changed, it is removed from the model and
    // consequently it is removed from the current list shown by the app.
    //removeAt(itemIndex);
}

void CityBikes::setIsFavorite(const QVariantList selectionList, const bool isFavorite)
{
    // If the selectionList parameter is a list of index paths update all the items
    if (selectionList.at(0).canConvert<QVariantList>()) {
        for (int i = selectionList.count() - 1; i >= 0; i--) {
            // Get the item at the index path of position i in the selection list
            QVariantList indexPath = selectionList.at(i).toList();
            updateItemIsFavoriteAtIndex(indexPath, isFavorite);
        }
    } else {
        updateItemIsFavoriteAtIndex(selectionList, isFavorite);
    }

    saveData();
}

void CityBikes::deleteItemAtIndex(QVariantList indexPath)
{
    m_jsonDataModel->removeAt(indexPath);
}

void CityBikes::deleteJsonItems(const QVariantList selectionList)
{
    // If the selectionList parameter is a list of index paths update all the items
    if (selectionList.at(0).canConvert<QVariantList>()) {
        for (int i = selectionList.count() - 1; i >= 0; i--) {

            // Get the item at the index path of position i in the selection list.
            QVariantList indexPath = selectionList.at(i).toList();
            deleteItemAtIndex(indexPath);
        }
    } else {
        deleteItemAtIndex(selectionList);
    }

    saveData();
}

// TODO: NOTHING; JUST SEPARATING QPROPERTIES

QString CityBikes::url(){
    return m_url;
}
void CityBikes::setUrl(QString url){
    m_url=url;
    emit urlChanged(url);
    refreshStations();
}
void CityBikes::refreshStations(){
    getRequest(m_url);
}
void CityBikes::refreshBoundingBox(){
    m_mapView->mapData()->provider("current-location-data-provider")->clear();
    foreach(QVariantMap m, m_originalList){
        addGeoLocation(m);
    }
    foreach(Geographic* g,m_mapView->mapData()->provider("stations-data-provider")->find(m_mapView->boundingBox()).toQList()){
        //m_mapView->mapData()->provider("stations-data-provider")->find(m_mapView->boundingBox()).remove(g->geoId());
        m_mapView->mapData()->provider("current-location-data-provider")->add(g);
        //qDebug() << g->geoId();
    }

}
bb::cascades::Image CityBikes::staticMapImage(){
    return m_staticMapImage;
}
void CityBikes::setStaticMapImage(bb::cascades::Image img){
    m_staticMapImage = img;
    emit staticMapImageChanged(img);
}
bb::cascades::GroupDataModel* CityBikes::dataModel(){
    return m_jsonDataModel;
}
void CityBikes::setGeneralFilter(QString generalFilter){
    m_generalFIlter=generalFilter;
}
QString CityBikes::generalFilter(){
    return m_generalFIlter;
}

