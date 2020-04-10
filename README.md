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

If f.e. the confidence level is set to 95% and the margin of error is set to 5%, then the evaluation results of  95% of trials lie between ±5% of the evaluation results of the whole test set. An example on the accuracy and runtimes can be seen [here](https://github.com/OpenBioLink/IRIFAB#trial), where the runtime was improved from 50 minutes to 40 seconds with estimated similar results.

## Performance

***For more details on the performance see [here](https://github.com/OpenBioLink/IRIFAB/wiki/Performance)***

### Used datasets

|                              | Train     | Test    | Valid   | Entites | Relations |
| ---------------------------- | --------- | ------- | ------- | ------- | --------- |
| FB15k                        | 483,142   | 59,071  | 50,000  | 14,951  | 1,345     |
| OpenBioLink (positive edges) | 4,193,905 | 180,509 | 183,653 | 180,926 | 28        |

### Test runs

Performance test performed on Windows on an Intel i7-6500U CPU @ 2.50GHz, 2 Cores, 4 Logical Processors. Almost all results were averaged over 10 runs (Except runs taking longer than 4 hours).

***Due to faster I/O, runs on Linux tend to be 20-25% faster.***

Each test was run with the following properties:

```
UNSEEN_NEGATIVE_EXAMPLES = 5
TOP_K_OUTPUT = 10
WORKER_THREADS = 3
```

### FB15K

|               | Preparation | Rule application | hits@1 | hits@3 | hits@10 |
| ------------- | ----------- | ---------------- | ------ | ------ | ------- |
| ***AnyBURL*** | 7.09 s      | 148.87 s         | 0.8094 | 0.8443 | 0.8785  |
| ***IRIFAB***  | 11.75 s     | 10.44 s          | 0.8090 | 0.8445 | 0.8782  |

### OpenBioLink (Positive edges only)

#### alpha-50 (4,845 rules)

|               | Preparation | Rule application | hits@1 | hits@3 | hits@10 |
| ------------- | ----------- | ---------------- | ------ | ------ | ------- |
| ***AnyBURL*** | 79.19 s     | 789 min (13.2 h) | 0.1160 | 0.2107 | 0.3514  |
| ***IRIFAB***  | 42.9 s      | 56.73 s          | 0.1191 | 0.2135 | 0.3560  |

#### alpha-1000 reinforced (393,841 rules)

|                    | Preparation | Rule application | hits@1 | hits@3 | hits@10 |
| ------------------ | ----------- | ---------------- | ------ | ------ | ------- |
| ***AnyBURL***      |             |                  |        |        |         |
| ***IRIFAB***       | 47.27 s     | 52.1 min         | 0.1646 | 0.2798 | 0.4375  |
| ***IRIFAB Trial*** | 46.96 s     | 45.43 s          | 0.1672 | 0.2866 | 0.4372  |