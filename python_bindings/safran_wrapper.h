#include "Index.h"
#include "TraintripleReader.h"
#include "ValidationtripleReader.h"
#include "RuleReader.h"
#include "RuleApplication.h"
#include "Properties.hpp"
#include "InMemoryExplanation.h"

#include <utility>
#include <string>
#include <vector>
#include <tuple>

using string_vector_t = std::vector<std::string>;

class pysafran {
public:
	pysafran(std::string train_path, std::string rule_path, int num_threads);
	~pysafran();

	// [(head, pred, tail)] -> [(head, pred, tail, rule_confidence, rule_id)]
	std::vector<std::pair<std::string, std::pair<std::string, std::pair<std::string, std::pair<float, int>>>>> query(const std::string & action, size_t k, const std::vector<std::string> & flat_triples) const;

private:
	Index *index;
	TraintripleReader* graph;
	ValidationtripleReader* vtr;
	RuleReader* rr;
	RuleApplication *ra;
	InMemoryExplanation *exp;
};