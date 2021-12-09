Creating explanation file
=========================

Safran currently supports the creation of explanations for predictions, as needed f.e. for `https://github.com/OpenBioLink/Explorer/ LinkExplorer`_.


Building SAFRAN with explanation capabilities
---------------------------------------------

1. Checkout the branch `explanation` of the SAFRAN repository
2. Download and extract boost 1.76.0 to project root directory

   1. Windows: `https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.zip <https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.zip>`_
   2. Unix: `https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz <https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz>`_

3. Have cmake installed (> 9.6.0)
4. Create and change to directory **build**
5. Run ``cmake ../``
6. Run ``make`` (Unix) or ``cmake --build .`` (Windows)

Create explanations
-------------------

You can now run the built SAFRAN executable with the actions `applynoisy`, `applymax`, `applynrnoisy` and `EXPLAIN = 1` in the properties file. This creates a sqlite database file that stores the dataset, predictions and their explanations.

Explanation DB schema
---------------------

.. image:: /img/graph.png
   :class: with-shadow
   :align: center

