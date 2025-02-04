'''
Map Editor for the proto engine
Currently to make set maps, but later I'll modify this for however imma do it later
I'm using pyqt, which is new. idk what i'm doing really, but it seems pretty similar to pygame w/ syntax. and tkinter. 

So, the plan here is to just make a simple level editor. And load in the currently drawn level for editing. 
from there, idk. Give the sectors names (numbers) and display them on each sector

I should learn a bit about pyqt before doing this, lol. it'll be weird if not. 
'''

import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QGraphicsView, QGraphicsScene, QGraphicsLineItem
from PyQt5.QtCore import Qt, QPointF
from PyQt5.QtGui import QMouseEvent, QPainter


class LineDrawer(QGraphicsView):
    def __init__(self):
        super().__init__()
        self.scene = QGraphicsScene(self)
        self.setScene(self.scene)
        self.setSceneRect(-500, -500, 1000, 1000)

        # Enable anti-aliasing rendering
        self.setRenderHint(QPainter.Antialiasing, True)

        self.start_point = None
        self.lines = []


    def mousePressEvent(self, event: QMouseEvent):
        if event.button() == Qt.LeftButton:
            self.start_point = self.mapToScene(event.pos())


    def mouseReleaseEvent(self, event: QMouseEvent):
        if event.button() == Qt.LeftButton and self.start_point:
            end_point = self.mapToScene(event.pos())
            line_item = QGraphicsLineItem(self.start_point.x(), self.start_point.y(), end_point.x(), end_point.y())
            self.scene.addItem(line_item)
            self.lines.append((self.start_point, end_point))
            self.start_point = None


    def closeEvent(self, event):
        print("Lines drawn:")
        for start, end in self.lines:
            print(f"Start: ({start.x()}, {start.y()}), End: ({end.x()}, {end.y()})")
        event.accept()

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Map Editor!")
        self.setGeometry(100, 100, 800, 600)
        self.canvas = LineDrawer()
        self.setCentralWidget(self.canvas)

    def closeEvent(self, event):
        self.canvas.closeEvent(event)
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())

