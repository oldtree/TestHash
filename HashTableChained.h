/*=====================================================================
	HashTableChained.h - Chained hash table template class

	Author: Per Nilsson

	Freeware and no copyright on my behalf. However, if you use the 
	code in some manner	I'd appreciate a notification about it
	perfnurt@hotmail.com

	Classes:

		// The hash table 
		template <class Key, class Value, 
		  class MyHasher = Hasher<Key>,
		  class MyGrower = DefaultGrower,
		  class Collection = std::map<Key, Value> 
		  >
		class HashTableChained
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
		To std::map if the default Collection is used

=====================================================================*/
#if !defined(HASHTABLECHAINED_H)
#define HASHTABLECHAINED_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <memory> // std::auto_ptr

//------------------------------------------------------------------------
// HashTableChained
// A generic hash collection, requires that the Hasher
// has been implemented somwehere for the apropriate Key type.
// class Key
//   The, well, key type
// class Value: 
//   The value type
// class MyHasher:
//   A class (function object) that will be called when computing the...well...hash value.
//   Substitute with your own if the generic hashers aren't good enough/applicable
// class MyGrower:
//   A class used to determine what size the array should grow to
// class Collection:
//   The HashTableChained will actually be an array of Collection*, which is a 
//   pointer to a std::map<Key, Value> by default. 
//   You could let it be any other <Key, Value> collection type given that 
//   it follows the same form as a std::map 
//   (insert, find, erase, value_type, iterators etc) 
template <class Key, class Value, 
		  class MyHasher = Hasher<Key>,
		  class MyGrower = DefaultGrower,
		  class Collection = std::map<Key, Value> 
		  >
class HashTableChained  
{
public:
	//------------------------------------------------------------------
	// Public Type Definitions
	//------------------------------------------------------------------
	typedef Collection::value_type value_type;

	//------------------------------------------------------------------
	// Public Classes
	//------------------------------------------------------------------
	// class iterator
	class iterator
	{
	public:

		iterator(HashTableChained& ht, size_t index):mHT(ht), mIndex(index), mIterator(0)
		{
			inc();
		}
		iterator(HashTableChained& ht, size_t index, Collection::iterator it):mHT(ht), mIndex(index), mIterator(new Collection::iterator(it)) 
		{
		}

		iterator& operator = (const iterator& src)
		{
			mIndex = src.index;
			mIterator = src.mIterator;
		}
		bool operator == (const iterator& src) const
		{
			return mHT == src.mHT && mIndex == src.mIndex && mIterator.get() == src.mIterator.get();
		}

		bool operator != (const iterator& src) const
		{
			return !(*this == src);
		}

		value_type& operator*()
		{
			return **(mIterator.get());
		}

		size_t getIndex() const { return mIndex; }

		iterator& operator ++ ()
		{
			// Have we reached the end()?
			if (mIndex<mHT.getAllocated())
			{				
				// The hash table won't hold any empty collections,
				Collection::iterator& iter = *(mIterator.get());
				Collection& collection = *mHT.getCollection(mIndex);

				// Have we reached the collection's end()?
				if (iter!=collection.end())
				{
					// No. Just to to next element in the collection
					++iter;

					// But now we might have reached the collection's end()
					if (iter==collection.end())
					{
						// Go find the next collection's begin() (if any)
						mIndex++;
						inc();
					}
				}

			}
			return (*this);
		}

	private:
		// Go to next used slot, or to end()
		void inc()
		{
			
			mIterator = CollectionIterator(0);
			while(mIndex<mHT.getAllocated() && mHT.getCollection(mIndex)==0)
			{
				mIndex++;
			}
			if(mIndex<mHT.getAllocated())
			{
				mIterator = CollectionIterator(new Collection::iterator(mHT.getCollection(mIndex)->begin()));
			}

		}

		HashTableChained& mHT;
		size_t mIndex;

		// auto_ptr is a nifty way to have "optional" members, as you dont have to worry about delete.
		// Just set it to 0 if it's not applicable. For this class it means that we might not have
		// an iterator to the collection, as we might not be pointing to a valid Collection, ie
		// iterator is at end().
		typedef std::auto_ptr<Collection::iterator> CollectionIterator;
		CollectionIterator mIterator;
	};

	// class const_iterator
	class const_iterator
	{
	public:

		const_iterator(const HashTableChained& ht, size_t index):mHT(ht), mIndex(index), mIterator(0)
		{
			inc();
		}
		const_iterator(const HashTableChained& ht, size_t index, Collection::const_iterator it):mHT(ht), mIndex(index), mIterator(new Collection::const_iterator(it)) 
		{
		}

		const_iterator& operator = (const const_iterator& src)
		{
			mIndex = src.index;
			mIterator = src.mIterator;
		}
		bool operator == (const const_iterator& src) const
		{
			return mHT == src.mHT && mIndex == src.mIndex && mIterator.get() == src.mIterator.get();
		}

		bool operator != (const const_iterator& src) const
		{
			return !(*this == src);
		}

		const value_type& operator*()
		{
			return **(mIterator.get());
		}

		size_t getIndex() const { return mIndex; }

		const_iterator& operator ++ ()
		{
			if (mIndex<mHT.getAllocated())
			{				
				Collection::const_iterator& iter = *(mIterator.get());
				const Collection& collection = *mHT.getCollection(mIndex);

				if (iter!=collection.end())
				{
					++iter;
					if (iter==collection.end())
					{
						mIndex++;
						inc();
					}
				}
			}
			return (*this);
		}
	private:
		void inc()
		{
			mIterator = CollectionIterator(0);
			while(mIndex<mHT.getAllocated() && mHT.getCollection(mIndex)==0)
			{
				mIndex++;
			}
			if(mIndex<mHT.getAllocated())
			{
				mIterator = CollectionIterator(new Collection::const_iterator(mHT.getCollection(mIndex)->begin()));
			}

		}
		typedef std::auto_ptr<Collection::const_iterator> CollectionIterator;
		const HashTableChained& mHT;
		size_t mIndex;
		CollectionIterator mIterator;
	};
	
	// Used as proxy when calling the [] operator.
	// Handles theHash["foo"] = 42 and i = theHash["Foo"] differently.
	class Access
	{

	public:
		Access(HashTableChained& ht, const Key& key):mHash(ht),mKey(key){}

		// Assignment operator. Handles the myHash["Foo"] = 32; situation
		operator=(const Value& value)
		{
			// Just use the Set method, it handles already exist/not exist situation
			mHash.set(mKey,value);
		}

		// ValueType operator
		operator Value()
		{
			iterator it = mHash.find(mKey);

			// Not found
			if (it == mHash.end())
			{
				throw "Item not found";
			}

			return (*it).second;
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
		HashTableChained& mHash;
		const Key& mKey;
	}; // Access
	
	//------------------------------------------------------------------ 
	// Public Construction
	//------------------------------------------------------------------
	// Default constructor
	explicit HashTableChained(size_t initialSize=1000) // Might be adjusted upwards
	{
		
		size_t newAlloc = mGrower.getPrimeGreaterThan(initialSize);

		mAllocated = newAlloc;
		mFreeSlots = mAllocated;
		mArray = new Collection*[mAllocated+1];
		memset(mArray, 0, sizeof(mArray[0])*(mAllocated+1));
		mSize=0;
	}

	// Destructor
	virtual ~HashTableChained()
	{
		clear();
	}

	//------------------------------------------------------------------
	// Public Queries
	//------------------------------------------------------------------
	
	// Find a const_iterator, returns end() if not found.
	const_iterator find(const Key& key) const
	{
		size_t hashValue = hash(key, mAllocated);

		const Collection* collection = mArray[hashValue];

		if (collection != 0)
		{
			Collection::const_iterator it = collection->find(key);
			if (it != collection->end())
			{
				return const_iterator(*this, hashValue, it);
			}
		}

		return end();
	}

	// Find a iterator, returns end() if not found.
	iterator find(const Key& key) 
	{
		size_t hashValue = hash(key, mAllocated);

		Collection* collection = mArray[hashValue];

		if (collection != 0)
		{
			Collection::iterator it = collection->find(key);
			if (it != collection->end())
			{
				return iterator(*this, hashValue, it);
			}
		}

		return end();
	}

	size_t size() const { return mSize; }
	size_t  getAllocated() const { return mAllocated; }

	//------------------------------------------------------------------
	// Public Iterators
	//------------------------------------------------------------------
	iterator		begin() { return iterator(*this, 0); }
	const_iterator	begin() const { return const_iterator(*this, 0); }
	iterator		end() { return iterator(*this, mAllocated); }
	const_iterator	end() const { return const_iterator(*this, mAllocated); }

	//------------------------------------------------------------------
	// Public Commands
	//------------------------------------------------------------------

	void set(const Key& key, const Value& value)
	{
		iterator it = find(key);
		if (it == end())
		{
			if (!insert(key, value))
				throw "Failed to insert";
		}
		else
		{
			(*it).second = value;
		}
			
	}

	// insert - returns false if no insertion took place, ie key already stored
	bool  insert(const value_type& vt)
	{
		return insert(vt.first, vt.second);
	}

	// insert - returns false if no insertion took place, ie key already stored
	bool insert(const Key& key, const Value& value)
	{
		if (find(key) != end())
			return false;

		size_t newAlloc = mGrower.getNewSize(mAllocated, mFreeSlots);
		if (newAlloc > mAllocated)
		{
			rehash(newAlloc);
		}

		size_t hashValue = hash(key, mAllocated);

		Collection* collection = mArray[hashValue];


		if (!collection)
		{
			collection= new Collection;
			mArray[hashValue] = collection;
			mFreeSlots--;
		}

		collection->insert(Collection::value_type(key, value)); 
		mSize++;
		return true;
	}

	size_t erase(const Key& key)
	{
		size_t erased = 0;

		size_t index = hash(key,mAllocated);
		
		Collection* collection = mArray[index];

		if (collection!=0)
		{
			erased = collection->erase(key);
			if (erased > 0)
			{
				if (collection->size() == 0)
				{
					delete collection;
					mArray[index] = 0;
					mFreeSlots++;
				}
				
			}
		}
		mSize-=erased;
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

	bool operator == (const HashTableChained& src) const
	{
		return mArray == src.mArray;
	}

	Collection*	getCollection(size_t index) { return mArray[index]; }
	const Collection*	getCollection(size_t index) const { return mArray[index]; }

private:
	//------------------------------------------------------------------
	// Disabled Methods
	//------------------------------------------------------------------
    // Copy constructor
	explicit HashTableChained(const HashTableChained&);

	// Assignment operator
	HashTableChained operator = (const HashTableChained&);

	//------------------------------------------------------------------
	// Private Type Definitions
	//------------------------------------------------------------------
	typedef Collection**	 Array; // == array of Collection pointer
	//------------------------------------------------------------------
	// Private Helper Methods
	//------------------------------------------------------------------
	size_t hash(const Key& key, size_t allocated) const
	{
		MyHasher hasher;
		size_t size_t = hasher(key, allocated);
		return size_t;
	}

	// Create a new, bigger, array
	void rehash(size_t newAlloc)
	{
		size_t oldAllocated = mAllocated;
		Array newArray = new Collection*[newAlloc+1];
		memset(newArray, 0, sizeof(newArray[0])*(newAlloc+1));

		size_t newFreeSlots = newAlloc;
		size_t oldSize=size();

		for (size_t i=0; i<oldAllocated; ++i)
		{
			const Collection* collection = mArray[i];
			if(collection)
			{
				for(Collection::const_iterator iElem=collection->begin();iElem!=collection->end();++iElem)
				{
					size_t newsize_t = hash((*iElem).first, newAlloc);
					Collection* newCollection = newArray[newsize_t];

					// We need create new collectiosn since it's not likely the collections themselves
					// will be identical to the ones the new array.
					if (!newCollection)
					{
						newCollection = new Collection;
						newArray[newsize_t] = newCollection;
						newFreeSlots--;
					}

					newCollection->insert(Collection::value_type((*iElem).first, (*iElem).second));
				}
			}
		}

		// Delete all old stuff, even the collections and their elements as new
		// ones have been created for the new array.
		clear();
		mArray = newArray;
		mAllocated = newAlloc;
		mFreeSlots = newFreeSlots;
		mSize = oldSize;
	}

	//------------------------------------------------------------------
	// Members
	//------------------------------------------------------------------
	// The hash table iteself
	Array	mArray;
	size_t	mAllocated; // The actual size of the array
	size_t	mFreeSlots; // Number of free slots in the array
	size_t	mSize;	// Number of collections stored in the hash table (incl. sub collections)

	MyGrower mGrower;
};

#endif // !defined(HASHTABLECHAINED_H)
