#include "kovplus.hpp"

#include <iostream>


int main() {
	WordBag bag;

	Sentence my_sentence = Sentence(bag, "Two rabbits gleefully jump around in the midst of moonlight");

	for (int i = 0; i < my_sentence.size(); i++) {
		std::cout << "(" << my_sentence.token_id(i) << ") " << my_sentence.token(i) << "\n";
	}
}