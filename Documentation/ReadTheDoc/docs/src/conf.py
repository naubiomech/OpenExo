# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import datetime
import os
import sys

import sphinx

sys.path.insert(0, os.path.abspath('../..'))


# -- Project information -----------------------------------------------------

project = 'Piccolo Theme'
year = datetime.datetime.now().year
copyright = f'{year}, Daniel Townsend'
author = 'Daniel Townsend'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.autodoc',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'piccolo_theme'
html_short_title = 'Piccolo Theme'
# Enabling this line will change the nav title from a text to an image:
# html_logo = './static/awesome_logo.png'

# To experiment with custom code block styles:
# pygments_style = "stata-dark"

html_theme_options = {
    "banner_text": (
        "This is an example notification banner - welcome to the Piccolo "
        "Theme!"
    ),
    "banner_hiding": "permanent",
    "show_theme_credit": True,
    "source_url": "https://github.com/piccolo-orm/piccolo_theme/",
    "dark_mode_code_blocks": True
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = []

if sphinx.version_info[0] < 7:
    # Breathe only works on certain Sphinx versions.

    extensions.append('breathe')

    breathe_projects = {
        "cpp_demo": "./cpp_breathe_demo",
        "c_demo": "./c_breathe_demo"
    }
    breathe_default_project = "cpp_demo"
    breathe_domain_by_extension = {
        "h": "c",
        "c": "c",
        "hpp": "cpp",
        "cpp": "cpp"
    }

    # By using tags, we can exclude the breathe examples from the docs using
    # the ``only`` directive.
    tags.add('include_breathe')  # noqa  # type: ignore
else:
    tags.add('exclude_breathe')  # noqa  # type: ignore
