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

#include "sentence.hpp"


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
	int my_size;
	std::uniform_real_distribution<double> distribution;
	bool own_rng = false;

public:
	KovPlusQuery(KovPlusChain &chain, std::string start, const char separator = ' ', std::default_random_engine *rng = NULL) : chain(chain), bag(chain.get_bag()), result(chain.get_bag()), my_size(0), distribution(0.0, 1.0) {
		this->rng = (rng != NULL) ? rng : new std::default_random_engine();

		own_rng = (rng == NULL);
	
		std::istringstream start_reader(start);
		std::string token;
	
		while (std::getline(start_reader, token, separator)) {
			add_context(token);
		}
	}

	KovPlusQuery(KovPlusChain &chain, std::vector<int> start, std::default_random_engine *rng = NULL) : chain(chain), bag(chain.get_bag()), result(chain.get_bag()), my_size(0), distribution(0.0, 1.0) {
		this->rng = (rng != NULL) ? rng : new std::default_random_engine();

		own_rng = (rng == NULL);
	
		for (auto word : start) {
			add_context(word);
		}
	}

	KovPlusQuery(KovPlusChain &chain, std::vector<std::string> start, std::default_random_engine *rng = NULL) : chain(chain), bag(chain.get_bag()), result(chain.get_bag()), my_size(0), distribution(0.0, 1.0) {
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
		add_context(bag.get(word));
	}

	void add_context(int word) {
		context.push_back(word);
		result.append(word);
		my_size++;
	}

	std::string str() const {
		return result.str();
	}
	
	const Sentence &get() const {
		return result;
	}

	int size() const {
		return my_size;
	}

	const std::string &make_next();

	void make_up_to(int limit);
};


#endif // INCLUDED_KOVPLUS_HPP