#ifndef INCLUDED_KOVPLUS_HPP
#define INCLUDED_KOVPLUS_HPP

#include <random>
#include <cassert>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <deque>


class WordBag {
private:
	std::vector<std::string> token_list;
	std::unordered_map<std::string, int> token_index;

public:
	WordBag();

	const std::string &get(int index) const;
	int get(std::string token) const;

	int add(std::string token);
};

class SentenceView;
class SentenceCursor;

class Sentence {
private:
	WordBag &bag;
	std::vector<int> tokens;
	int my_size;

public:
	Sentence(WordBag &bag) : bag(bag), my_size(0) {}

	Sentence(WordBag &bag, std::vector<int> tokens);
	Sentence(WordBag &bag, const std::string tokens, const char separator = ' ');

	WordBag &get_bag() const {
		return bag;
	}
	
	int size() const {
		return my_size;
	}

	const std::string &token(int index) const {
		return bag.get(tokens[index]);
	}
	
	int token_id(int index) const {
		return tokens[index];
	}

	void set_token_id(int index, int token) {
		tokens[index] = token;
	}

	void set_token(int index, std::string token) {
		tokens[index] = bag.add(token);
	}
	
	std::string str() const;

	void append(int index);
	void append(std::string token);

	SentenceView view();
	SentenceView slice(int start = 0, int end = -1);
	SentenceCursor iterator(int start = 0, int end = -1);
};

class SentenceView {
private:
	Sentence &sentence;
	int start;
	int end;

public:
	SentenceView(Sentence &sentence, int start, int end) : sentence(sentence), start(start), end(end) {}

	int get_start() const {
		return start;
	}

	int get_end() const {
		return end;
	}

	const Sentence &get_sentence() const {
		return sentence;
	}
	
	const std::string &token(int index) const {
		return sentence.token(index + start);
	}
	
	int token_id(int index) const {
		return sentence.token_id(index + start);
	}

	void set_token_id(int index, int token) {
		sentence.set_token_id(index + start, token);
	}

	void set_token(int index, std::string token) {
		sentence.set_token(index + start, token);
	}
	
	int size() const {
		return end - start;
	}

	WordBag &get_bag() const {
		return sentence.get_bag();
	}

	std::string str() const;

	SentenceCursor iterator();
};

class SentenceCursor {
private:
	SentenceView view;
	int curr;
	int end;

	int curr_id;
	const std::string *curr_token;

	void update_self();

public:
	SentenceCursor(SentenceView view, int start = 0, int end = -1) : view(view), curr(start), end(end) {
		if (has()) {
			update_self();
		}
	}

	int id() const {
		return curr_id;
	}
	
	const std::string &token() const {
		return *curr_token;
	}

	void set_token(std::string token) {
		view.set_token(curr, token);
	}

	int curr_index() const {
		return curr;
	}
	
	bool has() const;
	bool has_next() const;
	void next();

	SentenceCursor offset(int offs) const {
		return SentenceCursor(view, curr + offs, end);
	}
};

class KovPlusQuery;

struct Assessor {
	double strength;
	std::vector<int> context;
};

class KovPlusChain {
private:
	WordBag bag;
	std::unordered_map<int, std::unordered_map<int, std::vector<Assessor>>> response_assessment_index;
	std::unordered_map<int, int> word_count;
	int assessment_window_width;

public:
	KovPlusChain(int assessment_window_width) : assessment_window_width(assessment_window_width) {}

	WordBag &get_bag() {
		return bag;
	}

	void add_sentence(std::string sentence, const char separator = ' ', double strength = 1.0);

	bool can_assess(const std::vector<int> &from) const;
	bool can_assess(const std::string &from, const char separator) const;

	double assess(const SentenceView &tokens, const Assessor &assessor) const;
	double assess(const std::vector<int> &tokens, const Assessor &assessor) const;

	double get_assessment(const std::vector<int> &from, int to) const;
	double get_assessment(const std::string &from, int to, const char separator) const;
	
	double get_assessment(const std::vector<int> &from, std::string to) const {
		return get_assessment(from, bag.get(to));
	}

	double get_assessment(const std::string &from, std::string to, const char separator) const {
		return get_assessment(from, bag.get(to), separator);
	}

	std::pair<double, std::vector<std::pair<double, int>>> get_assessments(const std::vector<int> &from) const;
};

class KovPlusQuery {
private:
	KovPlusChain &chain;
	WordBag &bag;
	Sentence result;
	std::vector<int> context;
	std::default_random_engine *rng;
	int size;
	std::uniform_real_distribution<double> distribution;
	bool own_rng = false;

public:
	KovPlusQuery(KovPlusChain &chain, std::string start, const char separator = ' ', std::default_random_engine *rng = NULL) : chain(chain), bag(chain.get_bag()), result(chain.get_bag()), size(0), distribution(0.0, 1.0) {
		this->rng = (rng != NULL) ? rng : new std::default_random_engine();

		own_rng = (rng == NULL);
	
		std::istringstream start_reader(start);
		std::string token;
	
		while (std::getline(start_reader, token, separator)) {
			add_context(token);
		}
	}

	KovPlusQuery(KovPlusChain &chain, std::vector<int> start, std::default_random_engine *rng = NULL) : chain(chain), bag(chain.get_bag()), result(chain.get_bag()), size(0), distribution(0.0, 1.0) {
		this->rng = (rng != NULL) ? rng : new std::default_random_engine();

		own_rng = (rng == NULL);
	
		for (auto word : start) {
			add_context(word);
		}
	}

	KovPlusQuery(KovPlusChain &chain, std::vector<std::string> start, std::default_random_engine *rng = NULL) : chain(chain), bag(chain.get_bag()), result(chain.get_bag()), size(0), distribution(0.0, 1.0) {
		this->rng = (rng != NULL) ? rng : new std::default_random_engine();

		own_rng = (rng == NULL);
	
		for (auto word : start) {
			add_context(word);
		}
	}

	~KovPlusQuery() {
		if (own_rng) {
			delete rng;
		}
	}

	void add_context(std::string word) {
		context.push_back(bag.get(word));
		result.append(word);
		size++;
	}

	void add_context(int word) {
		context.push_back(word);
		result.append(word);
		size++;
	}

	std::string str() const {
		return result.str();
	}
	
	const Sentence &get() const {
		return result;
	}

	const std::string &make_next();

	void make_up_to(int limit);
};


#endif // INCLUDED_KOVPLUS_HPP