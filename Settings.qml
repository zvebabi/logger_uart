import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtCharts 2.2
import QtQuick.Dialogs 1.2
import "QMLs"

Item {
    Connections {
        target: reciever
        onSendPortName: {
            availablePorts.append({"text": port});
            console.log(port)
        }
        onSendDebugInfo: {
            tipsWithPath.showedText = qsTr(data)
            tipsWithPath.open()
            delay(time, tipsWithPath.close)
        }
    }
    Timer {
        id: timer
    }
    function delay(delayTime, cb) {
        timer.interval = delayTime;
        timer.repeat = false;
        timer.triggered.connect(cb);
        timer.start();
    }
    RowLayout{
        anchors.fill: parent
        anchors.margins: statusBar.height + 10*app.dp
        anchors.topMargin: menuBar.height + 10*app.dp
        Rectangle {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            color: "transparent"
//            Layout.preferredHeight: deviceSetter.height
            Layout.minimumHeight: 350*app.dp
            Layout.fillHeight: true
            Layout.minimumWidth: deviceSetter.itemsWidth * 1.1
            Column {
                id: deviceSetter
                spacing: 5*app.dp
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.minimumWidth: deviceSetter.itemsWidth * 1.1
                Layout.fillHeight: true
                property int itemsWidth: 250*app.dp
                Button {
                    id: listDeviceBTN
                    contentItem: ButtonLabel {text: qsTr("Select Device")}
                    width: deviceSetter.itemsWidth
                    onClicked: {//getPorts()
                        availablePorts.clear()
                        reciever.getListOfPort()
                    }
    //                height: 150*app.dp
                }
                ComboBox {
                    id: portsComboList
                    objectName: "comboList"
                    model: availablePorts
                    width: deviceSetter.itemsWidth
                    ListModel{
                        id: availablePorts
                    }
                }
                ComboBox {
                    id: baudRateComboList
                    objectName: "comboList"
                    model: availableBaudRate
                    width: deviceSetter.itemsWidth
                    currentIndex: 0
                    ListModel{
                        id: availableBaudRate
                            ListElement{ text: "115200" }
                            ListElement{ text: "57600"  }
                            ListElement{ text: "38400"  }
                            ListElement{ text: "19200"  }
                            ListElement{ text: "9600"   }
                            ListElement{ text: "4800"   }
                            ListElement{ text: "2400"   }
                            ListElement{ text: "1200"   }
                    }
                }
                Button {
                    id: connectBTN
                    contentItem: ButtonLabel {text: qsTr("Connect")}
                    width: deviceSetter.itemsWidth
                    onClicked: {
                        reciever.initDevice(portsComboList.currentText
                                          , baudRateComboList.currentText);
                    }
                }
                Button {
                    id:addSerialNumber_btn
                    contentItem: ButtonLabel {text: qsTr("Add device")}
                    width: deviceSetter.itemsWidth
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
                //logger part
                Row{
                    id: concSetterRow
                    spacing: 15*app.dp
                    width: deviceSetter.itemsWidth
                    TextField {
                        id: gasConc_tf
                        width: deviceSetter.itemsWidth/2 - concSetterRow.spacing/2
//                        anchors.rightMargin: 15*app.dp
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
                        width: deviceSetter.itemsWidth/2 - concSetterRow.spacing/2
                        enabled: true
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Set gas concentration")
                        contentItem: ButtonLabel {text: qsTr("Set C")}
                        onClicked: {
                            reciever.setCurrentConc(gasConc_tf.text)
                        }
                    }
                }
                Row{
                    id: loggerSwitchRow
                    spacing: 15*app.dp
                    anchors.margins: 15*app.dp
                    TextField {
                        id: writeDelay_tf
                        width: deviceSetter.itemsWidth/2 - loggerSwitchRow.spacing/2
                        text:"5"
                        validator: IntValidator {bottom: 1; top: 120;}
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Write delay, min")
                        placeholderText: "Write Delay"
                    }
                    Switch {
                        id: writeToFileSwitcher
                        width: deviceSetter.itemsWidth/2 - loggerSwitchRow.spacing/2
                        text: qsTr("Log")
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Enable/Disable writing to file")
                        onClicked: {
                            //TODO: send a signal to reciever to switch between DEBUG/WORK mode
                            console.log("position: " + position)
                            if (position == 1) {
                                if(listOfConnectedDevices.serialNum.length == 0) {
                                    showPopupTips(qsTr("Enter SN of device (Press Add device button)"), 1500)
                                    writeToFileSwitcher.position = 0
                                    writeToFileSwitcher.checked = false

                                } else {
//                                    writeToFileSwitcher.text = qsTr("WriteModeOn")
                                    reciever.enableLogging(writeDelay_tf.text, listOfConnectedDevices.serialNumValues)
                                }
                            }
                            else {
//                                writeToFileSwitcher.text = qsTr("WriteModeOff")
                                reciever.disableLogging();
                            }
                        }
                    }
                }
                //
                Label {
                    text: qsTr("Save images and data to: ")
                    font.family: "DejaVu Sans Mono"
                    font.pixelSize: app.fontPixelSize
                    visible: false
                }
                TextField {
                    id: filePathText
                    visible: false
                    width: deviceSetter.itemsWidth
                    text:reciever.getDataPath()
                    font.family: "DejaVu Sans Mono"
                    font.pixelSize: app.fontPixelSize
                    readOnly: true
                    selectByMouse: true
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: {
                            if(mouse.button === Qt.RightButton) {
                                filePathText.copy()
                                tipsWithPath.showedText = qsTr("Path has been copied to clipboard")
                                tipsWithPath.open()
                                delay(1500, tipsWithPath.close)
                                filePathText.deselect()
                            }
                            if(mouse.button === Qt.LeftButton) {
                                filePathText.selectAll()
                            }
                        }
                    }
                }
                Button {
                    id: selectPath
                    contentItem:  ButtonLabel {text:qsTr("Save data to…")}
                    width: deviceSetter.itemsWidth
                    visible: false
                    FileDialog {
                        id: fileDialog
                        title: qsTr("Select directory")
                        visible: false
                        folder: "file:///" + reciever.getDataPath()
                        selectExisting: true
                        selectFolder: true
                        selectMultiple: false
                        onAccepted: {
                            reciever.selectPath(fileUrl.toString().substring(8) + "/")
                            filePathText.text = reciever.getDataPath()

                        }
                    }
                    onClicked: fileDialog.open()
                }
            }
            //information
            Column {
                anchors.bottom: parent.bottom
                anchors.topMargin: 10*app.dp
                Label {
                    id: pathLbl_h
                    font.pixelSize: app.fontPixelSize
                    text: qsTr("Measurement data storage directory: ")
                }
                Label {
                    id: pathLbl
                    font.pixelSize: app.fontPixelSize
                    text: filePathText.text
                }
//                Label {
//                    id: pathLbl_f
//                    font.pixelSize: app.fontPixelSize
//                    text: qsTr("Click \"Save data to…\" to choose another location")
//                }
            }
        }
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
//                        text: listOfConnectedDevices.serialNumValues[index]
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
        }
    }
    function showPopupTips(text, dTime) {
        tipsWithPath.showedText = qsTr(text)
        tipsWithPath.open()
        delay(dTime !== undefined ? dTime : 300, tipsWithPath.close)
    }
}
