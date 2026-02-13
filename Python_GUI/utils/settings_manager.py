"""
Centralized settings persistence manager.
Handles loading and saving GUI settings to avoid code duplication.
"""

import os


class SettingsManager:
    """Manages persistent GUI settings."""
    
    DEFAULT_FILENAME = "gui_settings.txt"
    
    @staticmethod
    def get_settings_path(filename=None):
        """Get the full path to the settings file."""
        if filename is None:
            filename = SettingsManager.DEFAULT_FILENAME
        
        # Get base directory (Python_GUI/)
        base_dir = os.path.dirname(os.path.dirname(__file__))
        save_dir = os.path.join(base_dir, "Saved_Data")
        return os.path.join(save_dir, filename)
    
    @staticmethod
    def load_settings(filename=None):
        """
        Load settings from file.
        
        Returns:
            dict: Settings as key-value pairs, empty dict if file doesn't exist
        """
        settings_file = SettingsManager.get_settings_path(filename)
        settings = {}
        
        try:
            if os.path.exists(settings_file):
                with open(settings_file, 'r') as f:
                    for line in f.readlines():
                        line = line.strip()
                        if '=' in line:
                            key, val = line.split('=', 1)
                            settings[key] = val
        except Exception as e:
            print(f"[SettingsManager] Error loading settings: {e}")
        
        return settings
    
    @staticmethod
    def save_settings(settings, filename=None):
        """
        Save settings to file.
        
        Args:
            settings (dict): Settings as key-value pairs
            filename (str): Optional filename, defaults to gui_settings.txt
        """
        settings_file = SettingsManager.get_settings_path(filename)
        
        try:
            # Ensure directory exists
            save_dir = os.path.dirname(settings_file)
            os.makedirs(save_dir, exist_ok=True)
            
            # Write settings
            with open(settings_file, 'w') as f:
                for key, val in settings.items():
                    f.write(f"{key}={val}\n")
            
            print(f"[SettingsManager] Saved settings to {settings_file}")
        except Exception as e:
            print(f"[SettingsManager] Error saving settings: {e}")
    
    @staticmethod
    def update_settings(updates, filename=None):
        """
        Update specific settings while preserving others.
        
        Args:
            updates (dict): Settings to update
            filename (str): Optional filename
        """
        # Load existing settings
        settings = SettingsManager.load_settings(filename)
        
        # Update with new values
        settings.update(updates)
        
        # Save back
        SettingsManager.save_settings(settings, filename)
    
    @staticmethod
    def get_setting(key, default=None, filename=None):
        """
        Get a single setting value.
        
        Args:
            key (str): Setting key
            default: Default value if not found
            filename (str): Optional filename
            
        Returns:
            Setting value or default
        """
        settings = SettingsManager.load_settings(filename)
        return settings.get(key, default)
    
    @staticmethod
    def get_bool(key, default=False, filename=None):
        """Get a boolean setting."""
        val = SettingsManager.get_setting(key, default, filename)
        if isinstance(val, str):
            return val.lower() == "true"
        return bool(val)
    
    @staticmethod
    def get_int(key, default=0, filename=None):
        """Get an integer setting."""
        val = SettingsManager.get_setting(key, default, filename)
        try:
            return int(val)
        except (ValueError, TypeError):
            return default
    
    @staticmethod
    def get_float(key, default=0.0, filename=None):
        """Get a float setting."""
        val = SettingsManager.get_setting(key, default, filename)
        try:
            return float(val)
        except (ValueError, TypeError):
            return default
