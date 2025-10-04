import bb.cascades 1.3
import bb.data 1.0


Page {
    property alias dataModel: 
    Container {
        Container {
            preferredHeight: 500
            preferredWidth: 768
            ListView {
                dataModel: dataModel
                listItemComponents: ListItemComponent {
                    type: "item"
                    StandardListItem {
                        title: ListItemData.cleaname
                        description: "FREE: "+ListItemData.free + " BIKES: "+ListItemData.bikes
                    }
                }
            
            }
        }
    
    }
}
