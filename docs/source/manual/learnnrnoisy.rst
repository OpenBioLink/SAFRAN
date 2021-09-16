Action *learnnrnoisy*
---------------------

.. caution:: Needs similarity matrices from action *calcjacc*

Learning of the optimal thresholds for the clustering used by non redundant noisy or (requires similarity matrices)

Configuration file
^^^^^^^^^^^^^^^^^^

Input : 
   * PATH_TRAINING : Valid path (file)
        Path to training file (absolute or relative), **default: train.txt**
   * PATH_TEST : Valid path (file)
        Path to test file (absolute or relative), **default: test.txt**
   * PATH_VALID : Valid path (file)
        Path to validation file (absolute or relative), **default: valid.txt**
   * PATH_RULES : Valid path (file)
        Path to rules file (absolute or relative), **default: rules.txt**
   * PATH_JACCARD : Valid path (directory)
        Path to directory containing jaccard files, **default: jaccard/**
 
Properties :
   * WORKER_THREADS : int
        Number of threads that are used for computation. (-1 means all threads are used), **default: -1**
   * DISCRIMINATION_BOUND : int
        Discriminates (omits) rules which predict more elements than this, 0 means no limit., **default: 4000**
   * UNSEEN_NEGATIVE_EXAMPLES : int
        The number of negative examples for which we assume that they exist, however, we have not seen them. Rules with high coverage are favoured the higher the chosen number, **default: 5**
   * REFLEXIV_TOKEN : string
        Token used for substitution of reflexive rules. (Used if AnyBURL ruleset was trained with REWRITE_REFLEXIV = TRUE), **default: me_myself_i**
   * BUFFER_SIZE : int
        Buffer size (in amount of integers, 4 byte) used to limit memory consumption of buffering previously inferred rules. Should only be set if running out of memory. (2500000000 --> ~10 GB), **default: Maximum unsigned long long**
   * TOP_K_OUTPUT : int
        Top-K predictions that are used to calculate the MRR for hyperparameter search, **default: 10**
   * RESOLUTION : int
        Sets the accuracy of the Jaccard estimation. The number of hash functions used in MinHash (f.e. RESOLUTION = 200 --> 200 hash functions --> Max resolution of Jaccard 1/200), **default: 200**
   * STRATEGY : [grid|random]
        Sets the search strategy to be used for finding optimal clustering, **default: grid**
   * ITERATIONS : int
        Amount of iterations used in random search strategy, **default: 10000**
   * SEED : int
        Seed for the sampling of thresholds used in random search strategy, **default: 0**

Output : 
    * PATH_CLUSTER : Valid path (file)
        Path to clustering file, **default: cluster.txt**