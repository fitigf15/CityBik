import bb.cascades 1.3
import QtMobility.sensors 1.3
import bb.cascades.maps 1.4
import QtMobilitySubset.location 1.2

Page {
    actions: [
        //! [0]
        ActionItem {
            title: qsTr("Update")
            imageSource: "asset:///images/ic_reload.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                //_bicing.updateMap()
                _cityBikes.refreshStations()
                _cityBikes.refreshBoundingBox()
            }
        }
        ,
        ActionItem {
            title: qsTr("Go to me")
            imageSource: "asset:///images/me.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                //_bicing.goToMyLocation()
                _cityBikes.goToDeviceLocation()
                _cityBikes.refreshBoundingBox()
            }
        }
    ]
    MapView {
        id: mapview
        altitude: 1000
        objectName: "mapViewObj"
        //latitude: 41.3850639
        //longitude: 2.1734035
        onMapLongPressed: {
            console.log(qsTr("map long pressed"));
        }
        onFocusedChanged: {
            
            if(!focused){
                _cityBikes.refreshBoundingBox()
                console.log("focused changed")
            }
            
        }
        onMapDataChanged: {
            
        }
        onFollowedIdChanged: {
            console.log(qsTr("followed id changed to %1").arg(idOfFollowed));
        }
        onFocusedIdChanged: {
            console.log(qsTr("focused id changed to %1").arg(idWithFocus));
        }
        onCaptionButtonClicked: {
            //TAP A PIN BUTTOn
            console.log(qsTr("button clicked %1").arg(focusedId));
        }
        onCaptionLabelTapped: {
            //TAP A PIN LABELF
            console.log(qsTr("label clicked %1").arg(focusedId));
        }
        onLocationTapped: {
            //_bicing.locationTapped(id);
            bubble.stationID = id
            
            //var station = _cityBikes.getStationProperties(id)
            //bubble.setProperties(station["free_bikes"],station["empty_slots"],station["name"],station["localTimestamp"],station["id"],station["latitude"],station["longitude"],station.extra.status,station["isFavorite"])
            //console.log(qsTr("location pressed %1").arg(id));
        }
        onLocationLongPressed: {
            //MAINTAIN A PIN
            console.log(qsTr("location long pressed %1").arg(id));
        }
        captionContent: Bubble {
            objectName: "bubbleObj"
            id: bubble
        }
        
    
    }
    attachedObjects:  [
        PositionSource {
            id: positionSource
            active: true
            updateInterval: 30000

            onPositionChanged: {
                //_bicing.updateDeviceLocation(positionSource.position.coordinate.latitude, positionSource.position.coordinate.longitude)
                _cityBikes.updateDeviceLocation(positionSource.position.coordinate.latitude, positionSource.position.coordinate.longitude)
            }
        },
        Dialog {
            id: nearbyStationsDialog
            objectName: "nearbyStationsDialogObj"
            DialogContent {
                preferredWidth: 720
            }
            
    }]
//! [4]
}
