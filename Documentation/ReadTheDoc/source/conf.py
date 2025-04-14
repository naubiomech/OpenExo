import piccolo_theme

# -- Project information -----------------------------------------------------
project = 'OpenExo Documentation'
copyright = '2025, Landon Coonrod, Jack Williams, and Prof. Zach Lerner'
author = 'By Landon Coonrod, Jack Williams, and Prof. Zach Lerner'
release = '0.1-dev'

# -- General configuration ---------------------------------------------------
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.viewcode",
    # Removed "sphinx_rtd_theme" as it's not needed with piccolo_theme.
]

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
html_theme = "piccolo_theme"
# Remove the following line because piccolo_theme does not provide get_html_theme_path()
# html_theme_path = [piccolo_theme.get_html_theme_path()]

html_static_path = ['_static']
html_css_files = [
    'custom.css',
]

html_theme_options = {
    # Add any piccolo_theme specific options here if needed.
}
