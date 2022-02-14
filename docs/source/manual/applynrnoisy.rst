Action *applynrnoisy*
---------------------

.. caution:: Needs clusters from action *learnnrnoisy*

Application of Non-redundant Noisy-OR (NRNO) to the test set (requires learned clusters). NRNO tries to overcome the problem of redundancy when using the Noisy-OR aggregation method by clustering rules based on their redundancy degree prior to aggregation. Predictions of rules in a cluster are aggregated using the Maximum approach, as this approach is not susceptible to redundancies. Predictions of the different clusters are then further aggregated using the Noisy-OR approach. As a metric for redundancy between two rules :math:`r_i`, :math:`r_j` the Jaccard Index :math:`sim(r_i,r_j) = |\hat{H}_{r_i} \cap \hat{H}_{r_j}| / |\hat{H}_{r_i} \cup \hat{H}_{r_j}|` of the sets of inferred triples is used. As the calculation of the Jaccard coefficient is very inefficient for large sets, the Jaccard coefficient is estimated using the MinHash scheme, which makes time complexity linear and memory usage constant.

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
   * PATH_CLUSTER : Valid path (file)
        Path to clustering file, **default: cluster.txt**

Properties :
   * WORKER_THREADS : int
        Number of threads that are used for computation. (-1 means all threads are used), **default: -1**
   * DISCRIMINATION_BOUND : int
        Discriminates (omits) rules which predict more elements than this, 0 means no limit., **default: 4000**
   * UNSEEN_NEGATIVE_EXAMPLES : int
        The number of negative examples for which we assume that they exist, however, we have not seen them. Rules with high coverage are favoured the higher the chosen number, **default: 5**
   * REFLEXIV_TOKEN : string
        Token used for substitution of reflexive rules. (Used if AnyBURL ruleset was trained with REWRITE_REFLEXIV = TRUE), **default: me_myself_i**
   * TOP_K_OUTPUT : int
        The top-k results that are after filtering kept in the results, **default: 10**
   * PREDICT_UNKNOWN : int
        If set to 1, does not skip triples containing unkwown entities in the training set. F.e. generates predictions for *john speaks UNKOWN* if *UNKNOWN* is not in the training set. **default 0**
   * ONLY_XY : int
        If set to 1, only cyclic (XY) rules are read from the rules file, **default: 0**
        
Output : 
    * PATH_OUTPUT : Valid path (file)
        Path to file used for storing predictions, **default: predictions.txt**