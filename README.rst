.. raw:: html

   <p align="center">
     <img src="https://github.com/OpenBioLink/SAFRAN/blob/master/resources/img/logo.png">
   </p>

.. raw:: html

   <p align="center">
     <a href="https://github.com/OpenBioLink/SAFRAN/actions/workflows/build_cmake.yml">
       <img src="https://github.com/OpenBioLink/SAFRAN/actions/workflows/build_cmake.yml/badge.svg?branch=master"
            alt="CMake Build Matrix">
     </a>
   </p>

SAFRAN (Scalable and fast non-redundant rule application) is a framework
for fast inference of groundings and aggregation of logical rules on
large heterogeneous knowledge graphs. It is based on the work of
`AnyBURL <http://web.informatik.uni-mannheim.de/AnyBURL/>`__ (Anytime
Bottom Up Rule Learning), which is an algorithm for learning, applying
and evaluating logical rules from large knowledge graphs in the context
of link prediction.

`Paper preprint on arXiv <http://arxiv.org/abs/2012.05750>`__

Download binaries
-----------------

`Windows
x64 <https://github.com/OpenBioLink/IRIFAB/raw/master/resources/binaries/SAFRAN.exe>`__

`Linux <https://github.com/OpenBioLink/IRIFAB/raw/master/resources/binaries/SAFRAN>`__

Build from source
-----------------

1. Download and extract boost 1.76.0 to project root directory

   1. Windows:
      https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.zip
   2. Unix:
      https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz

2. Have cmake installed (> 9.6.0)
3. Create and change to directory **build**
4. Run ``cmake ../``
5. Run ``make``

If building using Visual Studio

1. Open SAFRAN.sln
2. Select **Release** as build type
3. Click on **Build** → **Build Solution**

Manual
------

To run IRIFAB a properties file called apply-config.properties is
required as an startup argument. Each action requires different
properties (See table below).

Actions
~~~~~~~

+--------------+--------------------------+--------------------------+
| Action       | Description              | Properties               |
+==============+==========================+==========================+
| calcjacc     | Calculation of the       | `here <https:            |
|              | similarity matrices      | //github.com/OpenBioLink |
|              | (Jaccard index) for each | /IRIFAB/wiki/Properties- |
|              | relation.                | file#action-calcjacc>`__ |
+--------------+--------------------------+--------------------------+
| learnnrnoisy | Learning of the optimal  | `here <https://gi        |
|              | thresholds for the       | thub.com/OpenBioLink/IRI |
|              | clustering used by non   | FAB/wiki/Properties-file |
|              | redundant noisy or       | #action-learnnrnoisy>`__ |
|              | (requires similarity     |                          |
|              | matrices)                |                          |
+--------------+--------------------------+--------------------------+
| applynrnoisy | Application of non       | `here <https://gi        |
|              | redundant noisy or to    | thub.com/OpenBioLink/IRI |
|              | the test set (requires   | FAB/wiki/Properties-file |
|              | learned clusters)        | #action-applynrnoisy>`__ |
+--------------+--------------------------+--------------------------+
| applymax     | Application using the    | `here <ht                |
|              | maximum approach only    | tps://github.com/OpenBio |
|              |                          | Link/IRIFAB/wiki/Propert |
|              |                          | ies-file#action-applynoi |
|              |                          | syonly--applymaxonly>`__ |
+--------------+--------------------------+--------------------------+
| applynoisy   | Application using        | `here <ht                |
|              | noisy-or only            | tps://github.com/OpenBio |
|              |                          | Link/IRIFAB/wiki/Propert |
|              |                          | ies-file#action-applynoi |
|              |                          | syonly--applymaxonly>`__ |
+--------------+--------------------------+--------------------------+

Windows
^^^^^^^

``SAFRAN.exe {action} {path_to_properties}``

Linux
^^^^^

``./SAFRAN {action} {path_to_properties}``

Quickstart
~~~~~~~~~~

Performs non-redundant Noisy-Or using grid search.

1. Clone project
2. To performs non-redundant Noisy-Or using grid search and a small
   ruleset of the FB15k-237 dataset, run from the directory
   ``resources/binaries``:

   1. Run ``./SAFRAN calcjacc quickstart/config-quickstart.properties``
   2. Run
      ``./SAFRAN learnnrnoisy quickstart/config-quickstart.properties``
   3. Run
      ``./SAFRAN applynrnoisy quickstart/config-quickstart.properties``



Performance
-----------

+-------------------------------+------------+-----------+----------+
|                               | AnyBURL    | SAFRAN    | Speed up |
+===============================+============+===========+==========+
| **FB15k-237**                 |            |           |          |
+-------------------------------+------------+-----------+----------+
| Rule inference (inference of  | 615 259 ms | 46 205 ms | 13.3 x   |
| complete ruleset)             |            |           |          |
+-------------------------------+------------+-----------+----------+
| Maximum aggregation           | 24 523 ms  | 4222 ms   | 5.8 x    |
+-------------------------------+------------+-----------+----------+
| **OpenBioLink**               |            |           |          |
+-------------------------------+------------+-----------+----------+
| Rule inference (inference of  | 2252 h\*   | 2.85 h    | 790.2 x  |
| complete ruleset)             |            |           |          |
+-------------------------------+------------+-----------+----------+
| Maximum aggregation           | 35.9 h     | 43 min    | 50.1 x   |
+-------------------------------+------------+-----------+----------+

Comparison of AnyBURL and SAFRAN runtimes (22 threads, Intel(R) Xeon(R)
CPU E5-2650 v4 @ 2.20GHz). Note that maximum aggregation takes less time
than the inference of the complete ruleset, as maximum aggregation,
unlike Noisy-Or, does not require the inference of all rules.
\*Estimation: After a runtime of 12 hours all rules were inferenced for
975 prediction tasks.
