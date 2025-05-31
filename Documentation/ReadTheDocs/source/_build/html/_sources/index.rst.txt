Firmware and Software Documentation for OpenExo: An Open-Source Modular Exoskeleton to Augment Human Function
===================================================================================================================

.. image:: photos/1042_exoskeleton_20250205.jpg
   :alt: OpenSourceLeg Banner
   :align: center

.. raw:: html

   <div align="center">
     <a href="https://github.com/naubiomech/OpenExo">
       <img src="https://img.shields.io/badge/GitHub-OpenExo-black?logo=github"
            alt="OpenExo on GitHub">
     </a>
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

Getting Started
---------------
For new users, visit the **User Guide** page for an overview of the project.

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
Hardware License
~~~~~~~~~~~~~~~~
OpenExo’s **hardware** is released under the **CERN-OHL-P v2.0** license.  
You may study, modify, manufacture, and distribute the hardware provided you share modifications under the same license.  
Full text: https://ohwr.org/project/cernohl/-/wikis/uploads/98ff9662c7ce4252ec91104118c2af8e/cern_ohl_p_v2.pdf

Software License
~~~~~~~~~~~~~~~~
OpenExo’s **software** is released under the **GNU Lesser General Public License v3.0 (LGPL-3.0)**.  
You may use, study, share, and modify the software so long as derivative works also comply with the LGPL.  
Details: https://www.gnu.org/licenses/lgpl-3.0.html

Contact & Resources
-------------------
For questions, issues, or further assistance, reach out to us at:

- **Email:** theopenexo@gmail.com
- **GitHub:** https://github.com/naubiomech/OpenExo
- **Website:** http://theopenexo.org
- **Biomechanics Lab:** https://biomech.nau.edu

.. _toc:

.. toctree::
   :maxdepth: 2
   :caption: Tutorials:

   introduction
   User_Guide
   installation
   addingelements
   controllers
   gui
   AddingNewPages
   vscode_setup
   gamification_template
   StyleGuide