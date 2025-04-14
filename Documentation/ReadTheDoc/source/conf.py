import piccolo_theme  # Import the Piccolo Theme

# -- Project information -----------------------------------------------------
project = 'OpenExo Documentation'
copyright = '2025, Landon Coonrod, Jack Williams, and Prof. Zach Lerner'
author = 'By Landon Coonrod, Jack Williams, and Prof. Zach Lerner'
release = '0.1-dev'

# -- General configuration ---------------------------------------------------
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.viewcode",
    # Removed "sphinx_rtd_theme" since we’re using piccolo_theme.
]

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
html_theme = "piccolo_theme"
# No need to set html_theme_path: Sphinx will find the theme via entry points.
html_static_path = ['_static']
html_css_files = [
    'custom.css',
]

# Optional: Add any Piccolo Theme specific options here if desired.
html_theme_options = {
    # For example, you might set a navigation title if supported:
    # 'nav_title': "OpenExo Documentation",
}
