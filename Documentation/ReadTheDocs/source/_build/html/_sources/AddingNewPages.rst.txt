Add Documentation
==================

This guide explains how to add new content pages to the Read the Docs site for the OpenExo project.

1. Create a new reStructuredText file
--------------------------------------
   - In your source folder (next to `conf.py`), create a file named, for example, `new_page.rst`.
   - At the top of `new_page.rst`, add a title and reference label:

     .. _new_page:

     My New Page Title
     =================

   - Write your content below the title.

2. Include the new page in the table of contents
------------------------------------------------
   - Open `index.rst` located in the same `source/` directory.
   - Locate the `.. toctree::` directive (usually after the main title).
   - Add an entry for your new file (without the `.rst` extension), for example:

   Add to your *index.rst*::


     .. toctree::
        :maxdepth: 2
        :caption: Contents:

        introduction
        installation
        new_page       <-- add this line
        usage

3. Update `conf.py` if needed
-----------------------------
   - If your new page requires additional Sphinx extensions, install them and add to the `extensions` list.
   - For custom styling, you can adjust theme options or add CSS files under the `_static` folder and reference them via `html_static_path`.

4. Build and preview locally
-----------------------------
.. code-block:: bash

   cd C:\Users\user\OpenExo\Documentation\ReadTheDocs
   make html
   start build\html\index.html   # open the docs in your browser

- If everything looks correct, **log into Read the Docs and click *Build* (or
  *Trigger Build*) to publish the updated documentation.**