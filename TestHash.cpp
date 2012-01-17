/*=====================================================================
	TestHash.cpp - Console application to test that the various 
	HashTables work as expected.

	Author: Per Nilsson

	Freeware and no copyright on my behalf. However, if you use the 
	code in some manner	I'd appreciate a notification about it
	perfnurt@hotmail.com

====================================================================*/

#include "stdafx.h"

//--------------------------------------------------
// Only needs to be included if you want to use the 
// template's defaults
#include "GenericHashers.h" 
#include "DefaultGrower.h"
//--------------------------------------------------
#include "HashTableChained.h"
#include "HashTableProbed.h"

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

// Forward declarations
void test();

//-----------------------------------------------------------------------
// A <CString> Hasher that reuses the generic const char* hasher
// Note that HashTableChained.h has no idea that there's something called CString
// but it works anyway :-).
// Won't work in UNICODE though (in that case you could modify it to transform the chars to w_chars)
class Hasher<CString>
{
public:
	size_t operator ()(const CString& key, size_t size)
	{
		Hasher<const char*> stringHasher;
		return stringHasher((LPCTSTR)key, size);
	}
};

//-----------------------------------------------------------------------
// A Second <CString> hasher. Used when the chaining table is using another
// hash table. We dont want them to use exactly the same hash algorithm
// as they'd then always get conflicts for the same Key.
// Note that a hasher can be any class, it doesn't have to be
// a Hasher<Key> template. Just as long as it has the expected
// () operator
class SecondStringHasher
{
public:
	size_t operator ()(const CString& key, size_t size)
	{
		Hasher<const char*> stringHasher;
		CString keyReversed(key);
		keyReversed.MakeReverse();
		return stringHasher((LPCTSTR)keyReversed, size);
	}
};

//-----------------------------------------------------------------------
// A <std::string> Hasher that reuses the generic const char* hasher
class Hasher<std::string>
{
public:
	size_t operator ()(const std::string& key, size_t size)
	{
		Hasher<const char*> stringHasher;
		return stringHasher(key.c_str(), size);
	}
};

//-----------------------------------------------------------------------
// Main entry of console application.
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		std::cerr << _T("Fatal Error: MFC initialization failed") << std::endl;
		nRetCode = 1;
	}
	else
	{
		test();
	}
		
	return nRetCode;
}

// ---------------------- Macros for testing some code  ---------------------
// BEGIN_TEST: Setup some internal test variables.
#define BEGIN_TEST int test_count__=0; int test_failed__=0; 
// TEST(b): Test the boolean expression b.
#define TEST(b) { \
					bool test_result__ = b; \
					if (!test_result__)\
					{\
					  std::cout << "\nTEST(" << #b << ") failed. Line " << __LINE__ << "\n";\
					  test_failed__++;\
					}\
					test_count__++; \
				}

// END_TEST: Display summary of test result.
#define END_TEST std::cout << "\nFinished. " << test_count__ << " tests. ";(test_failed__==0 ? std::cout << "All OK!\n" : std::cout << test_failed__ << " failed\n");
// ---------------------------------------------------------------------------

//-----------------------------------------------------------------------
// Runs the tests...
void test()
{
	const int cItems = 2000;

	BEGIN_TEST;
	{
		std::cout << "Testing HashTableChained<CString, int>..." << std::endl;
		HashTableChained<CString, int> ht(0); // Initialize to 0 to enforce rehashing (just for test). 

		TEST(ht.size() == 0);
		ht["ACDC"] = 42;
		TEST(ht.size() == 1);
		ht["Ozzy"] = 12;
		ht["Metallica"] = 23;
		ht["Toy Dolls"] = 90;
		ht["Toy Dolls"] = 40;
		TEST(ht.size() == 4);

		TEST(ht["ACDC"] == 42);
		TEST(ht["Ozzy"] == 12);
		TEST(ht["Metallica"] == 23);
		TEST(ht["Toy Dolls"] == 40);

		TEST(ht.insert("Judas Priest",34));
		TEST(!ht.insert("Judas Priest",44));
		TEST(ht["Judas Priest"] == 34);
	}
	{
		std::cout << "Testing HashTableChained<int, int>..." << std::endl;
		HashTableChained<int, int> ht(0);
		int i=0;
		{
			for (HashTableChained<int, int>::iterator it=ht.begin();it!=ht.end();++it)
			{
				i++;
			}
			TEST(i==0);
		}

		for (i=0;i<cItems;++i)
		{
			TEST(ht.insert(i,i));
		}

		// size() returns number of items in the hash table, not the allocated array size, that's
		// something internal only the hash table (and possibly its iterators) should care about
		TEST(ht.size() == cItems);

		for (i=0;i<cItems;++i)
		{
			TEST(ht[i] == i);
		}

		i=0;
		std::cout << "Testing HashTableChained<int, int>::iterator..." << std::endl;
		{
			for (HashTableChained<int, int>::iterator it=ht.begin();it!=ht.end();++it)
			{
				i++;
			}
		}
		TEST(i==cItems);
		TEST((*ht.find(456)).second == 456);
		std::cout << "Testing HashTableChained<int, int>::const_iterator..." << std::endl;
		{
			const HashTableChained<int, int>& cht=ht;
			i=0;
			for (HashTableChained<int, int>::const_iterator cit=cht.begin();cit!=cht.end();++cit)
			{
				i++;
			}
			TEST(i==cItems);
			TEST((*cht.find(123)).second == 123);
		}
		std::cout << "Testing HashTableChained<int, int>::erase..." << std::endl;
		TEST(ht.erase(123) == 1);
		TEST(ht.find(123) == ht.end());
		TEST(ht.erase(123) == 0);

	}
	{
		std::cout << "Testing HashTableProbed<int, int>..." << std::endl;
		HashTableProbed<int, int> ht(0);
		for (int i=0;i<cItems;++i)
		{
			TEST(ht.insert(i,i));
		}
		TEST(ht.size() == cItems);

		for (i=0;i<cItems;++i)
		{
			TEST(ht[i] == i);
		}

		std::cout << "Testing HashTableProbed<int, int>::iterator..." << std::endl;
		i = 0;
		for (HashTableProbed<int, int>::iterator it=ht.begin();it!=ht.end();++it)
		{
			i++;
		}
		TEST(i==cItems);
		TEST((*ht.find(456)).second == 456);

		std::cout << "Testing HashTableProbed<int, int>::const_iterator..." << std::endl;
		{
			const HashTableProbed<int, int>& cht = ht;
			i = 0;
			for (HashTableProbed<int, int>::const_iterator cit=cht.begin();cit!=cht.end();++cit)
			{
				i++;
			}
			TEST(i==cItems);
			TEST((*cht.find(634)).second == 634);

		}
	}
	{
		std::cout << "Testing HashTableProbed<int, int>::erase..." << std::endl;
		HashTableProbed<int, int> ht(0);
		ht.insert(0,0);
		ht.insert(1,1);
		ht.insert(2,2);
		
		TEST(ht.size() == 3);
		TEST( ht.erase(1) == 1);
		TEST(ht.size() == 2);

		TEST((*ht.find(0)).second == 0);
		TEST(ht.find(1) == ht.end());
		TEST((*ht.find(2)).second == 2);

		TEST( ht.erase(1) == 0);

	}

	{
		std::cout << "Testing HashTableChained<..., Collection = HashTableProbed>..." << std::endl;
		typedef HashTableProbed<CString, int, SecondStringHasher, DefaultGrower> MyProbed;
		HashTableChained<CString, int, Hasher<CString>, DefaultGrower, MyProbed> ht(0);
		TEST(ht.size() == 0);
		ht["ACDC"] = 42;
		TEST(ht.size() == 1);
		ht["Ozzy"] = 12;
		ht["Metallica"] = 23;
		ht["Toy Dolls"] = 90;
		ht["Toy Dolls"] = 40;
		TEST(ht.size() == 4);

		TEST(ht["ACDC"] == 42);
		TEST(ht["Ozzy"] == 12);
		TEST(ht["Metallica"] == 23);
		TEST(ht["Toy Dolls"] == 40);

		ht.insert("Kiss", 12);
		ht.insert("Iron Maiden", 12);
		ht.insert("Rainbow", 12);
	}
	{
		std::cout << "Testing HashTableChained<..., Collection = HashTableProbed>::iterator..." << std::endl;

		typedef HashTableProbed<int, int> MyProbed;
		typedef HashTableChained<int, int, Hasher<int>, DefaultGrower, MyProbed> MyChained;
		MyChained ht(0);
		for (int i=0;i<cItems;++i)
		{
			TEST(ht.insert(i,i));
		}
		TEST(ht.size() == cItems);

		for (i=0;i<cItems;++i)
		{
			TEST(ht[i] == i);
		}

		i=0;
		for (MyChained::iterator it=ht.begin();it!=ht.end();++it)
		{
			i++;
		}
		TEST(i==cItems);
		std::cout << "Testing HashTableChained<..., Collection = HashTableProbed>::const_iterator..." << std::endl;
		{
			const MyChained& cht = ht;
			i=0;
			for (MyChained::const_iterator it=cht.begin();it!=cht.end();++it)
			{
				i++;
			}
			TEST(i==cItems);

		}

	}

	{
		std::cout << "Testing HashTableChained<..., Collection = HashTableChained>..." << std::endl;

		typedef HashTableChained<CString, int, SecondStringHasher, DefaultGrower> MyChained2; // Uses default Collection, ie std::map
		typedef HashTableChained<CString, int, Hasher<CString>, DefaultGrower, MyChained2> MyChained;
		MyChained ht(0);
		TEST(ht.size() == 0);
		ht["ACDC"] = 42;
		TEST(ht.size() == 1);
		ht["Ozzy"] = 12;
		ht["Metallica"] = 23;
		ht["Toy Dolls"] = 90;
		ht["Toy Dolls"] = 40;
		TEST(ht.size() == 4);

		TEST(ht["ACDC"] == 42);
		TEST(ht["Ozzy"] == 12);
		TEST(ht["Metallica"] == 23);
		TEST(ht["Toy Dolls"] == 40);

		TEST(ht.insert("Kiss", 12));
		TEST(ht.insert("Iron Maiden", 12));
		TEST(ht.insert("Rainbow", 12));
		TEST(!ht.insert("Rainbow", 23));
		TEST(ht["Rainbow"] == 12);

	}

	{
		std::cout << "Testing HashTableChained<..., Collection = HashTableChained>::iterator..." << std::endl;
		typedef HashTableChained<int, int> MyChained2;
		typedef HashTableChained<int, int, Hasher<int>, DefaultGrower, MyChained2> MyChained;
		MyChained ht(0);
		for (int i=0;i<cItems;++i)
		{
			TEST(ht.insert(i,i));
		}

		TEST(ht.size() == cItems);
		for (i=0;i<cItems;++i)
		{
			TEST(ht[i] == i);
		}

		i=0;
		for (MyChained::iterator it=ht.begin();it!=ht.end();++it)
		{
			i++;
		}
		TEST(i==cItems);
		std::cout << "Testing HashTableChained<..., Collection = HashTableChained>::const_iterator..." << std::endl;
		{
			const MyChained& cht = ht;
			i=0;
			for (MyChained::const_iterator it=cht.begin();it!=cht.end();++it)
			{
				i++;
			}
			TEST(i==cItems);

		}
	}
	END_TEST;
	char ch; 
	std::cout << "Press <Enter>";
	std::cin.getline(&ch,1);
}