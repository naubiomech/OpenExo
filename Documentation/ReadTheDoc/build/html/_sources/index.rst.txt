An Open-Source Platform for Numerical Modeling, Data Acquisition, and Control of Lower-Limb Assistive Exoskeletons
=============================================================================================================

.. image:: 1042_exoskeleton_20250205.jpg
   :alt: OpenSourceLeg Banner
   :align: center

.. raw:: html

   <div align="center">
      <a href="https://travis-ci.org/your_repo"><img src="https://img.shields.io/travis/your_repo.svg" alt="Build Status"></a>
      <a href="https://readthedocs.org/"><img src="https://readthedocs.org/projects/your_project/badge/?version=latest" alt="Docs Status"></a>
      <a href="https://www.python.org/downloads/"><img src="https://img.shields.io/badge/python-3.9%2B-blue" alt="Python Versions"></a>
      <a href="https://opensource.org/licenses/GPL-3.0"><img src="https://img.shields.io/badge/license-GPL--3.0-green" alt="License"></a>
   </div>

Prerequisites
-------------
Before installing the **opensourceleg** library, ensure you have the following installed:

- **Python 3.9** or later
- **pip** package manager

Installation
------------
The easiest and quickest way to install the **opensourceleg** library is via pip:

.. code-block:: bash

   pip install opensourceleg

If you plan on installing the **opensourceleg** library on a Raspberry Pi, it is recommended to use the **opensourcelegpi** tool. This tool is a cloud-based CI service that builds an up-to-date OS for a Raspberry Pi. It bundles the **opensourceleg** library and its dependencies into a single OS image, which can be flashed onto a microSD card for headless or GUI-less operation to control autonomous/remote robotic systems. For more information, click the provided link in the original README.

Getting Started
---------------
For new users, visit the **Getting Started** page for an overview of the library and its documentation.

Developing
----------
If you wish to modify, develop, or contribute to the **opensourceleg** library, it is recommended to install **Poetry**, a Python packaging and dependency management tool. Once Poetry is installed, clone the repository and set up the environment using the following commands:

.. code-block:: bash

   git clone https://github.com/neurobionics/opensourceleg.git
   cd opensourceleg
   poetry install
   poetry shell

License
-------
The **opensourceleg** library is licensed under the GPL-3.0 license. This license grants you the following freedoms:

- **Use**: You are free to use the library for any purpose.
- **Modify**: You may modify the library to suit your needs.
- **Study**: You can study and change how the library works.
- **Distribute**: You are allowed to distribute modified versions of the library.

The GPL license ensures that these freedoms are upheld, requiring contributors to share their modifications publicly if they distribute the library.

.. toctree::
   :maxdepth: 2
   :caption: Tutorials:

   introduction
   installation
   addingelements

