import sys

try:
    from PySide6 import QtWidgets
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

from MainWindow import MainWindow
from styles import DARK_STYLESHEET


def main():
    app = QtWidgets.QApplication(sys.argv)
    
    # Apply permanent dark mode stylesheet
    app.setStyleSheet(DARK_STYLESHEET)
    
    w = MainWindow()
    w.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()


