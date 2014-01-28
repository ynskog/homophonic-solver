#include "message.h"
#include "language_data.h"
#include "key.h"

#include <iostream>

using namespace std;

Message::Message(const string ciphertext) : ciphertext_(ciphertext), num_symbols_(), 
											uniform_(), freqmap_()
{
	Init();
	//printFrequencyMap();
}

void Message::Init()
{
	for (char c : ciphertext_)
	{
		auto bound = freqmap_.lower_bound(c);
		if (bound != end(freqmap_) && !(freqmap_.key_comp()(c, bound->first))) //found
		{
			bound->second++;
		}
		else
		{
			freqmap_.insert(bound, FreqMap::value_type(c, 1)); //not found
		}			
	}
	num_symbols_ = freqmap_.size();
	uniform_ = ciphertext_.size() / num_symbols_;
}

const string Message::GetCiphertext() const
{
	return ciphertext_;
}

const Message::FreqMap Message::GetFrequencyMap() const
{
	return freqmap_;
}

void Message::PrintFrequencyMap() const
{
	for (auto pair : freqmap_)
	{
		std::cout << pair.first << " : " << pair.second << endl;
	}
	std::cout << std::endl;
}

int Message::GuessNumberOfHomophones(char c) const
{
	int expectedInText = LanguageData::GetLetterFrequency(c) * ciphertext_.size();
	if (expectedInText == 0) expectedInText = 1;
	return max(expectedInText / uniform_, 1);
}

int Message::GetUniformNumber() const
{
	return uniform_;
}

const std::string Message::DecryptAsString(Key &key) const
{
	std::string s = "";
	std::vector<int> dec = *(DecryptInt(key).get());
	for (auto el : dec)
	{
		char c = (char)el + 'A';
		s += c;
	}
	return s;
}

unique_ptr<std::vector<int>> Message::DecryptInt(Key &key) const
{
	unique_ptr<std::vector <int>> v = unique_ptr<std::vector <int>>(new std::vector<int>(key.GetLength()));
	for (const char c : ciphertext_)
	{
		v.get()->push_back(key.GetPlainSymbol(c));
	}
	return v;
}