.. Sferes_v2 documentation master file, created by
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Sferes_v2 -- documentation
============================

Sferes_v2 is a generic framework for evolutionary computation research. It provides the basic infrastructure to implement new evolution algorithms and new repesentations. The framework is voluntarily kept simple (less than 5000 lines of code).

Main features:

- modern, effective C++ (C++11)
- multi-core support (via Intel TBB)
- MPI support (via boost::mpi)
- main algorithms: NSGA-II, MAP-Elites (Quality Diversity)
- native support of behavioral diversity and novelty search

Additional modules:

- nn2: neuro-evolution
- fastsim: 2D simulator for mobile robot (e.g. Khepera or roomba)


License
-------

Sferes_v2 is currently licensed with a CeCILL License (equivalent to GPL-v2). If you use
Sferes_v2 in a scientific paper, please cite:


Mouret, J.-B. and Doncieux, S. (2010). SFERESv2: Evolvin' in the
Multi-Core World. *WCCI 2010 IEEE World Congress on Computational
Intelligence, Congress on Evolutionary Computation (CEC)*. Pages
4079–4086. `{pdf} <http://www.isir.upmc.fr/files/2010ACTI1524.pdf>`__


Contents:
----------

.. toctree::
  :hidden:
  :caption: Sferes_v2

.. toctree::
  :maxdepth: 2

  self
  concepts  
  reference


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

