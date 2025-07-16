Add Documentation
==================

This guide explains how to add new content pages to the Read the Docs site for the OpenExo project.

0. First time trying to update ReadTheDocs
------------------------------------------
   - Enter your command prompt
   - Install **Sphinx**::
   
      pip install sphinx

   - Install **Piccolo Theme**::
   
      pip install piccolo-theme

1. Create a new reStructuredText (.rst) file
--------------------------------------------
   - In your source folder (next to `conf.py`), create a file named, for example, `new_page.rst`.
   - At the top of `new_page.rst`, add a title and reference label::
   
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
   - Open the command prompt in the "source" folder (Example: C:\Users\NAME\Desktop\OpenExo\Documentation\ReadTheDocs\source)
   - Build locally::
   
      python -m sphinx -T -b html . _build/html

   - Preview locally::
   
      python -m http.server --directory _build/html/

   - Review changes in browser by typing::

      localhost:8000/index.html

   - If everything looks correct, push to main and **log into Read the Docs and click *Build* (or
     *Trigger Build*) to publish the updated documentation.**

5. Watch the Tutorial Video
-----------------------------
`▶️ Watch the video on YouTube <https://youtu.be/a9WaMBEafLQ?feature=shared>`_
