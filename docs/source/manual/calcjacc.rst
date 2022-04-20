Action *calcjacc*
-----------------

Calculates the similarity matrices (Jaccard index) used by Non-redundant Noisy-OR for each relation. 


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

Properties :
   * WORKER_THREADS : int
        Number of threads that are used for computation. (-1 means all threads are used), **default: -1**
   * DISCRIMINATION_BOUND : int
        Discriminates (omits) rules which predict more elements than this, 0 means no limit., **default: 4000**
   * REFLEXIV_TOKEN : string
        Token used for substitution of reflexive rules. (Used if AnyBURL ruleset was trained with REWRITE_REFLEXIV = TRUE), **default: me_myself_i**   
   * SEED : int
        Seed for generating hash functions used in MinHash, **default: 0**
   * RESOLUTION : int
        Sets the accuracy of the Jaccard estimation. The number of hash functions used in MinHash (f.e. RESOLUTION = 200 --> 200 hash functions --> Max resolution of Jaccard 1/200), **default: 200**
   * ONLY_XY : int
        If set to 1, only cyclic (XY) rules are read from the rules file, **default: 0**
        
Output :
    * PATH_JACCARD : Valid path (directory)
        Path to directory for storing binary files containing similarity matrices for each relation. *0_jacc.bin* f.e. is the similarity matrix of relation with ID 0., **default: jaccard/**
