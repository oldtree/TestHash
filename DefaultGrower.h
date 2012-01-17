/*=====================================================================
	DefaultGrower.h - A default grower for hash tables

	Author: Per Nilsson

	Freeware and no copyright on my behalf. However, if you use the 
	code in some manner	I'd appreciate a notification about it
	perfnurt@hotmail.com

	Classes:

		class DefaultGrower

  Requirements:
		N/A

  Dependencies:
		std::set

=====================================================================*/
#if !defined(DEFAULTGROWER_H)
#define DEFAULTGROWER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <set>

//------------------------------------------------------------------------
// DefaultGrower 
class DefaultGrower
{
public:
	// Default constructor
	DefaultGrower()
	{
		// We don't store _ALL_ primes, modify or make your own Grower if 
		// you find this inapropriate
		mPrimes.insert(5);
		mPrimes.insert(11);
		mPrimes.insert(23);
		mPrimes.insert(1009);
		mPrimes.insert(5009);
		mPrimes.insert(10009);
		mPrimes.insert(20011);
		mPrimes.insert(50021);
		mPrimes.insert(100003);
		mPrimes.insert(200003);
		mPrimes.insert(500009);
	}

	// Name is self explanatory I guess
	size_t getPrimeGreaterThan(size_t size) const
	{
		{
			PrimeSet::const_iterator i=mPrimes.find(size);

			if (i == mPrimes.end())
			{
				// size is not a prime number, gotta go looking for the next 
				// bigger prime. Only done the first time since queries after
				// initialization will give a size that is a prime.
				for (i=mPrimes.begin();i!=mPrimes.end(); ++i)
				{
					if (size<(*i))
						break;
				}
			}
			else
			{
				// size is a prime, just pick one following it
				i++;
			}

			// DefaultGrower has its limitations
			if (i == mPrimes.end())
				throw "Need more primes";

			return (*i);
		}
	}

	// Called by HashTable to figure out if the array needs to grow.
	// If returned value <= currentSize array won't grow.
	size_t getNewSize(size_t currentSize, size_t freeSlots) const
	{
		size_t newSize = currentSize;

		// Simple algoritm: Make sure at least 10% slots are free.
		while (freeSlots <= (newSize/10))
		{
			newSize =  getPrimeGreaterThan(newSize);
			// As the array grows more slots will be available
			freeSlots = freeSlots + (newSize-currentSize); 
		}
		return newSize;
	}


private:
	//------------------------------------------------------------------
	// Private Type Definitions
	//------------------------------------------------------------------
	typedef std::set<size_t> PrimeSet;

	//------------------------------------------------------------------
	// Members
	//------------------------------------------------------------------
	PrimeSet mPrimes;
};

#endif // DEFAULTGROWER_H