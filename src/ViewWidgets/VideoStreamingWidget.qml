/****************************************************************************
 *
 * Copyright (c) 2016, Intel Corporation
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/**
 * @file
 *   @brief QGC Video Streaming Widget
 *   @author Ricardo de Almeida Gonzaga <ricardo.gonzaga@intel.com>
 *   @author Otavio Pontes <otavio.pontes@intel.com>
 */

import QtQuick                    2.5
import QtQuick.Controls           1.2
import QtQuick.Controls.Styles    1.2
import QtQuick.Dialogs            1.2

import QGroundControl             1.0
import QGroundControl.Palette     1.0
import QGroundControl.Controls    1.0
import QGroundControl.Controllers 1.0
import QGroundControl.ScreenTools 1.0

QGCView {
    viewPanel:    panel

    property real _margins:             ScreenTools.defaultFontPixelWidth
    property real _labelsWidth:         ScreenTools.defaultFontPixelWidth * 7
    property real _elementsHeight:      ScreenTools.defaultFontPixelWidth * 3
    property real _elementsWidth:       ScreenTools.defaultFontPixelWidth * 28
    property real _buttonWidth:         ScreenTools.defaultFontPixelWidth * 15
    property real _frameSizeFieldWidth: ScreenTools.defaultFontPixelWidth * 12

    QGCPalette {
        id:                qgcPal
        colorGroupEnabled: enabled
    }

    QGCViewPanel {
        id:                panel
        anchors.fill:      parent
        enabled:           QGroundControl.videoManager.isMavlinkStream

        Rectangle {
            anchors.fill:                   parent
            color:                          qgcPal.window

            QGCLabel {
                id:                         title
                anchors.margins:            _margins
                anchors.left:               parent.left
                anchors.right:              parent.right
                anchors.top:                parent.top
                horizontalAlignment:        Text.AlignHCenter
                wrapMode:                   Text.WordWrap
                textFormat:                 Text.RichText
                text:                       "Mavlink Stream Configuration"
            }

            Row {
                id:                         streamsRow
                spacing:                    ScreenTools.defaultFontPixelWidth
                anchors.margins:            _margins
                anchors.left:               parent.left
                anchors.right:              parent.right
                anchors.top:                title.bottom

                QGCLabel {
                    id:                     streamsLabel
                    width:                  _labelsWidth
                    height:                 _elementsHeight
                    wrapMode:               Text.WordWrap
                    textFormat:             Text.RichText
                    text:                   "Stream: "
                }

                QGCComboBox {
                    id:                     streamsComboBox
                    width:                  _elementsWidth
                    height:                 _elementsHeight
                    model:                  QGroundControl.videoManager.mavlinkVideoManager.streamList
                    onModelChanged: {
                        currentIndex = QGroundControl.videoManager.mavlinkVideoManager.selectedStream
                    }
                    onActivated: {
                        QGroundControl.videoManager.mavlinkVideoManager.selectedStream = index;
                    }
                    textRole: 'text'
                }
            }

            Row {
                id:                         formatRow
                spacing:                    ScreenTools.defaultFontPixelWidth
                anchors.margins:            _margins
                anchors.left:               parent.left
                anchors.right:              parent.right
                anchors.top:                streamsRow.bottom

                QGCLabel {
                    id:                     formatLabel
                    width:                  _labelsWidth
                    height:                 _elementsHeight
                    wrapMode:               Text.WordWrap
                    textFormat:             Text.RichText
                    text:                   "Format: "
                }

                QGCComboBox {
                    id:                     formatComboBox
                    model:                  QGroundControl.videoManager.mavlinkVideoManager.currentFormatList
                    width:                  _elementsWidth
                    height:                 _elementsHeight
                    onModelChanged: {
                        currentIndex = QGroundControl.videoManager.mavlinkVideoManager.currentFormatIndex
                    }
                    onActivated: {
                        QGroundControl.videoManager.mavlinkVideoManager.currentFormatIndex = index
                    }
                }

            }

            Row {
                id:                         frameSizeRow
                spacing:                    ScreenTools.defaultFontPixelWidth
                anchors.margins:            _margins
                anchors.left:               parent.left
                anchors.right:              parent.right
                anchors.top:                formatRow.bottom

                QGCLabel {
                    id:                     resolutionLabel
                    width:                  _labelsWidth
                    height:                 _elementsHeight
                    wrapMode:               Text.WordWrap
                    textFormat:             Text.RichText
                    text:                   "Resolution: "
                }

                QGCTextField {
                    id:                     frameSizeWidthField
                    width:                  _frameSizeFieldWidth
                    height:                 _elementsHeight
                    anchors.verticalCenter: parent.verticalCenter
                    text:                   QGroundControl.videoManager.mavlinkVideoManager.currentResolutionH
                }

                QGCLabel {
                    id:                     frameSizeLabel
                    width:                  ScreenTools.defaultFontPixelWidth * 2
                    height:                 _elementsHeight
                    horizontalAlignment:    Text.AlignHCenter
                    wrapMode:               Text.WordWrap
                    textFormat:             Text.RichText
                    text:                   "x"
                }

                QGCTextField {
                    id:                     frameSizeHeightField
                    width:                  _frameSizeFieldWidth
                    height:                 _elementsHeight
                    anchors.verticalCenter: parent.verticalCenter
                    text:                   QGroundControl.videoManager.mavlinkVideoManager.currentResolutionV
                }
            }

            Row {
                id:                         uriRow
                spacing:                    ScreenTools.defaultFontPixelWidth
                anchors.margins:            _margins
                anchors.left:               parent.left
                anchors.right:              parent.right
                anchors.top:                frameSizeRow.bottom

                QGCLabel {
                    id:                     uriLabel
                    width:                  _labelsWidth
                    height:                 _elementsHeight
                    wrapMode:               Text.WordWrap
                    textFormat:             Text.RichText
                    text:                   "Uri: "
                }

                QGCTextField {
                    id:                     uriField
                    width:                  _elementsWidth
                    height:                 _elementsHeight
                    anchors.verticalCenter: parent.verticalCenter
                    readOnly:               true
                    text:                   QGroundControl.videoManager.mavlinkVideoManager.currentUri
                }
            }

            Row {
                id:                         buttonsRow
                spacing:                    ScreenTools.defaultFontPixelWidth
                anchors.margins:            _margins
                anchors.top:                uriRow.bottom
                anchors.horizontalCenter:   parent.horizontalCenter

                QGCButton {
                    id:                     refreshButton
                    text:                   "Refresh"
                    width:                  _buttonWidth
                    height:                 _elementsHeight
                    onClicked:              {
                        QGroundControl.videoManager.mavlinkVideoManager.updateCurrentStream()
                    }
                }

                QGCButton {
                    id:                     applyButton
                    text:                   "Apply"
                    width:                  _buttonWidth
                    height:                 _elementsHeight
                    onClicked:              {
                        QGroundControl.videoManager.mavlinkVideoManager.setResolution(parseInt(frameSizeWidthField.text), parseInt(frameSizeHeightField.text))
                    }
                }
            }
        }
    }
}
