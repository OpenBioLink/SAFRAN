<p align="center">
  <img src="https://github.com/OpenBioLink/IRIFAB/raw/dev/resources/img/logo.png">
</p>

SAFRAN (Scalable and fast non-redundant rule application) is a framework for fast inference of groundings and aggregation of logical rules on large heterogeneous knowledge graphs. It is based on the work of [AnyBURL](http://web.informatik.uni-mannheim.de/AnyBURL/) (Anytime Bottom Up Rule Learning), which is an algorithm for learning, applying and evaluating logical rules from large knowledge graphs in the context of link prediction.



## Download binaries

Coming soon...

## Build from source

1. Have cmake installed (> 9.6.0)
2. Create and change to directory **build**
3. Run `cmake ../`
4. Run `make`

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

`IRIFAB.exe {action} {path_to_properties}`

#### Linux

`./IRIFAB {action} {path_to_properties}`

## Performance

***For more details on the performance see [here](https://github.com/OpenBioLink/IRIFAB/wiki/Performance)***

## Results

### FB15K-237

|                                     |                                                        | hits@1     | hits@3     | hits@10    |
| ----------------------------------- | ------------------------------------------------------ | ---------- | ---------- | ---------- |
| ***AnyBURL*** Maximum approach      |                                                        | 0.2727     | 0.3884     | 0.5228     |
| ***AnyBURL*** Noisy-Or              |                                                        | 0.2228     | 0.3298     | 0.4621     |
| ***SAFRAN*** Non-redundant Noisy-Or | parameter sweep, single threshold, k=200               | 0.2888     | 0.4046     | 0.5346     |
|                                     | random search, multi threshold, k=10, iterations=10000 | **0.3013** | **0.3175** | **0.5465** |

### OpenBioLink (Positive edges only, [repository](https://github.com/OpenBioLink/OpenBioLink))

|                                     |                                                        | hits@1 | hits@3 | hits@10 |
| ----------------------------------- | ------------------------------------------------------ | ------ | ------ | ------- |
| ***AnyBURL*** Maximum approach      |                                                        | 0.1948 | 0.3066 | 0.4630  |
| ***AnyBURL*** Noisy-Or              |                                                        | 0.0754 | 0.1513 | 0.4217  |
| ***SAFRAN*** Non-redundant Noisy-Or | parameter sweep, single threshold, k=200               | 0.2205 | 0.3424 | 0.5056  |
|                                     | random search, multi threshold, k=10, iterations=10000 | **0.2232** | **0.3473** | **0.5110** |
