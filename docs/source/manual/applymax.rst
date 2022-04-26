Action *applymax*
=================

Application of rules using the Maximum aggregation approach:

.. math::
    score(e) = \max \{conf(r_1), \dots, \text{conf}(r_k)\}

where score(e) is the maximum confidence of all rules :math:`r_1, \dots, r_k` that predict entity :math:`e`. If the maximum confidences of two or more entities are the same, these entities are further ranked by their second best confidence and so on, until all top-k candidates can be distinguished or all rules are processed.

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