#ifndef __LIST_H__
#define __LIST_H__

#include <stddef.h>

template <class T>
class List
{
private:
	static const int DUMMY_SIZE = sizeof(T);
	char mEnd[DUMMY_SIZE];
	T	*mEndT;
	size_t mLength;
public:
	List()
	{
		mEndT = reinterpret_cast<T*>(&mEnd);
		mEndT->next = mEndT->prev = mEndT;
		mLength = 0;
	}
	inline T* First(){ return mEndT->next; }
	inline T* Last() { return mEndT->prev; }
	inline T* End() { return mEndT; }
	inline size_t Length() const { return mLength; }
	inline void Prepend (T *t)
	{
		t->next = mEndT->next;
		mEndT->next->prev = t;
		t->prev = mEndT;
		mEndT->next = t;
		mLength++;
	}
	inline void Append (T *t)
	{
		t->prev = mEndT->prev;
		mEndT->prev->next = t;
		t->next = mEndT;
		mEndT->prev = t;
		mLength++;
	}
	inline void Insert (T *after, T *t)
	{
		after->next->prev = t;
		t->next = after->next;
		t->prev = after;
		after->next = t;
		mLength++;
	}
	inline T* Remove(T *t)
	{
		t->prev->next = t->next;
		t->next->prev = t->prev;
		mLength--;
		return t->next;
	}
	
	// delete all
	inline void Purge ()
	{
		T* t = First ();
		while (t != End())
		{
			T* next = t->next;
			delete t;
			t = next;
		}
		mEndT->next = mEndT->prev = mEndT;
		mLength = 0;
	}
};

#endif


