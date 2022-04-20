
## eval.py

Script used to evaluate a single prediction file. 

##### Requires:

```
pip install scipy tqdm
```

##### Usage:

```bash
python eval.py {path to file containing predictions} {path to testset file}
```

##### Example output:

```
MRR: 0.389
Hits@1: 0.298
Hits@3: 0.371
Hits@10: 0.537
```

## eval_experiment.py

Script used to evaluate a experiment (Multiple datasets -> Multiple prediction files)

##### Requires:

```
pip install scipy tqdm
```

##### Usage:

```bash
python eval_experiment.py --datasets {list of datasets} --predictions {list of prediction file names}
```

**File structure:**

Each dataset should have its own folder. Evaluations are run 

```text
for each {dataset} in {list of datasets}: 
   for each {prediction file name} in {list of prediction file name}:
      Path to prediction file: f”./{dataset}/predictions/{prediction file name}”
      Path to testset file: f”./{dataset}/data/test.txt”
```

Example:

```bash
python eval_experiment.py --datasets OBL WN18RR --predictions predfile1.txt predfile2.txt
```

```text
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
```

## amie_2_anyburl.py

Converts rules learned by AMIE+ to the format of AnyBURL rules

**Usage:**

```
python amie_2_anyburl.py --from {path to amie rulefile} --to {path to file storing transformed rules, will be created} 
```

Optionally the flag `--pca` can be set to use pca confidence instead of std confidence.

