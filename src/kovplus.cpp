#include "kovplus.hpp"

#include <cmath>


// KovPlusChain
double KovPlusChain::assess(const std::vector<int> &tokens, const Assessor &assessor) const {
	double assessment = 0.0;

	int i = 0;
	int exp_i = 0;

	auto expected = assessor.context;

	if (tokens.size() < expected.size()) {
		exp_i = expected.size() - tokens.size();
	}

	else {
		i = tokens.size() - expected.size();
	}

	while (exp_i < assessment_window_width) {
		if (tokens[i] == expected[exp_i]) {
			double weight = 1.0 / (1.0 + std::sqrt(1 + std::sqrt((word_count.find(tokens[i]) != word_count.end() ? word_count.at(tokens[i]) : 0)) * std::abs(assessment_window_width - exp_i)));
			assessment += weight;
		}

		i++;
		exp_i++;
	}

	return assessment * assessor.strength;
}

double KovPlusChain::assess(const SentenceView &tokens, const Assessor &assessor) const {
	std::vector<int> token_ids;

	for (int i = 0; i < tokens.size(); i++) {
		token_ids.push_back(tokens.token_id(i));
	}

	return assess(token_ids, assessor);
}

void KovPlusChain::add_sentence(std::string sentence, const char separator, double strength) {
	Sentence rle = Sentence(bag, sentence, separator);

	for (auto resp_iter = rle.iterator(); resp_iter.has(); resp_iter.next()) {
		word_count[resp_iter.id()]++;
	
		if (resp_iter.has_next()) {
			Assessor new_assessor;

			new_assessor.strength = strength;

			for (int ctx_i = std::max(0, resp_iter.curr_index() - assessment_window_width); ctx_i <= resp_iter.curr_index(); ctx_i++) {
				new_assessor.context.push_back(rle.token_id(ctx_i));
			}
		
			int from_word = resp_iter.id();
			int to_word = resp_iter.offset(1).id();

			auto &to_word_index = response_assessment_index.emplace(from_word, std::unordered_map<int, std::vector<Assessor>>()).first->second;

			auto &answer_list = to_word_index.emplace(to_word, std::vector<Assessor>()).first->second;

			answer_list.push_back(new_assessor);
		}
	}
}

double KovPlusChain::get_assessment(const std::vector<int> &from, int to) const {
	double tally = 0.0;
	int from_word = from.back();

	if (response_assessment_index.find(from_word) == response_assessment_index.end()) {
		return 0.0;
	}

	auto to_index = response_assessment_index.at(from_word);

	if (to_index.find(to) == to_index.end()) {
		return 0.0;
	}

	auto assessors = to_index[to];

	for (auto assessor : assessors) {
		tally += assess(from, assessor);
	}

	return tally;
}

double KovPlusChain::get_assessment(const std::string &from, int to, const char separator) const {
	std::vector<int> from_words;
	std::istringstream token_reader(from);
	std::string token;

	while (std::getline(token_reader, token, separator)) {
		from_words.push_back(bag.get(token));
	}

	return get_assessment(from_words, to);
}

bool KovPlusChain::can_assess(const std::vector<int> &from) const {
	int from_word = from.back();
	
	if (response_assessment_index.find(from_word) == response_assessment_index.end()) {
		return false;
	}

	return true;
}

bool KovPlusChain::can_assess(const std::string &from, const char separator) const {
	std::vector<int> from_words;
	std::istringstream token_reader(from);
	std::string token;

	while (std::getline(token_reader, token, separator)) {
		from_words.push_back(bag.get(token));
	}

	return can_assess(from_words);
}

std::pair<double, std::vector<std::pair<double, int>>> KovPlusChain::get_assessments(const std::vector<int> &from) const {
	int from_word = from.back();
	double total_tally = 0.0;
	
	if (response_assessment_index.find(from_word) == response_assessment_index.end()) {
		return {0.0, {}};
	}

	auto to_index = response_assessment_index.at(from_word);
	std::vector<std::pair<double, int>> res_list;

	for (auto index_pair: to_index) {
		auto to = index_pair.first;
		auto assessors = index_pair.second;
	
		double tally = 0.0;

		for (auto assessor : assessors) {
			double assessment = assess(from, assessor);
			tally += assessment;
		}

		total_tally += tally;
		res_list.push_back({ tally, to });
	}

	return { total_tally, res_list };
}

// KovPlusQuery
const std::string &KovPlusQuery::make_next() {
	auto assessments = chain.get_assessments(context);

	double total_tally = assessments.first;
	double so_far_tally = 0.0;

	if (total_tally == 0) {
		return NULL_TOKEN;
	}

	double selection = distribution(*rng) * total_tally;

	for (auto assessment : assessments.second) {
		double this_tally = assessment.first;
		int this_to = assessment.second;

		so_far_tally += this_tally;

		if (selection < so_far_tally) {
			add_context(this_to);
			return bag.get(this_to);
		}
	}

	//std::cerr << "Warning: tally-based selection on '" << bag.get(context.back()) << "' gone wrong - selection " << selection << " but only went up to " << total_tally << "\n";
	return NULL_TOKEN;
}

void KovPlusQuery::make_up_to(int limit) {
	while (my_size < limit) {
		auto word = make_next();
	
		if (word == "") {
			break;
		}
	}
}
