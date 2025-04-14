An Open-Source Platform for Numerical Modeling, Data Acquisition, and Control of Lower-Limb Assistive Exoskeletons
===================================================================================================================

.. image:: 1042_exoskeleton_20250205.jpg
   :alt: OpenSourceLeg Banner
   :align: center

.. raw:: html

   <div align="center">
      <a href="https://github.com/naubiomech/OpenExo"><img src="https://github.com/naubiomech/OpenExo" alt="Build Status"></a>
      <a href="https://www.python.org/downloads/"><img src="https://img.shields.io/badge/python-3.9%2B-blue" alt="Python Versions"></a>
      <a href="https://opensource.org/licenses/GPL-3.0"><img src="https://img.shields.io/badge/license-GPL--3.0-green" alt="License"></a>
   </div>

Prerequisites
-------------
Before installing the **OpenExo** library, ensure you have the following installed:

- **Python 3.9** or later
- **pip** package manager

Installation
------------
The easiest and quickest way to install the **OpenExo** library is to clone the repository directly from GitHub:

.. code-block:: bash

   git clone https://github.com/naubiomech/OpenExo.git
   cd OpenExo
   pip install .

Alternatively, if you are interested in contributing or modifying the code, it is recommended to use **Poetry** for dependency management:

.. code-block:: bash

   git clone https://github.com/naubiomech/OpenExo.git
   cd OpenExo
   poetry install
   poetry shell

Getting Started
---------------
For new users, visit the **Getting Started** page for an overview of the library and its documentation.

Developing
----------
If you wish to modify, develop, or contribute to the **OpenExo** library, it is recommended to install **Poetry**, a Python packaging and dependency management tool. Once Poetry is installed, clone the repository and set up your development environment using the following commands:

.. code-block:: bash

   git clone https://github.com/naubiomech/OpenExo.git
   cd OpenExo
   poetry install
   poetry shell

License
-------
The **OpenExo** library is licensed under the GPL-3.0 license. This license grants you the following freedoms:

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
   controllers
