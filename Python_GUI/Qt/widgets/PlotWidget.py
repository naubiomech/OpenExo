try:
    from PySide6 import QtWidgets
    import pyqtgraph as pg
except Exception as e:
    raise SystemExit("PySide6 and pyqtgraph are required")


class PlotWidget(QtWidgets.QWidget):
    """Reusable plot widget with standardized API: set_lines(x, *ys)."""

    def __init__(self, parent=None, title: str = ""):
        super().__init__(parent)
        layout = QtWidgets.QVBoxLayout(self)
        self.plot = pg.PlotWidget()
        self.plot.showGrid(x=True, y=True, alpha=0.3)
        if title:
            self.plot.setTitle(title)
        layout.addWidget(self.plot)
        self.curves = []

    def set_lines(self, x, *ys):
        # Ensure correct number of curves
        while len(self.curves) < len(ys):
            self.curves.append(self.plot.plot())
        for i, y in enumerate(ys):
            self.curves[i].setData(x, y)

