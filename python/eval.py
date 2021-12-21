import os
import math
from scipy.stats import rankdata
from tqdm import tqdm
import sys

def read_predictions(path):
    with open(path, encoding="utf8") as infile:
        while True:
            triple = infile.readline().strip().split(" ")
            if not triple or triple[0] == "":
                break
            head,rel,tail = triple
            pred_heads = infile.readline().strip()[7:].split("\t")
            pred_tails = infile.readline().strip()[7:].split("\t")
            
            confidences_head = [int(x.replace("0.", "").replace("1.","1").ljust(100, "0")) if (not x.startswith("1.") and not x.startswith("1")) else int("1".ljust(101, "0")) for x in pred_heads[1::2]]
            confidences_tail = [int(x.replace("0.", "").replace("1.","1").ljust(100, "0")) if (not x.startswith("1.") and not x.startswith("1")) else int("1".ljust(101, "0")) for x in pred_tails[1::2]]
            
            yield (head, pred_heads[0::2], confidences_head)
            yield (tail, pred_tails[0::2], confidences_tail)
        

def get_n_test(path):
    content = None
    with open(path, encoding="utf8") as infile:
        content = infile.readlines()
    content = [x.strip() for x in content]
    return len(content)       
    

def evaluate_policy(path_predictions, n, policy):
    hits1 = 0
    hits3 = 0
    hits10 = 0
    mrr = 0.0
    mr = 0
    
    for true_entity, prediction, conf in read_predictions(path_predictions):
        ranking = rankdata([-x for x in conf], method=policy)
        try:
            idx = prediction.index(true_entity)
            rank = ranking[idx]
            
            if rank == 1.:
                hits1 = hits1 + 1
            if rank <= 3.:
                hits3 = hits3 + 1
            if rank <= 10.:
                hits10 = hits10 + 1
            mrr = mrr + (1 / rank)
        except ValueError:
            pass
    #return hits1/n, hits3/n, hits10/n, mrr/n
    return "MRR: %.3f" % (mrr/n), "Hits@1: %.3f" % (hits1/n), "Hits@3: %.3f" % (hits3/n) , "Hits@10: %.3f" % (hits10/n) 

def evaluate(path_predictions, path_test):
    n = get_n_test(path_test) * 2
    #["ordinal", "average", "min", "max", "dense"]
    result = evaluate_policy(path_predictions, n, "average")
    return "\n".join(result)


if __name__ == "__main__":
    res = evaluate(sys.argv[1], sys.argv[2])
    print(res)
