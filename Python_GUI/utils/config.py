"""
Configuration constants for OpenExo Qt GUI.
Centralizes all magic numbers, colors, and mappings.
"""


class UIConfig:
    """UI configuration constants."""
    
    # Font sizes (points)
    FONT_TITLE = 22
    FONT_TITLE_LARGE = 20
    FONT_SUBTITLE = 16
    FONT_LARGE = 18
    FONT_MEDIUM = 14
    FONT_SMALL = 12
    FONT_TINY = 10
    
    # Colors (hex)
    COLOR_CRITICAL = "#D32F2F"       # Red - for critical actions (End Trial)
    COLOR_ACTION = "#1976D2"         # Blue - for primary actions (Pause)
    COLOR_SUCCESS = "#4CAF50"        # Green - for success states (Battery good)
    COLOR_WARNING = "#FF5252"        # Light Red - for warnings (Battery low)
    COLOR_SEPARATOR = "#555555"      # Gray - for visual separators
    COLOR_LABEL = "#AAAAAA"          # Light Gray - for section labels
    
    # Button heights (pixels)
    BTN_HEIGHT_SMALL = 38
    BTN_HEIGHT_MEDIUM = 44
    BTN_HEIGHT_LARGE = 48
    BTN_HEIGHT_XLARGE = 56
    
    # Button widths (pixels)
    BTN_WIDTH_SMALL = 160
    BTN_WIDTH_MEDIUM = 200
    
    # Spacing (pixels)
    SPACING_TINY = 2
    SPACING_SMALL = 4
    SPACING_MEDIUM = 6
    SPACING_LARGE = 8
    SPACING_XLARGE = 10
    SPACING_XXLARGE = 12
    SPACING_SECTION = 20
    SPACING_HEADER = 35
    
    # Margins
    MARGIN_PAGE = 8
    MARGIN_FORM = 16
    
    # Logo sizes
    LOGO_OPENEXO_WIDTH = 160
    LOGO_OPENEXO_HEIGHT = 40
    LOGO_OPENEXO_SMALL_WIDTH = 140
    LOGO_OPENEXO_SMALL_HEIGHT = 28
    LOGO_LAB_WIDTH = 200
    LOGO_LAB_HEIGHT = 32
    
    # Window sizes
    WINDOW_MIN_WIDTH = 700
    WINDOW_MIN_HEIGHT = 400
    WINDOW_DEFAULT_WIDTH = 900
    WINDOW_DEFAULT_HEIGHT = 500
    
    # Device list
    LIST_DEVICE_MIN_HEIGHT = 100
    LIST_ITEM_HEIGHT = 36
    
    # Table
    TABLE_ROW_HEIGHT = 44
    TABLE_COL_WIDTH = 140
    
    # Battery thresholds
    BATTERY_LOW_VOLTAGE = 11.0


class JointConfig:
    """Joint configuration and mappings."""
    
    # Joint ID to joint number mapping (matches QtExoDeviceManager.jointDictionary)
    # Used by ActiveTrialSettingsPage for dynamic controller updates
    ID_TO_NUM = {
        33: 1,  # Left Hip
        65: 2,  # Right Hip
        34: 3,  # Left Knee
        66: 4,  # Right Knee
        36: 5,  # Left Ankle
        68: 6,  # Right Ankle
        40: 7,  # Left Elbow
        72: 8,  # Right Elbow
    }
    
    # Legacy joint names (for ActiveTrialBasicSettingsPage)
    JOINT_NAMES = [
        "Left hip",
        "Left knee",
        "Left ankle",
        "Left elbow",
        "Right hip",
        "Right knee",
        "Right ankle",
        "Right elbow",
    ]
    
    # Legacy joint name to index mapping
    # Used by ActiveTrialBasicSettingsPage for legacy firmware compatibility
    NAME_TO_INDEX = {
        "Right hip": 1,
        "Left hip": 2,
        "Right knee": 3,
        "Left knee": 4,
        "Right ankle": 5,
        "Left ankle": 6,
        "Right elbow": 7,
        "Left elbow": 8,
    }


class PlotConfig:
    """Plot configuration."""
    
    # Update rate
    RATE_HZ = 30
    WINDOW_SECS = 10
    
    # Curve colors (pyqtgraph format)
    COLOR_CONTROLLER = 'b'  # Blue
    COLOR_MEASUREMENT = 'r'  # Red
    COLOR_SIGNAL_A = 'g'     # Green
    COLOR_SIGNAL_B = 'm'     # Magenta
    
    # Curve width
    CURVE_WIDTH = 2
    
    # Grid alpha
    GRID_ALPHA = 0.3
