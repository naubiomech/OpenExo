Configuration
=============

``html_short_title``
--------------------

If you have a really long project name, you may prefer something shorter to
appear in the navigation bar. Specify this using ``html_short_title`` in
``conf.py``:

.. code-block:: python

    # conf.py

    # By default the project value is used in the nav bar.
    project = 'My Extra Special Amazing Docs'

    # If specified, this will be used in the nav bar instead.
    html_short_title = "Amazing Docs"

``html_logo``
-------------

If you want to use a logo in the nav bar instead of text, specify ``html_logo``
in ``conf.py``:

.. code-block:: python

    # conf.py

    # It can either be a path to an image, relative to conf.py:
    html_logo = './static/logo.png'

    # Or it can be a URL:
    html_logo = 'https://awesome.com/static/logo.png'

.. _PygmentsStyle:

``pygments_style``
------------------

We use the ``default`` Pygments theme for syntax highlighting of code blocks.
It gives good results out of the box (including great dark mode support).

If you'd prefer to use a different Pygments style, you can specify it using
``pygments_style`` in ``conf.py``:

.. code-block:: python

    # conf.py

    pygments_style = "stata-dark"

Dark Mode
~~~~~~~~~

When switching to dark mode, we automatically apply our own dark mode styles to
code blocks. If you'd like to disable this behaviour, see :ref:`DarkModeCodeBlocks`.

-------------------------------------------------------------------------------

Theme specific
--------------

``banner_text``
~~~~~~~~~~~~~~~

If this is provided then a banner is shown at the top of the page. It's useful
for important announcements.

.. code-block:: python

    # conf.py

    html_theme_options = {
        # Note how we can include links:
        "banner_text": 'We just launched a newletter, <a href="https://mynewsletter.com/">please subscribe</a>!'
    }


``banner_hiding``
~~~~~~~~~~~~~~~~~

This controls how the banner behaves when hidden. The options are
``'temporary'`` (the default) or ``'permanent'``.

If ``'temporary'``, when a user hides the banner they can still reopen it again.
This is useful if you want to store important information in the banner, which
the user may need to refer to again. For example:

.. code-block:: python

    # conf.py

    html_theme_options = {
        "banner_text": 'Please be aware of security issue XYZ!',
        "banner_hiding": "temporary"
    }

If ``'permanent'``, when a user hides the banner it disappears permanently. This is
useful when the banner contains information which the user is unlikely to
need again. For example:

.. code-block:: python

    # conf.py

    html_theme_options = {
        "banner_text": 'Welcome to our amazing documentation!',
        "banner_hiding": "permanent"
    }

.. note:: If you configure a different ``banner_text`` value in the future,
   then the banner will appear again, even if the user has previously hidden
   it.

.. _DarkModeCodeBlocks:

``dark_mode_code_blocks``
~~~~~~~~~~~~~~~~~~~~~~~~~

When switching to dark mode, we apply our own custom CSS styles to code blocks.
This gives a great dark mode experience out of the box.

However, if you've specified a custom Pygments theme (see :ref:`PygmentsStyle`),
and you want to use that theme for both light mode and dark mode, you can
disable our custom dark mode styles:

.. code-block:: python

    # conf.py

    html_theme_options = {
        "dark_mode_code_blocks": False,
    }

``globaltoc_collapse``
~~~~~~~~~~~~~~~~~~~~~~

By default, the sidebar just shows the top level:

.. image:: static/sidebar/default.png
    :width: 200px

When you click on an item, it shows the children:

.. image:: static/sidebar/clicked.png
    :width: 200px

If you want the children to be visible at all times, you can do so as follows:

.. code-block:: python

    # conf.py

    html_theme_options = {
        "globaltoc_collapse": False
    }

It will then look something like this:

.. image:: static/sidebar/expanded.png
    :width: 200px

``show_theme_credit``
~~~~~~~~~~~~~~~~~~~~~

At the bottom of the page is a very small link which says ``Styled using the Piccolo Theme``.

This helps grow awareness of the project, and attract new contributors.

You can hide this if required:

.. code-block:: python

    # conf.py

    html_theme_options = {
        "show_theme_credit": False
    }

If hiding it, please consider :ref:`supporting us <SupportUs>` in a different way.

``source_url``
~~~~~~~~~~~~~~

If specified, a link is shown in the nav bar to the source code.

.. code-block:: python

    # conf.py

    html_theme_options = {
        "source_url": 'https://github.com/piccolo-orm/piccolo_theme/'
    }

We try and detect if the URL points to GitHub or GitLab, and show the correct
icon. However, if you're using a self hosted version of GitHub or GitLab on a
custom URL, you can explicitly tell the theme which icon to use:

.. code-block:: python

    # conf.py

    html_theme_options = {
        "source_url": 'https://self-hosted.foo.com/',
        "source_icon": "gitlab"
    }

The available options for ``source_icon`` are:

* generic
* github
* gitlab
