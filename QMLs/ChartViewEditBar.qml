import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0

RowLayout {
    id: ctrlPane
    spacing: 10*app.dp
    property int itemWidth: 250*app.dp
    Connections {
        target: reciever
        onMakeSeries: {
            createSeries()
        }
    }
    Button {
        id: enableFlyModeBTN
        enabled: true
//                height: 2.3*24*app.dp
        width: 250*app.dp
        contentItem: ButtonLabel {text: qsTr("Fly mode")}
//                ToolTip.visible: hovered
//                    ToolTip.text: qsTr("Enable fly mode")
        onClicked: {
            //debug
//            createSeries()
//            reciever.readData()
            reciever.prepareCommandToSend("duty\r")
            reciever.setFlyMode(true)
        }
    }
    Button {
        id: setZeroLevelBTN
//                height: 2.3*48*app.dp
        width: 250*app.dp
//                ToolTip.visible: hovered
//                    ToolTip.text: qsTr("Set zero level")
        contentItem: ButtonLabel {text: qsTr("Set zero level")}
        onClicked: {
            reciever.prepareCommandToSend("zero\r")
        }
    }
    Switch {
        id: debugModeSwitcher
        text: qsTr("Debug mode")
        onClicked: {
            //TODO: send a signal to reciever to switch between DEBUG/WORK mode
            console.log("position: " + position)
            if (position == 1) {
                reciever.prepareCommandToSend("debug\r")
                reciever.setFlyMode(false)
            }
            else {
                reciever.prepareCommandToSend("work\r")
                reciever.setFlyMode(true)
            }
        }
    }
    function createSeries() {
        var colorList = [
                    "#F44336", "#673AB7", "#03A9F4", "#4CAF50", "#FFEB3B", "#FF5722",
                    "#E91E63", "#3F51B5", "#00BCD4", "#8BC34A", "#FFC107",
                    "#9C27B0", "#2196F3", "#009688", "#CDDC39", "#FF9800"
                ]
        //Umeas
        graph_1.numSeries++;
        var seriesName = qsTr("series_" + graph_1.numSeries)
        graph_1.createSeries(ChartView.SeriesTypeLine,
                            seriesName,
                            axisX_Umeas, axisY_Umeas);
        graph_1.series(seriesName).color = /*colorList[
                    Math.random()*100*( graph_Umeas.numSeries - 1) % colorList.length ]*/"#B71C1C"
        reciever.sendSeriesPointer(graph_1.series(seriesName),graph_1.axisX(seriesName));
        //U_ref
        graph_2.numSeries++;
        seriesName = qsTr("series_" + graph_2.numSeries)
        graph_2.createSeries(ChartView.SeriesTypeLine,
                            seriesName,
                            axisX_Uref, axisY_Uref);
        graph_2.series(seriesName).color = /*colorList[
                    Math.random()*100*( graph_Uref.numSeries - 1) % colorList.length ]*/"#B71C1C"
        reciever.sendSeriesPointer(graph_2.series(seriesName),graph_2.axisX(seriesName))
    }
}
