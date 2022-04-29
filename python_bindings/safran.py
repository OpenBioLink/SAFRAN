import itertools
import collections
from typing import List, Dict, Tuple
from safran_wrapper import pysafran, query_output_t, query_triples_t

class SAFRAN(pysafran):
	def __init__(self, train_path: str, rule_path: str, n_jobs: int = 1):
		pysafran.__init__(self, train_path, rule_path, n_jobs)

	def query(self, triples: List[List[str]], k: int = 100, action: str = 'applymax') -> Dict[Tuple[str, str], List[Tuple[str, float, int]]]:
		if action != 'applymax':
			raise ValueError('Actions supported in the SAFRAN Python wrapper are: applymax')
		flat_triples = list(itertools.chain.from_iterable(triples))
		pred_vals = query_output_t(pysafran.query(self, action, k, query_triples_t(flat_triples)))
		out = collections.defaultdict(list)
		for head, (pred, (tail, (val, rule_id))) in pred_vals:
			out[head, pred].append((tail, val, rule_id))
		return out
