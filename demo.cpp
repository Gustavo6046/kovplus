#include "kovplus.hpp"

#include <iostream>


int main() {
	WordBag bag;

	Sentence my_sentence = Sentence(bag, "Two rabbits gleefully jump around in the midst of moonlight, playfully hopping over each other in turns");

	for (auto iter = my_sentence.iterator(); iter.has(); iter.next()) {
		std::cout << "(" << iter.id() << ") " << iter.token() << "\n";
	}
}