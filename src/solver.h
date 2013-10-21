#ifndef _SOLVER_H
#define _SOLVER_H

#include <string>
#include <unordered_set>
#include <memory>

class Message;
class Key;

class Solver
{
private:
	Message* message_;
	Key* key_;
	Key* bestKey_;
	int CalculateScore(std::string plaintext);
	double GetDIoC(std::string& plaintext);
	std::unordered_set<std::string> tempTabu_;
	std::unordered_set<std::string> optimalTabu_;
	
	Solver(const Solver&) = delete;
	Solver& operator=(const Solver&) = delete;
public:
	Solver();
	Solver(Message* message, Key* key);
	void SetKey(Key* key);
	void Start();
	void TestScore(std::string& plaintext);
};

#endif