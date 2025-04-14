"""
Code snippets for testing autodoc.
"""
import typing as t
from enum import Enum


class Column():
    """
    All other columns inherit from ``Column``. Don't use it directly.

    The following arguments apply to all column types:

    :param null:
        Whether the column is nullable.

    :param primary_key:
        If set, the column is used as a primary key.

    :param default:
        The column value to use if not specified by the user.

    :param unique:
        If set, a unique contraint will be added to the column.

    :param index:
        Whether an index is created for the column, which can improve
        the speed of selects, but can slow down inserts.

    :param index_method:
        If index is set to ``True``, this specifies what type of index is
        created.

    :param required:
        This isn't used by the database - it's to indicate to other tools that
        the user must provide this value. Example uses are in serialisers for
        API endpoints, and form fields.

    :param help_text:
        This provides some context about what the column is being used for. For
        example, for a ``Decimal`` column called ``value``, it could say
        ``'The units are millions of dollars'``. The database doesn't use this
        value, but tools such as Piccolo Admin use it to show a tooltip in the
        GUI.

    :param choices:
        An optional Enum - when specified, other tools such as Piccolo Admin
        will render the available options in the GUI.

    :param db_column_name:
        If specified, you can override the name used for the column in the
        database. The main reason for this is when using a legacy database,
        with a problematic column name (for example ``'class'``, which is a
        reserved Python keyword). Here's an example:

        .. code-block:: python

            class MyTable(Table):
                class_ = Varchar(db_column_name="class")

            >>> await MyTable.select(MyTable.class_)
            [{'id': 1, 'class': 'test'}]

        This is an advanced feature which you should only need in niche
        situations.

    :param secret:
        If ``secret=True`` is specified, it allows a user to automatically
        omit any fields when doing a select query, to help prevent
        inadvertent leakage of sensitive data.

        .. code-block:: python

            class Band(Table):
                name = Varchar()
                net_worth = Integer(secret=True)

            >>> await Band.select(exclude_secrets=True)
            [{'name': 'Pythonistas'}]

    """

    def __init__(
        self,
        null: bool = False,
        primary_key: bool = False,
        unique: bool = False,
        index: bool = False,
        required: bool = False,
        help_text: t.Optional[str] = None,
        choices: t.Optional[t.Type[Enum]] = None,
        db_column_name: t.Optional[str] = None,
        secret: bool = False,
        **kwargs,
    ) -> None:
        pass
