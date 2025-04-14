#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import typing as t

from setuptools import find_packages, setup

from piccolo_theme import __VERSION__ as VERSION

directory = os.path.abspath(os.path.dirname(__file__))


with open(os.path.join(directory, "README.md")) as f:
    LONG_DESCRIPTION = f.read()


def parse_requirement(req_path: str) -> t.List[str]:
    with open(os.path.join(directory, "requirements", req_path)) as f:
        contents = f.read()
        return [i.strip() for i in contents.strip().split("\n")]


setup(
    name="piccolo_theme",
    version=VERSION,
    description="A modern Sphinx theme.",
    long_description=LONG_DESCRIPTION,
    long_description_content_type="text/markdown",
    author="Daniel Townsend",
    author_email="dan@dantownsend.co.uk",
    python_requires=">=3.6.0",
    url="https://github.com/piccolo-orm/piccolo_theme",
    packages=find_packages(),
    package_data={
        "piccolo_theme": [
            "*.html",
            "**/*.html",
            "theme.conf",
            "static/*",
            "static/**/*"
        ],
    },
    install_requires=parse_requirement("requirements.txt"),
    license="MIT",
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: Implementation :: CPython",
        "Framework :: Sphinx :: Theme"
    ],
    entry_points={
        "sphinx.html_themes": [
            "piccolo_theme = piccolo_theme",
        ]
    },
)
