import bb.cascades 1.3

Container {
    preferredHeight: 72
    background: Color.create("#7affffff")
    verticalAlignment: VerticalAlignment.Center
    ListView {
        verticalAlignment: VerticalAlignment.Center
        preferredWidth: parent.preferredWidth
        id: nearbyList
        dataModel: GroupDataModel {
            id: nearbyListDataModel
            grouping: ItemGrouping.None
            sortedAscending: false
            
        
        }
        listItemComponents: ListItemComponent {
            type: "item"
            Label {
                preferredHeight: 72
                text: ListItemData.extra.uid
                textStyle.color:{
                    if(ListItemData.extra.status.toString()=="OPN"){
                        if(ListItemData.free_bikes.toString()>5){
                            return Color.Green
                        }
                        if(ListItemData.free_bikes.toString()==0){
                            return Color.Red
                        }
                            return Color.Yellow
                    }
                    return Color.Black
                }
    
                textStyle.textAlign: TextAlign.Center
                verticalAlignment: VerticalAlignment.Bottom
                horizontalAlignment: HorizontalAlignment.Center
                textStyle.fontWeight: FontWeight.Bold
                textStyle.fontSize: FontSize.Small
                leftMargin: 7.5
                rightMargin: 7.5
    
            }
    
        }
    }
}
