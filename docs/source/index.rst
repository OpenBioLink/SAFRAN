.. SAFRAN documentation master file, created by
   sphinx-quickstart on Wed Sep 15 16:36:42 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. image:: /img/logo.png
   :class: with-shadow
   :align: center

------------

Welcome to SAFRAN's documentation!
==================================

SAFRAN (Scalable and fast non-redundant rule application) is a framework
for fast inference of groundings and aggregation of predictions of logical
rules in the context of knowledge graph completion/link prediction. It uses
rules learned by `AnyBURL <http://web.informatik.uni-mannheim.de/AnyBURL/>`__ (Anytime
Bottom Up Rule Learning), a highly-efficient approach for learning logical rules from knowledge graphs.

.. note::
   Currently only rules learned with the `AnyBURL-RE` version are supported. Further information can be found at the *Previous and Special Versions* section at the [AnyBURL homepage](https://web.informatik.uni-mannheim.de/AnyBURL).

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
   manual/applymax
   manual/applynoisy
   manual/applynrnoisy
   manual/learnnrnoisy
   manual/calcjacc
   manual/input_fmt
   manual/expl

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
