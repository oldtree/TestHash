/*=====================================================================
	GenericHashers.h - Some generis hasher classes

	Author: Per Nilsson

	Freeware and no copyright on my behalf. However, if you use the 
	code in some manner	I'd appreciate a notification about it
	perfnurt@hotmail.com

	Classes:

		class Hasher<int>

		class Hasher<const char*>

  Requirements:
		N/A

  Dependencies:
		No external dependencies

=====================================================================*/

#if !defined(GENERICHASHERS_H)
#define GENERICHASHERS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// A generic, empty, template for the hash function. 
template <class Key> class Hasher;

// If you implement your own Hasher: 
//   It's supposed to work like a function object (aka functor): It should have
//   a () operator defined that takes a const Key& and a size_t and return 
//   a size_t. See the generic hashers below...
//   Note: 
//     It's only required to have the proper () operator, it doesn't have
//     to be dependant on the classes below.
//------------------------------------------------------------------------ 
// Some generic hashers

// int
class Hasher<int>
{
public:
	size_t operator ()(const int& key, size_t size)
	{
		return (static_cast<unsigned int>(key)) % size;
	}
};

// const char*
class Hasher<const char*>
{
public:
	size_t operator ()(const char* key, size_t size)
	{
		// This is a rather common string hasher algorithm. Don't ask me to explain it :-P

		size_t h = 0;
		size_t g;
		while (*key)
		{
			h = (h << 4) + *key++;

			if ( (g = h & 0xF0000000)!=0)
				h ^= g >> 24;
			h &= ~g;
		}
		return (h % size);
	}
};

#endif // GENERICHASHERS_H