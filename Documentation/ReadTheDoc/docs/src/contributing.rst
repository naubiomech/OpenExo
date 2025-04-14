Contributing
============

Styles
------

We use `Sass <https://sass-lang.com/>`_ as a CSS preprocessor. The styles live
in ``src/sass``.

To modify the styles, first install Sass:

.. code-block:: bash

    npm install -g sass

Then run:

.. code-block:: bash

    ./scripts/build-styles.sh

This will automatically rebuild your CSS when it detects a change in the
Sass files.

-------------------------------------------------------------------------------

Running the docs
----------------

By running Piccolo Theme's docs you can verify that your changes look OK.

First install the requirements:

.. code-block:: bash

    pip install -r requirements/doc-requirements.txt

Then launch a web server using the following script:

.. code-block:: bash

    ./scripts/run-docs.sh

It auto reloads when it detects changes to the documentation or theme files.
