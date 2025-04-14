e2e tests
=========

We use Playwright to make sure the theme works on multiple versions of Sphinx.

It is run automatically in GitHub Actions.

To run it locally:

.. code-block:: bash

    pip install -r requirements/e2e-requirements.txt
    playwright install chromium

Make sure the local docs have been built, as this is what we test against:

.. code-block:: bash

    cd docs
    make html

Then from the root of the project, run the Playwright tests:

.. code-block:: bash

    ./scripts/run-e2e-tests.sh

You can see the tests run in the browser using the ``--headed`` flag:

.. code-block:: bash

    ./scripts/run-e2e-tests.sh --headed
