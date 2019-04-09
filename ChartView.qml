import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import "QMLs"

Item {
    //save names here
    property variant allSeriesName
//    property alias editBar_a: editBar
    Connections {
        target: reciever
        onSendDebugInfo: {
            showPopupTips(qsTr(data), time)
        }
    }
    RowLayout{
        spacing: 10*app.dp
        anchors.top: menuBar.bottom
        anchors.fill: parent
//        anchors.margins: 10*app.dp
        anchors.topMargin: menuBar.height//+10*app.dp
        Rectangle {
            id: listOfConnectedDevices
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            color: "transparent"
//            Layout.preferredHeight: deviceSetter.height
            Layout.minimumHeight: 350*app.dp
            Layout.fillHeight: true
            Layout.minimumWidth: 400 *app.dp
            property var serialNumValues: []
            property var serialNum:[]
            ListModel{
                id: listOfCD_model
            }
            ListView {
                id: listOfCD_view
                width: 180; height: app.height*0.8
                anchors.left: parent.left
                anchors.top: parent.top
                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                model: listOfCD_model
//                model: ListModel {id : listModel}
                delegate: RowLayout {
                    spacing: 2*app.dp
                    Text {
                        Layout.preferredWidth: 70* app.dp
                        Layout.alignment: Qt.AlignBottom
                        Layout.bottomMargin: 23 * app.dp
                        text: name
                        font.pixelSize: app.fontPixelSize}
                    TextField {
                        Layout.alignment: Qt.AlignBottom
                        text: listOfConnectedDevices.serialNumValues[index]
//                        inputMask: index == 0 ? "dd;_" : "9.9dddd;_"
                        placeholderText: "serialNumber#" + name
                        font.pixelSize: app.fontPixelSize
                        onEditingFinished: {
                            listOfConnectedDevices.serialNumValues[index] = text;
                            listOfConnectedDevices.serialNum[index] = name;
                            //TODO: check correctnes of input
                        }
                    }
                }
                Layout.fillHeight: true
                Layout.fillWidth: true
                ScrollBar.vertical: ScrollBar {}
            }
            Button {
                id:addSerialNumber_btn
                contentItem: ButtonLabel {text: qsTr("Add device")}
                anchors.top: listOfCD_view.bottom
                function createListElement(num_) {
                    return {
                        name: num_
                    };
                }
                onClicked: {
                    listOfConnectedDevices.serialNumValues.push(listOfCD_model.count)
                    listOfConnectedDevices.serialNum.push(listOfCD_model.count)
                    console.log(listOfConnectedDevices.serialNumValues.length)
                    listOfCD_model.append(createListElement(listOfCD_model.count))
                }
            }
        }
        ColumnLayout {
            id: controlPanelLayout
            Layout.alignment:  Qt.AlignTop
            Layout.rightMargin: 10*app.dp
            RowLayout {
                TextField {
                    id: gasConc_tf
                    text:qsTr("0")
                    placeholderText: "Conc"
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Gas conc.")
                    validator: DoubleValidator {
                        bottom: 0;
                        top: 99999;
                        decimals: 10;
                        notation: "StandardNotation"

                    }
                }
                Button {
                    id: setConcentration
                    enabled: true
                    contentItem: ButtonLabel {text: qsTr("SetConcentration")}
                    onClicked: {
                        reciever.setCurrentConc(gasConc_tf.text)
                    }
                }
            }
            RowLayout{
                TextField {
                    id: writeDelay_tf
                    text:"5"
                    validator: IntValidator {bottom: 1; top: 120;}
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Write delay, min")
                    placeholderText: "Write Delay"
                }
                Switch {
                    id: writeToFileSwitcher
                    text: qsTr("WriteModeOff")
                    onClicked: {
                        //TODO: send a signal to reciever to switch between DEBUG/WORK mode
                        console.log("position: " + position)
                        if (position == 1) {
                            if(serialNumber_tf.text.length < 2) {
                                showPopupTips(qsTr("Enter serial number(minimum2 character)"), 1500)
                            } else {
                                writeToFileSwitcher.text = qsTr("WriteModeOn")
                                reciever.enableLogging(writeDelay_tf.text)
                            }
                        }
                        else {
                            writeToFileSwitcher.text = qsTr("WriteModeOff")
                            reciever.disableLogging();
                        }
                    }
                }
            }
        }
    }
    Timer {
        id: timer1
    }
    Timer {
        id: timer2
    }
    function delay(delayTime, cb) {
        timer1.interval = delayTime;
        timer1.repeat = false;
        timer1.triggered.connect(cb);
        timer1.start();
    }
    function createAxis(min, max) {
        // The following creates a ValueAxis object that can be then
        //set as a x or y axis for a series
        return Qt.createQmlObject("import QtQuick 2.7;
                                   import QtCharts 2.7;
                                   ValueAxis { min: "
                                  + min + "; max: " + max + " }", graphs);
    }
    function showPopupTips(text, dTime) {
        tipsWithPath.showedText = qsTr(text)
        tipsWithPath.open()
        delay(dTime !== undefined ? dTime : 300, tipsWithPath.close)
    }
}

