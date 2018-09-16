#include "refcount.h"

//think this stands for W3DMemoryPoolObject
// it might be the one creating the Clone functions

class W3DMPO// : public RefCountClass
{
public:
    // i think the glueEnforcer comes from this\
	// original is a pure virtual
	// but its always a return 4 in classes that derive from it
	// so ill just set it here
    virtual int const glueEnforcer() { return 4; }; 
    virtual ~W3DMPO() {}
};
