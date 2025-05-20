# This Python file uses the following encoding: utf-8
import sys
from PySide6 import QtWidgets, QtCore
from PySide6.QtWidgets import QApplication, QWidget
#import random


class Widget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.layout = QtWidgets.QGridLayout(self)

        DELTA = 1
        buttons_dict = {
            '←' : {
                'delta':{
                    'x': -DELTA,
                    'y': 0
                },
                'grid_pos':{
                    'row': 1,
                    'col': 0
                }
            },
            '→' : {
                'delta':{
                    'x': DELTA,
                    'y': 0
                },
                'grid_pos':{
                    'row': 1,
                    'col': 2
                }
            },
            '↑' : {
                'delta':{
                    'x': 0,
                    'y': DELTA
                },
                'grid_pos':{
                    'row': 0,
                    'col': 1
                }
            },
            '↓' : {
                'delta':{
                    'x': 0,
                    'y': -DELTA
                },
                'grid_pos':{
                    'row': 2,
                    'col': 1
                }
            },
        }
        for b in buttons_dict.keys():
            bObj = buttons_dict[b]
            bDelta = bObj['delta']
            bGridPos = bObj['grid_pos']

            newButton = MoveButton(
            b,
            bDelta['x'],
            bDelta['y']
            )

            self.layout.addWidget(newButton.button, bGridPos['row'], bGridPos['col'])
            newButton.button.clicked.connect(lambda _, dx=newButton.deltaX, dy=newButton.deltaY: self.move_window(dx, dy))

    @QtCore.Slot()
    def move_window(self, x, y):
        pos = self.pos()
        for i in range(100):
            self.move(pos.x() + i*x, pos.y() - i*y)

class MoveButton:
    def __init__(self, text, deltaX, deltaY):
        SIZE = 70
        self.button = QtWidgets.QPushButton(text)
        self.button.setFixedSize(SIZE, SIZE)
        self.button.setStyleSheet("font-size: 40px; font-weight: bold;")

        self.deltaX = deltaX
        self.deltaY = deltaY

if __name__ == "__main__":
    app = QApplication([])
    window = Widget()
    window.show()
    sys.exit(app.exec())
