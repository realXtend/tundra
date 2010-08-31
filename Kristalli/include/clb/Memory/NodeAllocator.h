#ifndef NodeAllocator_h
#define NodeAllocator_h

template<typename T>
class DynamicNodeAllocator
{
public:
	T *AllocateNode() { return new T; }
	void DeleteNode(T *node) { delete node; }
};

#endif
