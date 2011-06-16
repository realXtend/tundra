import Qt 4.7

Rectangle {
    id: mainWindow
    width:  1000
    height: 600

    Rectangle {
        id: sidePanel
        width: 200
        color: "lightblue"
        anchors.left: mainWindow.left
        anchors.top: mainWindow.top
        anchors.bottom: mainWindow.bottom
        clip: false
        ListView {
            anchors.fill: parent
            model: qmlList
            delegate: listDelegate
        }
    }

    Rectangle {
        color: "lightgrey"
        anchors { left: sidePanel.right; top: mainWindow.top; bottom: mainWindow.bottom; right: mainWindow.right }
        clip: true
        Loader {
            anchors.fill: parent
            id: loaderqml
            source:  ""
        }
    }

    Component {
        id: listDelegate
        Column {
            Text {
                text: name; font.pointSize: 20;
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        loaderqml.source = source
                    }
                }
            }
        }
    }

    ListModel {
        id: qmlList

         ListElement {
             name: "PhotoViewer"
             source: "./photoviewer/photoviewer.qml"
         }
         ListElement {
             name: "Samegame"
             source: "./samegame/samegame.qml"
         }
         ListElement {
             name: "Snake"
             source: "./snake/snake.qml"
         }
         ListElement {
             name: "Calculator"
             source: "./calculator/calculator.qml"
         }
         ListElement {
             name: "RSS news"
             source: "./rssnews/rssnews.qml"
         }
     }
}
