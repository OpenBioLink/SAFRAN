<p align="center">
  <img src="https://github.com/OpenBioLink/IRIFAB/raw/dev/resources/img/logo.png">
</p>

SAFRAN (Scalable and fast non-redundant rule application) is a framework for fast inference of groundings and aggregation of logical rules on large heterogeneous knowledge graphs. It is based on the work of [AnyBURL](http://web.informatik.uni-mannheim.de/AnyBURL/) (Anytime Bottom Up Rule Learning), which is an algorithm for learning, applying and evaluating logical rules from large knowledge graphs in the context of link prediction.

[Paper preprint on arXiv](http://arxiv.org/abs/2012.05750)


## Download binaries

[Windows x64](https://github.com/OpenBioLink/IRIFAB/raw/master/resources/binaries/SAFRAN.exe)

[Linux](https://github.com/OpenBioLink/IRIFAB/raw/master/resources/binaries/SAFRAN)

## Build from source

1. Download and extract boost 1.76.0 to project root directory
   1. Windows: https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.zip
   2. Unix: https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz
2. Have cmake installed (> 9.6.0)
3. Create and change to directory **build**
4. Run `cmake ../`
5. Run `make`

If building using Visual Studio
1. Open SAFRAN.sln
2. Select **Release** as build type
4. Click on **Build** → **Build Solution**

## Manual

To run IRIFAB a properties file called apply-config.properties is required as an startup argument. Each action requires different properties (See table below).

### Actions

| Action       | Description                                                  | Properties |
| ------------ | ------------------------------------------------------------ | ---------- |
| calcjacc     | Calculation of the similarity matrices (Jaccard index) for each relation. |  [here](https://github.com/OpenBioLink/IRIFAB/wiki/Properties-file#action-calcjacc)      |
| learnnrnoisy | Learning of the optimal thresholds for the clustering used by non redundant noisy or (requires similarity matrices) |   [here](https://github.com/OpenBioLink/IRIFAB/wiki/Properties-file#action-learnnrnoisy)         |
| applynrnoisy | Application of non redundant noisy or to the test set (requires learned clusters) |  [here](https://github.com/OpenBioLink/IRIFAB/wiki/Properties-file#action-applynrnoisy)       |
| applymax     | Application using the maximum approach only                  |      [here](https://github.com/OpenBioLink/IRIFAB/wiki/Properties-file#action-applynoisyonly--applymaxonly)         |
| applynoisy   | Application using noisy-or only                              |     [here](https://github.com/OpenBioLink/IRIFAB/wiki/Properties-file#action-applynoisyonly--applymaxonly)           |

#### Windows

`SAFRAN.exe {action} {path_to_properties}`

#### Linux

`./SAFRAN {action} {path_to_properties}`

### Quickstart

Performs non-redundant Noisy-Or using grid search.

1. Clone project
2. To performs non-redundant Noisy-Or using grid search and a small ruleset of the FB15k-237 dataset, run from the directory ``resources/binaries``:
   1. Run ``./SAFRAN calcjacc quickstart/config-quickstart.properties``
   2. Run ``./SAFRAN learnnrnoisy quickstart/config-quickstart.properties``
   3. Run ``./SAFRAN applynrnoisy quickstart/config-quickstart.properties``

## Results

### FB15K-237
- [Dataset](http://web.informatik.uni-mannheim.de/AnyBURL/datasets.zip) retrieved from [http://web.informatik.uni-mannheim.de/AnyBURL/](http://web.informatik.uni-mannheim.de/AnyBURL/)
- [Ruleset](https://zenodo.org/record/4317155/files/fb15k237-1000.zip?download=1) used to achieve these results

|                                     |                                                        | hits@1     | hits@3     | hits@10    |
| ----------------------------------- | ------------------------------------------------------ | ---------- | ---------- | ---------- |
| ***AnyBURL*** Maximum approach      |                                                        | 0.2727     | 0.3884     | 0.5228     |
| ***AnyBURL*** Noisy-Or              |                                                        | 0.2228     | 0.3298     | 0.4621     |
| ***SAFRAN*** Non-redundant Noisy-Or | parameter sweep, single threshold, k=200               | 0.2888     | 0.4046     | 0.5346     |
|                                     | random search, multi threshold, k=10, iterations=10000 | **0.3013** | **0.4175** | **0.5465** |

### OpenBioLink (Positive edges only)

- [Repository](https://github.com/OpenBioLink/OpenBioLink)
- [Dataset](https://zenodo.org/record/4317155/files/obl-dataset.zip?download=1) (ready for use with AnyBURL)
- [Ruleset](https://zenodo.org/record/4317155/files/obl-1000.zip?download=1) used to achieve these results

|                                     |                                                        | hits@1 | hits@3 | hits@10 |
| ----------------------------------- | ------------------------------------------------------ | ------ | ------ | ------- |
| ***AnyBURL*** Maximum approach      |                                                        | 0.1948 | 0.3066 | 0.4630  |
| ***AnyBURL*** Noisy-Or              |                                                        | 0.0754 | 0.1513 | 0.4217  |
| ***SAFRAN*** Non-redundant Noisy-Or | parameter sweep, single threshold, k=200               | 0.2205 | 0.3424 | 0.5056  |
|                                     | random search, multi threshold, k=10, iterations=10000 | **0.2232** | **0.3473** | **0.5110** |


## Performance

|                                                | AnyBURL    | SAFRAN    | Speed up |
| ---------------------------------------------- | ---------- | --------- | -------- |
| **FB15k-237**                                  |            |           |          |
| Rule inference (inference of complete ruleset) | 615 259 ms | 46 205 ms | 13.3 x   |
| Maximum aggregation                            | 24 523 ms  | 4222 ms   | 5.8 x    |
| **OpenBioLink**                                |            |           |          |
| Rule inference (inference of complete ruleset) | 2252 h*    | 2.85 h    | 790.2 x  |
| Maximum aggregation                            | 35.9 h     | 43 min    | 50.1 x   |

Comparison of AnyBURL and SAFRAN runtimes (22 threads, Intel(R) Xeon(R) CPU E5-2650 v4 @ 2.20GHz). Note that maximum aggregation takes less time than the inference of the complete ruleset, as maximum aggregation, unlike Noisy-Or, does not require the inference of all rules. \*Estimation: After a runtime of 12 hours all rules were inferenced for 975 prediction tasks.
