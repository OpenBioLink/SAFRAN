Format of the rule file
=======================

.. caution:: Whitespaces in entity and relation names are not supported!

Safran currently only supports rules in the format of AnyBURL:

.. code-block:: 

    {predicted: int}\t{correctlyPredicted: int}\t{confidence: double}\t{rule}

where ``predicted`` is the absolute number of all predictions made by the rule and ``correctlyPredicted`` is the absolute number of all predictions that are correct. ``confidence`` is redundant and never is used within SAFRAN as it gets calculated through :math:`\frac{correctlyPredicted}{predicted + \text{Properties.UNSEEN_NEGATIVE_EXAMPLES}`.

AnyBURL rule format
-------------------

``rule`` is a rule in the format of AnyBURL:

.. code-block::

    {headatom} <= {(bodyatom)+}

where the seperator of ``headatom`` and `(bodyatom)+` is " <= " (notice the whitespaces).

``headatom`` is the head atom of a rule:

* Cyclic rules: can be only ``{r}(X,Y)``, with ``r`` being a valid relation
* Acyclic rules: can be either ``{r}(X,{c})`` or ``{r}({c},Y)``, with ``r`` being a valid relation and ``c`` being a valid constant entity.

``(bodyatom)+`` is a list of bodyatoms:

* Bodyatoms are ordered from 'X' to 'Y'
* Adjacent bodyatoms have to be connected by a single variable
* If ``head`` contains 'X': First body atom has to contain 'X'
* If ``head`` contains 'Y': Last body atom has to contain 'Y'
* Bodyatoms are seperated by ", " (notice the whitespace)

Example of valid rules
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block::

    1302	165	0.126728111	hasNeighbor(X,Y) <= dealsWith(Y,X)
    1302	164	0.125960061	hasNeighbor(X,Y) <= dealsWith(X,Y)
    2000	216	0.108	GENE_GO(X,Y) <= GENE_CATALYSIS_GENE(X,A), GENE_CATALYSIS_GENE(A,B), GENE_GO(B,Y)
    2000    73      0.0365  GENE_REACTION_GENE(X,Y) <= GENE_CATALYSIS_GENE(A,X), GENE_PTMOD_GENE(B,A), GENE_BINDING_GENE(Y,B)
    42      3       0.07142857142857142     P106(X,Q520549) <= P101(X,Q413)
    29	2	0.06896551724137931	P27(Q75612,Y) <= P30(Y,Q49)
    194	3	0.015463917525773196	P27(Q75612,Y) <= P30(Y,A)
    50	2	0.04	P27(Q154756,Y) <= P551(A,Y)
    959	7	0.0072992700729927005	P27(X,Q27) <= P27(X,A)
    238	6	0.025210084033613446	P136(X,Q676) <= P737(A,X)