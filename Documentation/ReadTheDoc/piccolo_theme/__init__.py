import os
import typing as t

if t.TYPE_CHECKING:
    from sphinx.application import Sphinx


__VERSION__ = "0.24.0"


def setup(app: 'Sphinx') -> t.Dict:
    """
    :param app:
        Passed by Sphinx.

    """

    app.add_html_theme(
        "piccolo_theme", os.path.abspath(os.path.dirname(__file__))
    )

    ###########################################################################
    # Try and infer which Git icon to use based on the URL:

    html_theme_options = getattr(app.config, 'html_theme_options')

    if html_theme_options:
        if isinstance(html_theme_options, dict):
            source_url = html_theme_options.get('source_url')
            source_icon = html_theme_options.get('source_icon')
            if isinstance(source_url, str) and not source_icon:
                if 'github.com' in source_url:
                    html_theme_options['source_icon'] = 'github'
                elif 'gitlab.com' in source_url:
                    html_theme_options['source_icon'] = 'gitlab'
                else:
                    html_theme_options['source_icon'] = 'git'

    ###########################################################################
    # Return metadata
    # https://www.sphinx-doc.org/en/master/extdev/index.html#extension-metadata

    return {
        "version": __VERSION__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
