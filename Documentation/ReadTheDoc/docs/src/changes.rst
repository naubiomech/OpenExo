Changes
=======

0.24.0
------

Improved the colours used for syntax highlighting whilst in dark mode (one of
the blue colours was too dark). Thanks to @superstes for reporting this issue.

-------------------------------------------------------------------------------

0.23.0
------

Fixing some style issues in the search results page in Sphinx 7.3.x.

-------------------------------------------------------------------------------

0.22.0
------

Fixed a styling issue with the searchbox in Sphinx 7.3.x. Thanks to @heuer for
reporting this issue.

-------------------------------------------------------------------------------

0.21.0
------

Allow `parallel builds <https://www.sphinx-doc.org/en/master/man/sphinx-build.html#cmdoption-sphinx-build-j>`_.

-------------------------------------------------------------------------------

0.20.0
------

Added additional ordered list styles - thanks to @fizbin for reporting this.

-------------------------------------------------------------------------------

0.19.0
------

Fixed span tags (thanks to @jvcarli for this).

Added support for Python 3.12.

-------------------------------------------------------------------------------

0.18.0
------

Added ``z-index`` for the left sidebar on mobile.

-------------------------------------------------------------------------------

0.17.0
------

Fixes to support Sphinx 7.2. Thanks to @alexlancaster for reporting this.

-------------------------------------------------------------------------------

0.16.1
------

Bundling the Roboto Mono bold-italic font with the theme, as it's used in
some code blocks. Thanks to @noxpardalis for adding this.

-------------------------------------------------------------------------------

0.16.0
------

The custom fonts are now bundled with the theme. Thanks to @noxpardalis for
this.

-------------------------------------------------------------------------------

0.15.0
------

Improved the colours used for showing emphasis in code blocks in dark mode.

Made some minor fixes to the HTML templates.

-------------------------------------------------------------------------------

0.14.0
------

When switching to dark mode, we automatically apply our own custom dark mode
styles to code blocks.

This gives a great experience out of the box. However, if someone uses their
own Pygments theme, they might want to use that theme in both light mode, and
dark mode. They can now do so, using the ``dark_mode_code_blocks`` option.

.. code-block:: python

    # conf.py

    html_theme_options = {
        "dark_mode_code_blocks": False
    }

-------------------------------------------------------------------------------

0.13.0
------

A logo can now be used in the nav bar, instead of text.

.. code-block:: python

    # conf.py

    # Relative to conf.py:
    html_logo = './path/to/logo.png'

    # Or an absolute URL:
    html_logo = 'https://awesome.com/static/logo.png'

Thanks to @are-scenic for adding this.

-------------------------------------------------------------------------------

0.12.0
------

You can now specify the source code URL, and it will show in the nav bar.

.. code-block:: python

    # conf.py

    html_theme_options = {
        "source_url": 'https://github.com/piccolo-orm/piccolo_theme/'
    }

The icon is inferred automatically based on the URL (in the above example, we
show the GitHub logo). You can explicitly set the icon if you prefer:

.. code-block:: python

    # conf.py

    html_theme_options = {
        "source_url": 'https://self-hosted.foo.com/',
        "source_icon": "gitlab"
    }

-------------------------------------------------------------------------------

0.11.1
------

Minor style fix on search page.

-------------------------------------------------------------------------------

0.11.0
------

Fixed some styles in Sphinx v5.

-------------------------------------------------------------------------------

0.10.2
------

Drop Python 3.7 specific syntax.

-------------------------------------------------------------------------------

0.10.1
------

Fix typo in ``setup.py``.

-------------------------------------------------------------------------------

0.10.0
------

Added support for Python 3.6, as many Ubuntu systems will still be using that
version, and Sphinx still supports it. Thanks to @oncilla for reporting this
issue.

-------------------------------------------------------------------------------

0.9.0
-----

Improved the appearance of autodoc output for C files (when using
`breathe <https://breathe.readthedocs.io/en/latest/>`_). Courtesy @thijsmie.

-------------------------------------------------------------------------------

0.8.1
-----

Changed the arrow symbols - they didn't look great on mobile.

-------------------------------------------------------------------------------

0.8.0
-----

Added spacing between sections, so it's not necessary to add horizontal
dividers any more.

.. code-block:: rst

    My Heading
    ==========

    Section 1
    ---------

    Some content

    -------------------------------------------

    Section 2
    ---------

    Some content

We can now just do:

.. code-block:: rst

    My Heading
    ==========

    Section 1
    ---------

    Some content


    Section 2
    ---------

    Some content

Other minor changes:

* Using unicode triangle character instead of < for some links
* Plain admonitions are now styled properly:

.. code-block:: rst

  .. admonition:: A custom admonition

     This is my custom admonition!

-------------------------------------------------------------------------------

0.7.1
-----

Improvements to the notification feature - it was causing too many browser
reflow operations.

-------------------------------------------------------------------------------

0.7.0
-----

A notification can now be shown at the top of each page.

.. code-block:: python

    # conf.py
    html_theme_options = {
        "banner_text": 'Welcome to our amazing documentation!',
        "banner_hiding": "permanent"
    }

This involved quite a few CSS changes - please clear your browser cache if
anything appears broken.

-------------------------------------------------------------------------------

0.6.0
-----

If ``html_short_title`` is in ``conf.py`` then this is used in the nav bar
instead of the full project title.

-------------------------------------------------------------------------------

0.5.1
-----

Fixed dark mode styles - some elements weren't visible. Thanks to @alorence for
reporting this issue.

-------------------------------------------------------------------------------

0.5.0
-----

Added table styles.

-------------------------------------------------------------------------------

0.4.0
-----

Improved the appearance of autodoc output for C++ files (when using
`breathe <https://breathe.readthedocs.io/en/latest/>`_). Courtesy @thijsmie.

-------------------------------------------------------------------------------

0.3.0
-----

Added dark mode.

-------------------------------------------------------------------------------

0.2.5
-----

Improved search styles.

-------------------------------------------------------------------------------

0.2.4
-----

Added missing ``requirements.txt`` file to manifest. Thanks to @moorepants for
reporting this.

-------------------------------------------------------------------------------

0.2.3
-----
Make the ``page contents`` text smaller when the right hand sidebar is hidden.

-------------------------------------------------------------------------------

0.2.2
-----
Fix missing static files.

-------------------------------------------------------------------------------

0.2.1
-----
Fix missing static files.

-------------------------------------------------------------------------------

0.2.0
-----

Improved the main header on mobile - the search bar is replaced with a search
icon. Also increased the size of the touch targets for showing / hiding the
right sidebar, for easier use on mobile. See `PR 7 <https://github.com/piccolo-orm/piccolo_theme/pull/7>`_.
