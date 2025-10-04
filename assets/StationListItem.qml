import bb.cascades 1.3
CustomListItem {
    property variant map
 property int free_bikes
 property int empty_slots
 property string name
 property string timestamp
 property bool status
 property bool isFavorite
 /*
 contextActions: ActionSet {
     actions: [
     ActionItem {
         title: "Favorite/Unfavorite station"
         onTriggered: {
             console.log(ListItem.view.dataModel)
             //ListItem.view.dataModel.setIsFavorite(ListItem.indexPath, !ListItemData.isFavorite);
         }
     },
     ActionItem {
         title: "log visibility"
         onTriggered: {
             console.log(ListItemData.isFavorite)
             console.log(map.isFavorite)
         }
     }]
 }*/
 highlightAppearance: HighlightAppearance.Full
 dividerVisible: true
 content:Container {
     preferredHeight: 110
     preferredWidth: 768
     background: {
         if(status){
             if(free_bikes > 5){
                 return Color.Green
             }
             if(free_bikes == 0){
                 return Color.Red
             }
             return Color.Yellow
         }
         return Color.Black
     }
     ScrollView {
         scrollViewProperties.scrollMode: ScrollMode.Horizontal
         scrollViewProperties.overScrollEffectMode: OverScrollEffectMode.OnScroll
         scrollViewProperties.pinchToZoomEnabled: false
         Label {
             text: name
             textStyle.color: {
                 if(status){
                     return Color.Black
                 }else{
                     return Color.White
                 }
             }
         }
     }
     Container {
         preferredWidth: 768
         layout: StackLayout {
             orientation: LayoutOrientation.LeftToRight
         }
         /*Container {
             ImageView {
                 imageSource:{
                     if(isFavorite){
                         return "asset:///images/ic_favorite_yellow.png"
                     }
                     return "asset:///images/ic_favorite.png"
                 }
             }
         }*/
         Container {
             layout: StackLayout {
                 orientation: LayoutOrientation.LeftToRight
             }
             verticalAlignment: VerticalAlignment.Center
             Label {
                 visible: status
                 text: "\uD83D\uDEB2 : "+free_bikes.toString()
             
             }
             Label {
                 visible: status
                 text: "\u24DF : "+empty_slots.toString() 
             }
             
             Label {
                 visible: !status
                 text: "Closed"
                 textStyle.color: Color.White
             }
         
         }
         Label {
             text: "\uD83D\uDD50 : "+timestamp
             textStyle.color: {
                 if(status){
                     return Color.Black
                 }
                 return Color.White
             }
             horizontalAlignment: HorizontalAlignment.Right
             verticalAlignment: VerticalAlignment.Center
         }
     }
 }
 
}
    

