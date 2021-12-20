Evaluate predictions
====================

Safran currently supports the creation of explanations for predictions, as needed f.e. for `https://github.com/OpenBioLink/Explorer/ LinkExplorer`_.

Evaluate a single prediction file
---------------------------------

Script name: eval.py

**Requires:**

.. code:: bash

   pip install scipy tqdm

**Usage:**

.. code:: bash

   python eval.py {path to file containing predictions} {path to testset file}

**Example output:**

::

   MRR: 0.389
   Hits@1: 0.298
   Hits@3: 0.371
   Hits@10: 0.537

Evaluate an experiment (Multiple datasets -> Multiple prediction files)
-----------------------------------------------------------------------

Script name: eval_experiment.py

**Requires:**

.. code:: bash

   pip install scipy tqdm

.. _usage-1:

**Usage:**

.. code:: bash

   python eval_experiment.py --datasets {list of datasets} --predictions {list of prediction file names}

**File structure:**

Path to prediction file: f”./{dataset}/predictions/{prediction}” Path to
testset file: f”./{dataset}/data/test.txt”

Example:

.. code:: bash

   python eval_experiment.py --datasets OBL WN18RR --predictions predfile1.txt predfile2.txt

.. code:: text

   ---- OBL
       |
       ---- predictions
           |
           ---- predfile1.txt
           |
           ---- predfile2.txt
       |
       ---- data
           |
           ---- test.txt
   ---- WN18RR
       |
       ---- predictions
           |
           ---- predfile1.txt
           |
           ---- predfile2.txt
       |
       ---- data
           |
           ---- test.txt

