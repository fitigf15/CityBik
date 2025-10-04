import bb.cascades 1.3
WebView {
    property string latitude
    property string longitude
    preferredHeight:1030
    
    html: 
    "<head>
        <meta charset=\"utf-8\">
        <title>Street View containers</title>
        <style>
            html, body, #map-canvas {
            height: 100%;
            margin: 0px;
            padding: 0px
            }
        </style>
        <script src=\"https://maps.googleapis.com/maps/api/js?v=3.exp\"></script>
        <script>
            function initialize() {
                var place = new google.maps.LatLng("+latitude+", "+longitude+");
                var panoramaOptions = {
                position: place,
                pov: {
                    heading: 165,
                    pitch: 0
                },
                zoomControlOptions:{
                    position : google.maps.ControlPosition.TOP_RIGHT,
                    style : google.maps.ZoomControlStyle.LARGE
                },
                addressControlOptions:{
                    position: google.maps.ControlPosition.TOP_CENTER
                },
                zoom: 1
            };
            var myPano = new google.maps.StreetViewPanorama(
                document.getElementById('map-canvas'),
                panoramaOptions);
                myPano.setVisible(true);
            } 
            google.maps.event.addDomListener(window, 'load', initialize);
    
        </script>
    </head>
    <body>
        <div id=\"map-canvas\"></div>
    </body>"
}