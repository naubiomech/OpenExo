"""
UI helper functions for OpenExo Qt GUI.
Reduces code duplication across page files.
"""

import os

try:
    from PySide6 import QtCore, QtWidgets, QtGui
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

from .config import UIConfig


def get_base_dir():
    """
    Get the base directory path (Python_GUI/).
    
    Returns:
        str: Absolute path to Python_GUI directory
    """
    return os.path.dirname(os.path.dirname(__file__))


def get_image_path(image_name):
    """
    Get the full path to an image file.
    
    Args:
        image_name (str): Name of the image file (e.g., "OpenExo.png")
        
    Returns:
        str: Full path to the image file
    """
    base_dir = get_base_dir()
    return os.path.join(base_dir, "Images", image_name)


def load_logo(image_name, width, height, alignment=None):
    """
    Load and scale a logo image into a QLabel.
    
    Args:
        image_name (str): Name of the image file (e.g., "OpenExo.png")
        width (int): Target width in pixels
        height (int): Target height in pixels
        alignment: Qt alignment flags (default: None)
        
    Returns:
        QtWidgets.QLabel: Label containing the scaled logo, or None if loading fails
    """
    try:
        label = QtWidgets.QLabel()
        label.setContentsMargins(0, 0, 0, 0)
        
        logo_path = get_image_path(image_name)
        pixmap = QtGui.QPixmap(logo_path)
        
        if not pixmap.isNull():
            scaled_pixmap = pixmap.scaled(
                width, height,
                QtCore.Qt.KeepAspectRatio,
                QtCore.Qt.SmoothTransformation
            )
            label.setPixmap(scaled_pixmap)
            
            if alignment:
                label.setAlignment(alignment)
            
            return label
        else:
            print(f"[UI] Failed to load logo from: {logo_path}")
            return None
    except Exception as e:
        print(f"[UI] Error loading logo '{image_name}': {e}")
        return None


def style_button(button, height=None, width=None, font_size=None, padding=None):
    """
    Apply consistent styling to a button.
    
    Args:
        button (QtWidgets.QPushButton): Button to style
        height (int): Minimum height in pixels (optional)
        width (int): Minimum width in pixels (optional)
        font_size (int): Font size in points (optional)
        padding (str): CSS padding string (optional, e.g., "8px 12px")
    """
    if height is not None:
        button.setMinimumHeight(height)
    
    if width is not None:
        button.setMinimumWidth(width)
    
    if font_size is not None:
        font = button.font()
        font.setPointSize(font_size)
        button.setFont(font)
    
    if padding is not None:
        button.setStyleSheet(f"padding: {padding};")


def style_label(label, font_size=None, bold=False, color=None):
    """
    Apply consistent styling to a label.
    
    Args:
        label (QtWidgets.QLabel): Label to style
        font_size (int): Font size in points (optional)
        bold (bool): Whether to make font bold
        color (str): Text color in hex format (optional, e.g., "#FFFFFF")
    """
    font = label.font()
    
    if font_size is not None:
        font.setPointSize(font_size)
    
    if bold:
        font.setBold(True)
    
    label.setFont(font)
    
    if color is not None:
        label.setStyleSheet(f"color: {color};")


def style_combo_box(combo, height=None, font_size=None):
    """
    Apply consistent styling to a combo box.
    
    Args:
        combo (QtWidgets.QComboBox): Combo box to style
        height (int): Minimum height in pixels (optional)
        font_size (int): Font size in points (optional)
    """
    if height is not None:
        combo.setMinimumHeight(height)
    
    if font_size is not None:
        font = combo.font()
        font.setPointSize(font_size)
        combo.setFont(font)


def style_spinbox(spinbox, height=None, font_size=None):
    """
    Apply consistent styling to a spin box.
    
    Args:
        spinbox (QtWidgets.QDoubleSpinBox or QtWidgets.QSpinBox): Spin box to style
        height (int): Minimum height in pixels (optional)
        font_size (int): Font size in points (optional)
    """
    if height is not None:
        spinbox.setMinimumHeight(height)
    
    if font_size is not None:
        font = spinbox.font()
        font.setPointSize(font_size)
        spinbox.setFont(font)


def create_separator(orientation='horizontal', color=None):
    """
    Create a visual separator line.
    
    Args:
        orientation (str): 'horizontal' or 'vertical'
        color (str): Line color in hex format (default: UIConfig.COLOR_SEPARATOR)
        
    Returns:
        QtWidgets.QFrame: Separator frame
    """
    separator = QtWidgets.QFrame()
    
    if orientation == 'horizontal':
        separator.setFrameShape(QtWidgets.QFrame.HLine)
    else:
        separator.setFrameShape(QtWidgets.QFrame.VLine)
    
    if color is None:
        color = UIConfig.COLOR_SEPARATOR
    
    separator.setStyleSheet(f"background-color: {color};")
    
    return separator


def create_section_label(text, font_size=None, color=None):
    """
    Create a styled section label.
    
    Args:
        text (str): Label text
        font_size (int): Font size in points (default: UIConfig.FONT_TINY)
        color (str): Text color (default: UIConfig.COLOR_LABEL)
        
    Returns:
        QtWidgets.QLabel: Styled label
    """
    label = QtWidgets.QLabel(text)
    
    if font_size is None:
        font_size = UIConfig.FONT_TINY
    
    if color is None:
        color = UIConfig.COLOR_LABEL
    
    style_label(label, font_size=font_size, bold=True, color=color)
    
    return label


def apply_button_style_batch(buttons, height=None, width=None, font_size=None, padding=None):
    """
    Apply consistent styling to multiple buttons at once.
    
    Args:
        buttons (list): List of QPushButton widgets
        height (int): Minimum height in pixels (optional)
        width (int): Minimum width in pixels (optional)
        font_size (int): Font size in points (optional)
        padding (str): CSS padding string (optional)
    """
    for button in buttons:
        style_button(button, height, width, font_size, padding)


def set_size_policy_expanding(widget):
    """Set widget size policy to expand in both directions."""
    widget.setSizePolicy(
        QtWidgets.QSizePolicy.Expanding,
        QtWidgets.QSizePolicy.Expanding
    )


def set_size_policy_fixed_height(widget):
    """Set widget size policy to expand horizontally but fixed height."""
    widget.setSizePolicy(
        QtWidgets.QSizePolicy.Expanding,
        QtWidgets.QSizePolicy.Fixed
    )
