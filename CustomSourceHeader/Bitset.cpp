#include "stdafx.h"
#include "Bitset.h"

Bitset::Bitset()
{
	//cout << &bits << endl;
}

Bitset::~Bitset()
{

}

void Bitset::Write(unsigned int data, unsigned short bitSize)
{
	bits |= (data << bitOffset);

	bitOffset += bitSize;
}

void Bitset::PrintBitset()
{
	//std::cout << bits. << ;
}