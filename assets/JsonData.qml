import bb.cascades 1.3
import bb.data 1.0

Page {
    titleBar: TitleBar {
        id: segmentedTitle
        kind: TitleBarKind.Segmented
        scrollBehavior: TitleBarScrollBehavior.Sticky
        
        // The segmented control decides which filter should be set on the
        // dataModel used by the photo bucket list.
        options: [
            Option {
                text: qsTr("all") + Retranslate.onLanguageChanged
                value: ("all")
            },
            Option {
                text: qsTr("favorites") + Retranslate.onLanguageChanged
                value: ("favorites")
            }
        ]
        
        onSelectedValueChanged: {
            // When a new Option is selected the dataModel of the ListView, the jsonDataModels
            // filter is set and the list is repopulated.
            jsonList.favSelected=selectedValue.toString()=="favorites"
            console.log(selectedValue.toString())
        }
    }
    actions: [
        //! [0]
        ActionItem {
            title: qsTr("Update")
            imageSource: "asset:///images/ic_reload.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                //_bicing.updateMap()
                _cityBikes.refreshStations()
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
            }
        }
    ]
    Container {
        
        TextField {
            id: textFilter
            hintText: "Filter stations"
            onTextChanging: {
                //_bicing.filterCurrentStationsDataModel(text);
                _cityBikes.applyFilter(text)
            }
        }
        ListView {
            id: listView
            dataModel: _cityBikes.dataModel
            /*
            dataModel: JsonDataModel{
                
            }*/
            listItemComponents: ListItemComponent {
                type: "item"
                    StationListItem {
                        name: ListItemData.name
                        free_bikes: ListItemData.free_bikes
                        empty_slots: ListItemData.empty_slots
                        timestamp: ListItemData.localTimestamp
                        status: ListItemData.extra.status.toString()=="OPN"
                    
                    } 
             }
            onTriggered: {
                var selectedItem = dataModel.data(indexPath);
                //_bicing.locationTapped(selectedItem["id"]);
                openPlaceInspector(selectedItem["id"])
            }

        
        }
    }
    
    
    // this function is included locally, but you can also include separately via a header definition

}
