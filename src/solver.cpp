#include <string>
#include <iostream>
#include <ctime>
#include <array>
#include <cmath>

#include "solver.h"
#include "message.h"
#include "key.h"
#include "language_data.h"


Solver::Solver(Message* message, Key* key) : message_(message), key_(key), bestKey_(nullptr),
											tempTabu_(std::unordered_set<std::string>()), 
											optimalTabu_((std::unordered_set<std::string>()))
{
	srand(time(nullptr));
}

void Solver::SetKey(Key* key)
{
	key_ = key;
}

void Solver::Start()
{
	int lastScore = CalculateScore(message_->Decrypt(*key_));
	int bestScore = lastScore;
	int currentBestScore = bestScore;
	unsigned int iterations = 0;
	const int maxTolerance = 40;
	const int endIterationShuffles = 5;
	const int tempClearProbability = 80;
	
	int tolerance = 0;
	int currentTolerance = 0;
	bool improved = false;
	int currentTabu = 0;
	
	Key* bestKey = new Key(*key_);
	Key* currentBestKey = new Key(*key_);

	currentBestScore = bestScore = lastScore = CalculateScore(message_->Decrypt(*key_));
	
	while (bestScore < 42000 && iterations < 30)
	{
		for (int p1 = 0; p1 < key_->GetLength(); p1++)
		{
			for (int p2 = 0; p2 < key_->GetLength(); p2++)
			{
				if (!key_->Swap(p1, p2)) continue;
				int score = CalculateScore(message_->Decrypt(*key_));				
				
				//TABU_STR_A would just assign key representation to a string
				
				//SET_SCORE starts here
				if (tempTabu_.find(key_->AsPlainText()) != std::end(tempTabu_) ||
					optimalTabu_.find(key_->AsPlainText()) != std::end(optimalTabu_))
				{
					//std::cout << "Blacklisted " << std::endl;
					score = -10000;
				}
				else
				{
					score = CalculateScore(message_->Decrypt(*key_));
				}
				//SET_SCORE ends here
				tempTabu_.insert(key_->AsPlainText()); //insert copy of that key
				//ADD_TEMP_TABU: inserts this key in temp_tabu. strange fucking way of doing it
				// fuck's sake, I suspect the map is just being used as a hashmap
				
				if (maxTolerance)
				{
					tolerance = rand() % (maxTolerance - currentTolerance + 1);
				}
				
				if (score < lastScore - tolerance)
				{
					key_->Swap(p1, p2); //undo last swap
				}
				else
				{
					lastScore = score;
					if (lastScore > bestScore)
					{
						improved = true;
						bestScore = lastScore;
						if (bestKey != nullptr) delete bestKey;
						bestKey = new Key(*key_);
					}
					
					if (score > currentBestScore)
					{
						currentBestScore = score;
						if (currentBestKey != nullptr) delete currentBestKey;
						currentBestKey = new Key(*key_);
					}
				}
			}
		}
		
		if (!improved)
		{
			if (++currentTolerance >= maxTolerance)
			{
				currentTolerance = 0;
			}
			
			//tabu black magic
			currentTabu++;
			if (currentTabu >= 300 /* max tabu */)
			{
				optimalTabu_.insert(currentBestKey->AsPlainText());
				currentBestScore = -10000;
				
				if (rand() % 2)
				{
					key_->RandomShuffle(100);
				}
				else
				{
					if (key_!= nullptr) delete key_;
					key_ = new Key(*bestKey);
				}
				currentTabu = 0;
			}
		}
		else
		{
			currentTolerance = 0;
			currentTabu = 0; //whatever the fuck it is
		}
		key_->RandomShuffle(endIterationShuffles); //info->swaps
		if (tempTabu_.find(key_->AsPlainText()) != std::end(tempTabu_) ||
			optimalTabu_.find(key_->AsPlainText()) != std::end(optimalTabu_))
		{
			lastScore = -10000;
		}
		else
		{
			lastScore = CalculateScore(message_->Decrypt(*key_));
		}
		
		if ((rand() % 100) < tempClearProbability) { //CLEAR_TABU_PROB
		//if (!(rand() % tempClearProbability)) {
			std::cout << "Clearing temp" << std::endl;
			tempTabu_.clear();
		}
		
		tolerance = 0;
		
		iterations++;
		std::cout << "Iteration " << iterations - 1 << " done with score " << lastScore << ", best is " << bestScore << std::endl;
	} //end of hill climber loop
	bestKey_ = bestKey;

	std::cout << iterations << " iterations completed" << std::endl;
	std::cout << "Best key is " << bestKey->AsPlainText() << std::endl;
	std::cout << "Best score is " << bestScore << std::endl;
	std::cout << "And it decrypts to" << std::endl;
	std::cout << message_->Decrypt(*bestKey_);
	
}

int Solver::CalculateScore(std::string plaintext)
{
	unsigned int score = 0;
	unsigned int len = plaintext.size();
	unsigned int remaining = len;
	
	unsigned int biscore, triscore, tetrascore, pentascore;
	biscore = triscore = tetrascore = pentascore = 0;

	char c1 = plaintext[0];
	char c2 = plaintext[1];
	char c3 = plaintext[2];
	char c4 = plaintext[3];
	char c5 = plaintext[4];
	
	for (unsigned int i = 0; i < len - 1; i++)
	{
		std::string gram(1, plaintext[i]);
		gram += plaintext[i + 1];
		biscore += LanguageData::GetBigramFrequency(gram);
		
		if (remaining > 2) {
		  gram += plaintext[i + 2];
		  triscore += LanguageData::GetTrigramFrequency(gram);
		}
		
		if (remaining > 3) {
		  gram += plaintext[i + 3];
		  tetrascore += LanguageData::GetTetragramFrequency(gram);
		}
		
		if (remaining > 4) {
		  gram += plaintext[i + 4];
		  pentascore += LanguageData::GetPentagramFrequency(gram); //hehe, pentagram
		}
		
		c1 = c2;
		c2 = c3;
		c3 = c4;
		c4 = c5;
		c5 = plaintext[i + 5];
		remaining--;
	}
	
	score = pentascore + (tetrascore >> 1) + (triscore >> 2) + (biscore >> 3);
	
	//update statistics
	
//This is a perfectly normal way to value-initialize an array in C++11
//GCC is sadly being too paranoid about this
#pragma GCC diagnostic push	
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
	std::array<int, 26> freqs {};
#pragma GCC diagnostic pop
	int unique = 0;
	for (int i = 0; i < plaintext.size(); i++)
	{
		int idx = plaintext[i] - 'A';
		if (!freqs[idx])
		{
			unique++;
		}
		freqs[idx]++;
	}
	double probMass = (double)plaintext.size() / unique;
	double entropy = 0.0;
	double entropyProb = 0.0;
	double ioc = 0.0;
	double chi2 = 0.0;
	double chiTemp = 0.0;
	for (int i = 0; i < 26; i++)
	{
		if (freqs[i] > 1)
		{
			ioc += freqs[i] * (freqs[i] - 1);
		}
		
		if (freqs[i])
		{
			entropyProb = (double)freqs[i] / plaintext.size();
			entropy += entropyProb * (std::log(entropyProb) / std::log(2));

			chiTemp = freqs[i] - probMass;
			chiTemp *= chiTemp;
			chiTemp /= probMass;
			chi2 += chiTemp;
		}
	}
			
	ioc /= plaintext.size() * (plaintext.size() - 1);
	chi2 /= plaintext.size();
	entropy *= -1;
	
	double dioc = GetDIoC(plaintext);
	
	
	double multiplier = 1.0;
	multiplier *= 1.05 - (5 * abs(ioc - LanguageData::EnglishIoC));
	multiplier *= 1.05 - ((5 >> 1) * abs(dioc - LanguageData::EnglishDIoC));
	multiplier *= 1.05 - (5 * abs(chi2 - LanguageData::EnglishChi2)) / 60.0;
	multiplier *= 1.05 - (5 * abs(entropy - LanguageData::EnglishEntropy)) / 150.0;
	
	return int(score * multiplier);
}

double Solver::GetDIoC(std::string& plaintext)
{
	#pragma GCC diagnostic push	
	#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
	std::array<int, 676> freqs {};
	#pragma GCC diagnostic pop
	
	double dioc = 0;
	int count = 0;
	
	for (int i = 0; i < plaintext.size() - 1; i += 2)
	{
		int idx1 = plaintext[i] - 'A';
		int idx2 = plaintext[i + 1] - 'A';
		freqs[idx1 * 26 + idx2]++;
		count++;
	}
	for (int i = 0; i < freqs.size(); i++)
	{
		if(freqs[i] > 1)
		{
			dioc += freqs[i] * (freqs[i] - 1);
		}
	}
	
	dioc /= count * (count - 1);
	
	return dioc;
}

void Solver::TestScore(std::string& plaintext)
{
	std::cout << "Got " << CalculateScore(plaintext) << std::endl;
}
