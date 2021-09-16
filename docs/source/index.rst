.. SAFRAN documentation master file, created by
   sphinx-quickstart on Wed Sep 15 16:36:42 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to SAFRAN's documentation!
==================================

.. image:: https://github.com/OpenBioLink/SAFRAN/blob/master/resources/img/logo.png
   :class: with-shadow


SAFRAN (Scalable and fast non-redundant rule application) is a framework
for fast inference of groundings and aggregation of logical rules on
large heterogeneous knowledge graphs. It is based on the work of
`AnyBURL <http://web.informatik.uni-mannheim.de/AnyBURL/>`__ (Anytime
Bottom Up Rule Learning), which is an algorithm for learning, applying
and evaluating logical rules from large knowledge graphs in the context
of link prediction.

.. toctree::
   :caption: Getting Started
   :name: quickstart
   :maxdepth: 2

   installation
   
.. toctree::
   :caption: Manual
   :name: manual
   :maxdepth: 2

   manual/command_lines
   manual/actions
   manual/properties
   manual/trial
   manual/input_fmt

.. toctree::
   :caption: Benchmark results
   :name: benchmark
   :maxdepth: 2

   results



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
