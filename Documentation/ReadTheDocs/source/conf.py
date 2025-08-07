import piccolo_theme  # This import may help, but isn't strictly required.

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "OpenExo"
copyright = '2025, Landon Coonrod, Jack Williams, and Prof. Zach Lerner'
author = 'By Landon Coonrod, Jack Williams, and Prof. Zach Lerner'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration


extensions = []


templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output
html_title       = "OpenExo Documentation"
html_short_title = "OpenExo"
html_theme = 'piccolo_theme'
html_static_path = ['_static']
html_css_files = [
    'custom.css',
]

html_theme_options = {
}