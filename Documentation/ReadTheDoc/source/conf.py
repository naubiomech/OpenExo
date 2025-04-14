# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'OpenExo Documentation'
copyright = '2025, Landon Coonrod, Jack Williams, and Prof. Zach Lerner'
author = 'By Landon Coonrod, Jack Williams, and Prof. Zach Lerner'
release = '0.1-dev'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration


extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.viewcode",
    "sphinx_rtd_theme",  # Not strictly required, but ensures Sphinx sees the theme
]


templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_material"
html_static_path = ['_static']
html_css_files = [
    'custom.css',
]

html_theme_options = {
    # The maximum depth of the global table of contents (on the left).
    'navigation_depth': 4,
    
    # Whether to collapse nested navigation sections.
    'collapse_navigation': False,
    
    # Keeps the navigation pane “fixed” when scrolling.
    'sticky_navigation': True,
    
    # If you only want to show headings for the current page in the left menu, set titles_only=True
    # 'titles_only': True,
}
