#include "key.h"
#include "message.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>

Key::Key() : length_(0), key_(MapType()), cached_()
{}

Key::Key(const Key& other) : length_(other.length_), key_(other.key_), cached_()
{ }

Key& Key::operator=(const Key& other)
{
	length_ = other.length_;
	key_ = MapType(other.key_);
	cached_[0] = other.cached_[0];
	cached_[1] = other.cached_[1];
	
	return *this;
}

void Key::Init(const Message& message)
{
	std::map<char, int> letterHomophones;
	for (char c = 'A'; c <= 'Z'; c++)
	{
		letterHomophones.insert(std::pair<char, int> (c, message.GuessNumberOfHomophones(c)));
	}
	for (auto &pair : message.GetFrequencyMap())
	{
		char guess = 0;
		for (char c = 'A'; c <= 'Z'; c++)
		{
			if (letterHomophones[c] > 0)
			{
				guess = c;
				letterHomophones[c]--;
				break;
			}
		}
		int numericalGuess = static_cast<int>(guess) - 'A';
		key_[pair.first] = numericalGuess;
	}
	length_ = message.GetFrequencyMap().size();
}

int Key::GetPlainSymbol(const char cipherSymbol) const
{
	return key_.at(cipherSymbol);
}

void Key::SetMapSymbol(const char cipherSymbol, int plainSymbol)
{
	key_[cipherSymbol] = plainSymbol;
}

std::string Key::AsPlainText() const
{
	std::string s = "";
	for (auto pair : key_)
	{
		char c = (char)pair.second + 'A';
		s += c;
	}
	return s;
}

void Key::PrintKey() const
{
	for (auto pair : key_)
	{
		std::cout << pair.first << " : " << pair.second << " " << static_cast<char>(pair.second + 'A') << std::endl;
	}
	std::cout << std::endl;
}

int Key::GetLength() const
{
	return length_;
}

bool Key::Swap(size_t p1, size_t p2)
{
	MapType::iterator it = key_.begin();
	MapType::iterator it2 = key_.begin();
	if (p1 > key_.size() || p2 > key_.size())
	{
		std::cout << "impossible " << key_.size() << std::endl;
	}
	if (p1 == p2) {
		return false;
	}
	for (size_t i = 0; i < p1; i++, it++);
	for (size_t i = 0; i < p2; i++, it2++);
	//std::cout << "Swapping " << (*it).second.second << " and " << (*it2).second.second << std::endl;
	if ((*it).second == (*it2).second) {
		return false;
	}
	cached_[0] = (*it).first;
	cached_[1] = (*it2).first;
	std::swap((*it).second, (*it2).second);
	return true;
}

char* Key::GetCached()
{
	return cached_;
}

void Key::RandomShuffle(int times)
{
	for (int i = 0; i < times; i++)
	{
		int p1 = rand() % GetLength();
		int p2 = rand() % GetLength();
		
		Swap(p1, p2);
	}
}

