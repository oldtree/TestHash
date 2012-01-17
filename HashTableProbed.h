/*=====================================================================
	HashTableProbed.h - Probing hash table template class

	Author: Per Nilsson

	Freeware and no copyright on my behalf. However, if you use the 
	code in some manner	I'd appreciate a notification about it
	perfnurt@hotmail.com

	Classes:

		// The hash table 
		template <class Key, class Value, 
		  class MyHasher = Hasher<Key>,
		  class MyGrower = DefaultGrower,
		  >
		class HashTableProbed
		{
			class iterator
			class const_iterator

			// Used as a proxy when operator[] is called
			class Access			
		}

  Requirements:
		A Hasher must be implemented if the generic ones isn't applicable.
		Caller needs to #inlude default Grower/Hasher if they are to be used.

  Dependencies:
		To std::map if the default value_type is used

=====================================================================*/
#if !defined(HASHTABLEPROBED_H)
#define HASHTABLEPROBED_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>

//------------------------------------------------------------------------
// HashTableProbed
// A generic hash collection, requires that the Hasher
// has been implemented somwehere for the apropriate Key type.
// It will simply hold ann array of <Key, Value> pairs, and using probing
// to handle collisions
// class Key
//   The, well, key type
// class Value: 
//   The value type
// class MyHasher:
//   A class (function object) that will be called when computing the...well...hash value.
//   Substitute with your own if the generic hashers aren't good enough/applicable
// class MyGrower:
//   A class used to determine what size the array should grow to
template <class Key, class Value, 
		  class MyHasher = Hasher<Key>,
		  class MyGrower = DefaultGrower
		  >
class HashTableProbed  
{
public:
	//------------------------------------------------------------------
	// Public Type Definitions
	//------------------------------------------------------------------
	typedef std::pair<Key, Value> value_type;

	//------------------------------------------------------------------
	// Public Classes
	//------------------------------------------------------------------
	// class iterator
	class iterator
	{
	public:
		iterator(HashTableProbed& ht, size_t index):mHT(ht), mIndex(index) 
		{
			if (index<mHT.getAllocated() && mHT.getElement(index)==0)
			{
				++(*this);
			}
		}

		bool operator == (const iterator& src) const
		{
			return mHT == src.mHT && mIndex == src.mIndex;
		}

		bool operator != (const iterator& src) const
		{
			return !(*this == src);
		}

		value_type& operator*()
		{
			return *mHT.getElement(mIndex);
		}

		size_t getIndex() const { return mIndex; }

		iterator& operator ++ ()
		{
			mIndex++;
			while (mIndex<mHT.getAllocated() && mHT.getElement(mIndex)==0)
			{
				mIndex++;
			}
			return (*this);
		}

	private:
		HashTableProbed& mHT;
		size_t mIndex;
	};
	// class const_iterator
	class const_iterator
	{
	public:
		const_iterator(const HashTableProbed& ht, size_t index):mHT(ht), mIndex(index) 
		{
			if (index<mHT.getAllocated() && mHT.getElement(index)==0)
			{
				++(*this);
			}
		}


		bool operator == (const const_iterator& src) const
		{
			return mHT == src.mHT && mIndex == src.mIndex;
		}

		bool operator != (const const_iterator& src) const
		{
			return !(*this == src);
		}

		const value_type& operator*() const
		{
			return *mHT.getElement(mIndex);
		}

		size_t getIndex() const { return mIndex; }

		const_iterator& operator ++ ()
		{
			mIndex++;
			while (mIndex<mHT.getAllocated() && mHT.getElement(mIndex)==0)
			{
				mIndex++;
			} 
			return (*this);
		}

	private:
		const HashTableProbed& mHT;
		size_t mIndex;
	};

	// Used as a proxy when operator[] is called
	// Handles theHash["foo"] = 42 and i = theHash["Foo"] differently.
	class Access
	{
	public:
		Access(HashTableProbed& ht, const Key& key):mHash(ht),mKey(key){}

		// Assignment operator. Handles the myHash["Foo"] = 32; situation
		operator=(const Value& value)
		{
			// Just use the Set method, it handles already exist/not exist situation
			mHash.set(mKey,value);
		}

		// ValueType operator
		operator Value()
		{
			HashTableProbed::iterator i = mHash.find(mKey);

			// Not found
			if (i==mHash.end())
			{
				throw "Item not found";
			}

			return (*i).second;
		}
	private:
		//------------------------------
		// Disabled Methods
		//------------------------------
		// Default constructor
		Access();

		//------------------------------
		// Private Members
		//------------------------------
		HashTableProbed& mHash;
		const Key& mKey;
	}; // Access
	
	//------------------------------------------------------------------ 
	// Public Construction
	//------------------------------------------------------------------
	// Default constructor
	explicit HashTableProbed(size_t initialSize=1000) // Might be adjusted upwards
	{
		
		size_t newAlloc = mGrower.getPrimeGreaterThan(initialSize);

		mAllocated = newAlloc;
		mFreeSlots = mAllocated;
		mArray = new value_type*[mAllocated+1];
		memset(mArray, 0, sizeof(mArray[0])*(mAllocated+1));
		mSize=0;
	}

	// Destructor
	virtual ~HashTableProbed()
	{
		clear();
	}

	//------------------------------------------------------------------
	// Public Queries
	//------------------------------------------------------------------
	
	// Find a non-const iterator, returns end() if not found.
	const_iterator find(const Key& key) const
	{
		size_t hashValue = hash(key, mAllocated);
		size_t index = hashValue;

		const value_type* element = mArray[index];
		const Value* v=0;

		bool searchedAll = false;

		while (!searchedAll)
		{
			if (element!=0 && element->first == key)
			{
				v = &element->second;
				break;
			}

			index = (index + cIncBy) % mAllocated;
			searchedAll = index==hashValue;
			element = mArray[index];
		}
		if(v == 0)
			return end();
		else
			return const_iterator(*this,index);
	}


	// Find a non-const iterator, returns end() if not found.
	iterator find(const Key& key) 
	{
		size_t hashValue = hash(key, mAllocated);
		size_t index = hashValue;

		value_type* element = mArray[index];
		Value* v=0;

		bool searchedAll = false;

		while (!searchedAll)
		{
			if (element!=0 && element->first == key)
			{
				v = &element->second;
				break;
			}

			index = (index + cIncBy) % mAllocated;
			searchedAll = index==hashValue;
			element = mArray[index];
		}
		if(v == 0)
			return end();
		else
			return iterator(*this,index);
	}

	size_t size() const { return mSize; }

	value_type* getElement(size_t index) { return mArray[index]; }
	const value_type* getElement(size_t index) const { return mArray[index]; }
	size_t getAllocated() const { return mAllocated; }

	//------------------------------------------------------------------
	// Public Commands
	//------------------------------------------------------------------

	void set(const Key& key, const Value& value)
	{
		iterator i = find(key);
		if (i == end())
		{
			if (!insert(key, value))
				throw "Failed to insert";
		}
		else
		{
			(*i).second = value;
		}
			
	}

	// insert - returns false if no insertion took place
	// key already stored or no free slots
	bool insert(const value_type& vt)
	{
		return insert(vt.first, vt.second);
	}

	// insert - returns false if no insertion took place
	// key already stored or no free slots
	bool insert(const Key& key, const Value& value)
	{
		if (find(key) != end())
			return false;

		size_t newAlloc = mGrower.getNewSize(mAllocated, mFreeSlots);
		bool allSearched=false;
		if (newAlloc > mAllocated)
		{
			rehash(newAlloc);
		}

		size_t hashValue = hash(key, mAllocated);
		size_t index = hashValue;
		value_type* element = mArray[hashValue];

		
		while (element!=0 && !allSearched)
		{
			index = (index + cIncBy) % mAllocated;
			element = mArray[index];

			allSearched = index == hashValue;
		}

		if (allSearched)
			return false;
		else
		{
			element= new value_type(key, value);
			mArray[index] = element;
			mFreeSlots--;
			mSize++;
			return true;
		};
	}

	size_t erase(const Key& key)
	{
		iterator it = find(key);
		size_t erased=0;

		if (it != end())
		{
			size_t index = it.getIndex();
			delete mArray[index];
			mArray[index] = 0;
			mFreeSlots++;
			mSize--;
			erased++;
		}

		return erased;
	}

	void clear()
	{
		if (mArray != 0)
		{
			for (size_t i=0;i<mAllocated;++i)
			{
				delete mArray[i];
				mArray[i] = 0;
			}

			delete [] mArray;
			mArray = 0;
		}
		mAllocated=0;
		mFreeSlots=0;
		mSize=0;
	}
	//------------------------------------------------------------------
	// Public Operators
	//------------------------------------------------------------------
	Access operator[](const Key& key)
	{
		return Access(*this, key);
	}

	bool operator == (const HashTableProbed& src) const
	{
		return mArray == src.mArray;
	}

	//------------------------------------------------------------------
	// Public Iterators
	//------------------------------------------------------------------
	iterator		begin() { return iterator(*this, 0); }
	const_iterator	begin() const { return const_iterator(*this, 0); }
	iterator		end() { return iterator(*this, mAllocated); }
	const_iterator	end() const { return const_iterator(*this, mAllocated); }

private:
	//------------------------------------------------------------------
	// Disabled Methods
	//------------------------------------------------------------------
    // Copy constructor
	explicit HashTableProbed(const HashTableProbed&);

	// Assignment operator
	HashTableProbed operator = (const HashTableProbed&);

	//------------------------------------------------------------------
	// Private Type Definitions
	//------------------------------------------------------------------    
	typedef value_type**	 Array; // == array of value_type pointer
	//------------------------------------------------------------------
	// Private Helper Methods
	//------------------------------------------------------------------
	size_t hash(const Key& key, size_t allocated) const
	{
		MyHasher hasher;
		size_t size_t = hasher(key, allocated);
		return size_t;
	}

	static void deleteElement(value_type* m)
	{
		delete m;
		m = 0;
	}

	// Create a new, bigger, array
	void rehash(size_t newAlloc)
	{
		size_t oldAllocated = mAllocated;
		Array newArray = new value_type*[newAlloc+1];
		memset(newArray, 0, sizeof(newArray[0])*(newAlloc+1));

		size_t newFreeSlots = newAlloc;

		for (size_t i=0; i<oldAllocated; ++i)
		{
			value_type* element = mArray[i];
			if(element)
			{
				size_t newHashValue = hash(element->first, newAlloc);
				size_t index = newHashValue;
				value_type* newElement = newArray[newHashValue];

				while (newElement!=0)
				{
					index = (index + cIncBy) % newAlloc;
					newElement = newArray[index];
				}

				// Simply move the element to the new array
				newArray[index] = element;
				newFreeSlots--;
			}
		}

		// Note: Dont delete the elements in the old array, they are moved
		// as-is to the new array.
		if (mArray!=0)
			delete [] mArray;
		mArray = newArray;
		mAllocated = newAlloc;
		mFreeSlots = newFreeSlots;
	}
	//------------------------------------------------------------------
	// Private Constants
	//------------------------------------------------------------------
	// Probing increment
	enum { cIncBy = 7 };

	//------------------------------------------------------------------
	// Members
	//------------------------------------------------------------------
	// The hash table iteself
	Array	mArray;
	size_t	mAllocated; // The actual size of the array
	size_t	mFreeSlots; // Number of free slots in the array
	size_t	mSize;	// Number of elements stored in the hash table (incl. sub collections)

	MyGrower mGrower;

};

#endif // !defined(HASHTABLEPROBED_H)
