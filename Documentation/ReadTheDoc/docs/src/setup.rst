Setup
=====

Install Sphinx
--------------

.. code-block:: bash

    pip install sphinx

Create a ``docs`` folder within your project, and run ``sphinx-quickstart``.

.. code-block:: bash

    mkdir docs
    cd docs
    sphinx-quickstart

``sphinx-quickstart`` will scaffold your documentation for you.

-------------------------------------------------------------------------------

Install Piccolo Theme
---------------------

.. code-block:: bash

    pip install piccolo_theme

Find the ``conf.py`` file that Sphinx generated for you. Within that set the
following value:

.. code-block:: python

    html_theme = 'piccolo_theme'

-------------------------------------------------------------------------------

Building your docs
------------------

Sphinx creates a ``Makefile`` in your ``docs`` folder. To generate some HTML
docs, run the following in the same directory as your ``Makefile``:

.. code-block:: bash

    make html

Within your docs folder, Sphinx will have generated some HTML files in
``_build/html``.

To serve these files using Python, you can use:

.. code-block:: bash

    python -m http.server --directory _build/html/

Now open up http://localhost:8000 in your browser to see your beautiful docs!
