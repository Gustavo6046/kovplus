#include "kovplus.hpp"

#include <cmath>


const std::string NULL_TOKEN = "";

// WordBag
WordBag::WordBag() {
	add("");
}

const std::string &WordBag::get(int index) const {
	if (index == -1) {
		return NULL_TOKEN;
	}

	return token_list.at(index);
}

int WordBag::get(std::string token) const {
	return token_index.at(token);
}

int WordBag::add(std::string token) {
	auto found = token_index.find(token);

	if (found != token_index.end()) {
		return found->second;
	}

	int new_index = token_list.size();
	
	token_list.push_back(token);
	token_index[token] = new_index;

	return new_index;
}

// Sentence
Sentence::Sentence(WordBag &bag, std::vector<int> tokens) : bag(bag), tokens(tokens), my_size(tokens.size()) {}

Sentence::Sentence(WordBag &bag, const std::string tokens, const char separator) : bag(bag), my_size(0) {
	std::istringstream token_reader(tokens);
	std::string token;
	std::vector<std::string> token_list;

	while (std::getline(token_reader, token, separator)) {
		this->tokens.push_back(bag.add(token));
		my_size++;
	}
}

std::string Sentence::str() const {
	std::ostringstream stringifier;

	for (int i = 0; i < my_size; i++) {
		stringifier << token(i);

		if (i + 1 < my_size) {
			stringifier << " ";
		}
	}

	return stringifier.str();
}

void Sentence::append(int index) {
	tokens.push_back(index);
	my_size++;
}

void Sentence::append(std::string token) {
	tokens.push_back(bag.add(token));
	my_size++;
}

SentenceView Sentence::view() {
	return SentenceView(*this, 0, my_size);
}

SentenceView Sentence::slice(int start, int end) {
	return SentenceView(*this, start, end);
}

SentenceCursor Sentence::iterator(int start, int end) {
	return SentenceCursor(view(), start, end);
}

// SentenceView
std::string SentenceView::str() const {
	std::ostringstream stringifier;

	int my_size = size();

	for (int i = 0; i < my_size; i++) {
		stringifier << token(i);

		if (i + 1 < my_size) {
			stringifier << " ";
		}
	}

	return stringifier.str();
}

SentenceCursor SentenceView::iterator() {
	return SentenceCursor(*this, 0, end - start);
}

// SentenceCursor
bool SentenceCursor::has() const {
	return curr < view.size() && (end == -1 || curr < end);
}

bool SentenceCursor::has_next() const {
	return (curr + 1) < view.size() && (end == -1 || (curr + 1) < end);
}
	
void SentenceCursor::next() {
	curr++;

	if (has()) {
		update_self();
	}
}

void SentenceCursor::update_self() {
	curr_id = view.token_id(curr);
	curr_token = &view.token(curr);
}
	
// AttentionAssessor
double AttentionAssessor::assess(const SentenceView &tokens) {
	double assessment = 0.0;

	int i = 0;
	int exp_i = 0;

	if (tokens.size() < (int) expected.size()) {
		exp_i = expected.size() - tokens.size();
	}

	else {
		i = tokens.size() - expected.size();
	}

	while (exp_i < width) {
		if (tokens.token_id(i) == expected[exp_i]) {
			double weight = (double) width / (1.0 + std::sqrt(width - exp_i));
			assessment += weight;
		}

		i++;
		exp_i++;
	}

	return assessment * strength;
}

double AttentionAssessor::assess(const std::vector<int> &tokens) {
	double assessment = 0.0;

	int i = 0;
	int exp_i = 0;

	if (tokens.size() < expected.size()) {
		exp_i = expected.size() - tokens.size();
	}

	else {
		i = tokens.size() - expected.size();
	}

	while (exp_i < width) {
		if (tokens[i] == expected[exp_i]) {
			double weight = (double) width / (1.0 + std::sqrt(width - exp_i));
			assessment += weight;
		}

		i++;
		exp_i++;
	}

	return assessment * strength;
}

// KovPlusChain
void KovPlusChain::add_sentence(std::string sentence, const char separator, double strength) {
	responses.push_back(LogEntry(Sentence(bag, sentence, separator)));

	process_registered_response(responses.back(), strength);
}

void KovPlusChain::process_registered_response(LogEntry &rle, double strength) {
	for (auto resp_iter = rle.response.iterator(); resp_iter.has(); resp_iter.next()) {
		if (resp_iter.has_next()) {
			AttentionAssessor new_assessor = rle.get_assessor(resp_iter.curr_index() + 1, assessment_window_width, strength);
		
			int from_word = resp_iter.id();
			int to_word = resp_iter.offset(1).id();

			std::unordered_map<int, std::vector<AttentionAssessor>> &to_word_index = response_assessment_index.emplace(from_word, std::unordered_map<int, std::vector<AttentionAssessor>>()).first->second;

			std::vector<AttentionAssessor> &answer_list = to_word_index.emplace(to_word, std::vector<AttentionAssessor>()).first->second;

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
		tally += assessor.assess(from);
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
			double assessment = assessor.assess(from);
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

	//std::cerr << "Warning: tally-based selection on '" << bag.get(context.back()) << "' gone wrong - selection " << selection << " but only went up to " << total_tally << std::endl;
	return NULL_TOKEN;
}

void KovPlusQuery::make_up_to(int limit) {
	while (size < limit) {
		auto word = make_next();
	
		if (word == "") {
			break;
		}
	}
}

const LogEntry &KovPlusChain::fetch_log(int index) const {
		return responses[index];
}
