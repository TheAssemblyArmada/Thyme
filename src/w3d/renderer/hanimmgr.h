#pragma once

#include "chunkio.h"
#include "hash.h"
#include "wwstring.h"

class W3DExclusionListClass;
class DynamicVectorClass;
class HAnimClass;

class MissingAnimClass : public HashableClass
{
public:
    MissingAnimClass(const char *name) : Name(name) {}
    MissingAnimClass(MissingAnimClass &){}; // unknown if it just sets name or hash table too
    virtual ~MissingAnimClass() { m_name.Free_String(); };
    virtual const char *Get_Key() override { return m_name.Peek_Buffer(); };

private:
    StringClass m_name;
};

class HAnimManagerClass
{
public:
    HAnimManagerClass();
    ~HAnimManagerClass();
    int Load_Anim(ChunkLoadClass &cload);
    int Load_Morph_Anim(ChunkLoadClass &cload);
    int Load_Raw_Anim(ChunkLoadClass &cload);
    int Load_Compressed_Anim(ChunkLoadClass &cload);
    HAnimClass *Get_Anim(const char *name);
    void Free_All_Anims();
    void Free_All_Anims_With_Exclusion_List(W3DExclusionListClass const &);
    void Create_Asset_List(DynamicVectorClass<StringClass> &);
    unsigned char Add_Anim(HAnimClass *new_anim);
    void Register_Missing(const char *name);
    unsigned char Is_Missing(const char *name);
    void Reset_Missing();
    HAnimClass *Peek_Anim(const char *name);

private:
    HashTableClass *AnimPtrTable;
    HashTableClass *MissingAnimTable;
};

class HAnimManagerIterator : public HashTableIteratorClass
{
public:
    HAnimManagerIterator(HAnimManagerIterator &iterator) : HashTableIteratorClass(iterator) {}
    HAnimManagerIterator(HAnimManagerClass &animmanager) {} // code unknown currently
    virtual ~HAnimManagerIterator() {}
    HAnimClass *Get_Current_Anim()
    {
        if (Get_Current()) {
            return (HAnimClass *)Get_Current();//i think..
        }
        return 0;
    }
};
