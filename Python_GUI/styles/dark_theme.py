"""
Dark mode stylesheet for OpenExo Qt GUI.
Professional and futuristic theme with robotic aesthetics.
"""

DARK_STYLESHEET = """
    QWidget {
        background-color: #1E1E1E;
        color: #E0E0E0;
        font-family: 'Segoe UI', Arial, sans-serif;
    }
    
    QMainWindow {
        background-color: #1E1E1E;
    }
    
    QPushButton {
        background-color: #2D2D2D;
        color: #FFFFFF;
        border: 1px solid #3D3D3D;
        border-radius: 4px;
        padding: 8px;
        font-size: 11pt;
    }
    
    QPushButton:hover {
        background-color: #3D3D3D;
        border: 1px solid #4D4D4D;
    }
    
    QPushButton:pressed {
        background-color: #252525;
    }
    
    QPushButton:disabled {
        background-color: #252525;
        color: #666666;
    }
    
    QLabel {
        background-color: transparent;
        color: #E0E0E0;
    }
    
    QLineEdit, QSpinBox, QDoubleSpinBox {
        background-color: #2D2D2D;
        color: #FFFFFF;
        border: 1px solid #3D3D3D;
        border-radius: 4px;
        padding: 6px;
    }
    
    QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
        border: 1px solid #0078D4;
    }
    
    QComboBox {
        background-color: #2D2D2D;
        color: #FFFFFF;
        border: 1px solid #3D3D3D;
        border-radius: 4px;
        padding: 6px;
    }
    
    QComboBox:hover {
        border: 1px solid #4D4D4D;
    }
    
    QComboBox::drop-down {
        border: none;
    }
    
    QComboBox QAbstractItemView {
        background-color: #2D2D2D;
        color: #FFFFFF;
        selection-background-color: #0078D4;
    }
    
    QListWidget {
        background-color: #2D2D2D;
        color: #FFFFFF;
        border: 1px solid #3D3D3D;
        border-radius: 4px;
    }
    
    QListWidget::item:selected {
        background-color: #0078D4;
    }
    
    QListWidget::item:hover {
        background-color: #3D3D3D;
    }
    
    QTableWidget {
        background-color: #2D2D2D;
        color: #FFFFFF;
        gridline-color: #3D3D3D;
        border: 1px solid #3D3D3D;
    }
    
    QTableWidget::item:selected {
        background-color: #0078D4;
    }
    
    QHeaderView::section {
        background-color: #252525;
        color: #FFFFFF;
        padding: 6px;
        border: 1px solid #3D3D3D;
    }
    
    QScrollBar:vertical {
        background-color: #2D2D2D;
        width: 14px;
        border-radius: 7px;
    }
    
    QScrollBar::handle:vertical {
        background-color: #4D4D4D;
        border-radius: 7px;
        min-height: 30px;
    }
    
    QScrollBar::handle:vertical:hover {
        background-color: #5D5D5D;
    }
    
    QScrollBar:horizontal {
        background-color: #2D2D2D;
        height: 14px;
        border-radius: 7px;
    }
    
    QScrollBar::handle:horizontal {
        background-color: #4D4D4D;
        border-radius: 7px;
        min-width: 30px;
    }
    
    QScrollBar::handle:horizontal:hover {
        background-color: #5D5D5D;
    }
    
    QCheckBox {
        color: #E0E0E0;
        spacing: 8px;
    }
    
    QCheckBox::indicator {
        width: 18px;
        height: 18px;
        border: 1px solid #3D3D3D;
        border-radius: 3px;
        background-color: #2D2D2D;
    }
    
    QCheckBox::indicator:checked {
        background-color: #0078D4;
        border: 1px solid #0078D4;
    }
    
    QRadioButton {
        color: #E0E0E0;
        spacing: 8px;
    }
    
    QRadioButton::indicator {
        width: 18px;
        height: 18px;
        border: 1px solid #3D3D3D;
        border-radius: 9px;
        background-color: #2D2D2D;
    }
    
    QRadioButton::indicator:checked {
        background-color: #0078D4;
        border: 1px solid #0078D4;
    }
    
    QGroupBox {
        color: #E0E0E0;
        border: 1px solid #3D3D3D;
        border-radius: 6px;
        margin-top: 12px;
        padding-top: 12px;
    }
    
    QGroupBox::title {
        subcontrol-origin: margin;
        subcontrol-position: top left;
        padding: 4px 8px;
        color: #FFFFFF;
    }
    
    QTabWidget::pane {
        border: 1px solid #3D3D3D;
        background-color: #1E1E1E;
    }
    
    QTabBar::tab {
        background-color: #2D2D2D;
        color: #E0E0E0;
        padding: 8px 16px;
        border: 1px solid #3D3D3D;
        border-bottom: none;
        border-top-left-radius: 4px;
        border-top-right-radius: 4px;
    }
    
    QTabBar::tab:selected {
        background-color: #1E1E1E;
        color: #FFFFFF;
    }
    
    QTabBar::tab:hover {
        background-color: #3D3D3D;
    }
    
    QMessageBox {
        background-color: #1E1E1E;
    }
    
    QMessageBox QLabel {
        color: #E0E0E0;
    }
    
    QMessageBox QPushButton {
        min-width: 80px;
    }
"""
