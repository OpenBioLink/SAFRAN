# IRIFAB

IRIFAB (Improved rule inference for AnyBurl) is a application for inferencing the results of symbolic rules and create candidate rankings of them. It is based on the work of [AnyBURL](http://web.informatik.uni-mannheim.de/AnyBURL/) (Anytime Bottom Up Rule Learning), which is an algorithm for learning, applying and evaluating logical rules from large knowledge graphs in the context of link prediction.

There are two versions of the rule applicator. 
+ The ***CPU*** version is designed for the excecution on one or multiple CPU threads. 
+ The ***CUDA*** version was designed for the excecution on CUDA-capable GPU's, but does currently ***NOT*** work

## Download binaries

[Windows x64](https://github.com/OpenBioLink/IRIFAB/raw/master/resources/binaries/IRIFAB.exe)

[Linux](https://github.com/OpenBioLink/IRIFAB/raw/master/resources/binaries/IRIFAB)

[Example config-apply.properties](https://github.com/OpenBioLink/IRIFAB/raw/master/resources/binaries/config-apply.properties)

## Build from source

1. Have cmake installed
2. Create folder **build** in {PATH_TO_PROJECT}/CPU/
3. Switch to {PATH_TO_PROJECT}/CPU/build
4. Run `cmake ../`

If building using Visual Studio
1. Open IRIFAB.sln
2. Select **Release** as build type
4. Click on **Build** → **Build Solution**

Else
1. Run `make` from **build** folder

## Manual

To run IRIFAB a properties file called apply-config.properties is required as an startup argument. To quickly perform a test run with the FB15k dataset, clone this repository, change to path `resources/binaries` and run:

#### Windows

`IRIFAB.exe apply-config.properties`

#### Linux

`./IRIFAB apply-config.properties`

#### Properties file

An overview of the format and possible parameters can be found [here](https://github.com/OpenBioLink/IRIFAB/wiki/Properties-file).

#### Trials

To be able to quickly estimate evaluation results, the prediction can be performed only on a sample of the test set. The sample size is dependent on the confidence level and the margin of error, both can be set in the properties file.

If f.e. the confidence level is set to 95% and the margin of error is set to 5%, then the evaluation results of  95% of trials lie between ±5% of the evaluation results of the whole test set. An example on the accuracy and runtimes can be seen [here](https://github.com/OpenBioLink/IRIFAB#trial), where the runtime was improved from 4.6 hours to 40 seconds with estimated similar results.

## Performance

### Used datasets

|                              | Train     | Test    | Valid   | Entites | Relations |
| ---------------------------- | --------- | ------- | ------- | ------- | --------- |
| FB15k                        | 483,142   | 59,071  | 50,000  | 14,951  | 1,345     |
| OpenBioLink (positive edges) | 4,193,905 | 180,509 | 183,653 | 180,926 | 28        |

### Test runs

Performance test performed on an Intel i7-6500U CPU @ 2.50GHz, 2 Cores, 4 Logical Processors. (Averaged over 10 runs)

Each test was run with the following properties:

```
UNSEEN_NEGATIVE_EXAMPLES = 5
TOP_K_OUTPUT = 10
WORKER_THREADS = 3
```

The column "Configuration" shows the different configurations of the following properties:

```
FAST DISCRIMINATION_UNIQUE INTERMEDIATE_DISCRIMINATION DISCRIMINATION_BOUND
```

While 0 1 0 is closest to the original implementation and 1 0 1 is the fastest.

-----

#### FB15k

Ruleset (alpha-50) size: 106,480

##### IRIFAB

| Configuration | Runtime file reading and preperation (ms) | Runtime rule application (ms) | hits@1 | hits@3 | hits@10 |
| ------------- | ----------------------------------------- | ----------------------------- | ------ | ------ | ------- |
| 0 0 0 1000    | 11,751                                    | 18,956                        | 0.7933 | 0.8292 | 0.8640  |
| 0 0 1 1000    | 11,852                                    | 18,536                        | 0.7934 | 0.8292 | 0.8639  |
| 0 1 0 1000    | 11,780                                    | 25,282                        | 0.7933 | 0.8292 | 0.8643  |
| 0 1 1 1000    | 11,778                                    | 24,700                        | 0.7933 | 0.8292 | 0.8641  |
| 1 0 0 1000    | 11,834                                    | 9,844                         | 0.7933 | 0.8292 | 0.8640  |
| 1 0 1 1000    | 11,796                                    | 9,714                         | 0.7934 | 0.8292 | 0.8639  |
| 1 1 0 1000    | 11,654                                    | 10,450                        | 0.7933 | 0.9292 | 0.8643  |
| 1 1 1 1000    | 11,714                                    | 10,370                        | 0.7933 | 0.8292 | 0.8641  |
| 0 1 0 4000    | 11,868                                    | 27,280                        | 0.8090 | 0.8445 | 0.8783  |
| 1 0 1 4000    | 11,763                                    | 10,444                        | 0.8090 | 0.8445 | 0.8782  |

##### Original AnyBURL Rule Application

| Configuration | Runtime file reading and preperation (ms) | Runtime rule application (ms) | hits@1 | hits@3 | hits@10 |
| ------------- | ----------------------------------------- | ----------------------------- | ------ | ------ | ------- |
| 0 1 0* 1000   | 7,170                                     | 148,877                       | 0.8094 | 0.8443 | 0.8785  |
| 0 1 0* 4000   | 7,053                                     | 203,534                       | 0.8094 | 0.8443 | 0.8785  |

------

#### OpenBioLink (Positive edges only)

Ruleset (alpha-50) size: 4,845

##### IRIFAB

| Configuation | Runtime file reading and preperation (ms) | Runtime rule application | hits@1 | hits@3 | hits@10 |
| ------------ | ----------------------------------------- | ------------------------ | ------ | ------ | ------- |
| 0 1 0 1000   | 43,585                                    | 276 mins (4.6 h)         | 0.1160 | 0.2107 | 0.3514  |
| 1 1 0 1000   | 42,897                                    | 19 mins                  | 0.1160 | 0.2107 | 0.3514  |
| 1 0 1 1000   | 42,912                                    | 56,736 ms                | 0.1191 | 0.2135 | 0.3560  |

##### Original AnyBURL Implementation

| Configuation | Runtime file reading and preperation (ms) | Runtime rule application | hits@1 | hits@3 | hits@10 |
| ------------ | ----------------------------------------- | ------------------------ | ------ | ------ | ------- |
| 0 1 0* 1000  | 79,191                                    | 789 mins (13.2 h)        | 0.1160 | 0.2107 | 0.3514  |

##### Trial

Means over 10 runs:

| Configuration | Runtime file reading and preperation (ms) | Runtime rule application (ms) | Confidence level | Margin of error | Sample size | hits@1  | hits@3  | hits@10 |
| ------------- | ----------------------------------------- | ----------------------------- | ---------------- | --------------- | ----------- | ------- | ------- | ------- |
| 0 1 0 1000    | 43,380                                    | 35,239                        | 95%              | 5%              | 384         | 0.11758 | 0.20925 | 0.35235 |

Boxplot of 10 runs:
![boxplot_obl](https://github.com/OpenBioLink/IRIFAB/blob/master/resources/img/boxplotobl.png)

----

*) You cannot actually set these  parameters (FAST DISCRIMINATION_UNIQUE INTERMEDIATE_DISCRIMINATION DISCRIMINATION_BOUND) in the AnyBURL implementation. Here 0 1 0 just represents a run with the original AnyBURL implementation and the specified DISCRIMINATION_BOUND set.