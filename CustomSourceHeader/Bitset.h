#pragma once

class Bitset
{
public:
	Bitset();
	~Bitset();
	void Write(unsigned int data, unsigned short bitSize);

	void PrintBitset();

private:
	int bitsetSize = 0;

	std::bitset<25> bits;

	int bitOffset = 0;
};

