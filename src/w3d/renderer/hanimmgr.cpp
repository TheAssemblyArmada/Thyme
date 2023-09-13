/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief Hashed animation manager.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "hanimmgr.h"
#include "hanim.h"
#include "hash.h"
#include "hcanim.h"
#include "hrawanim.h"
#include "w3d_file.h"
#include "w3dexclusionlist.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

HAnimManagerClass::HAnimManagerClass()
{
    m_animPtrTable = new HashTableClass(2048);
    m_missingAnimTable = new HashTableClass(2048);
}

// 0x00831C80
HAnimManagerClass::~HAnimManagerClass()
{
    Free_All_Anims();
    Reset_Missing();
    delete m_animPtrTable;
    m_animPtrTable = nullptr;
    Reset_Missing();
    delete m_missingAnimTable;
    m_missingAnimTable = nullptr;
}

// 0x00831CE0
W3DErrorType HAnimManagerClass::Load_Anim(ChunkLoadClass &cload)
{
    switch (cload.Cur_Chunk_ID()) {
        case W3D_CHUNK_ANIMATION:
            return Load_Raw_Anim(cload);
        case W3D_CHUNK_COMPRESSED_ANIMATION:
            return Load_Compressed_Anim(cload);
        case W3D_CHUNK_MORPH_ANIMATION:
            return Load_Morph_Anim(cload);
        default:
            return W3D_ERROR_OK;
    }
}

W3DErrorType HAnimManagerClass::Load_Morph_Anim(ChunkLoadClass &cload)
{
    captainslog_assert("Morph animations not supported");
    return W3D_ERROR_OK;
}

W3DErrorType HAnimManagerClass::Load_Raw_Anim(ChunkLoadClass &cload)
{
    HRawAnimClass *anim = new HRawAnimClass();

    if (!anim) {
        return W3D_ERROR_GENERIC;
    }

    if (anim->Load_W3D(cload) != W3D_ERROR_OK) {
        anim->Release_Ref();
        return W3D_ERROR_GENERIC;
    }

    if (Peek_Anim(anim->Get_Name()) != nullptr) {
        anim->Release_Ref();
        return W3D_ERROR_GENERIC;
    }

    Add_Anim(anim);
    anim->Release_Ref();
    return W3D_ERROR_OK;
}

W3DErrorType HAnimManagerClass::Load_Compressed_Anim(ChunkLoadClass &cload)
{
    HCompressedAnimClass *anim = new HCompressedAnimClass();

    if (!anim) {
        return W3D_ERROR_GENERIC;
    }

    if (anim->Load_W3D(cload) != W3D_ERROR_OK) {
        anim->Release_Ref();
        return W3D_ERROR_GENERIC;
    }

    if (Peek_Anim(anim->Get_Name()) != nullptr) {
        anim->Release_Ref();
        return W3D_ERROR_GENERIC;
    }

    Add_Anim(anim);
    anim->Release_Ref();
    return W3D_ERROR_OK;
}

// 0x00832000
HAnimClass *HAnimManagerClass::Get_Anim(const char *name)
{
    HAnimClass *anim = Peek_Anim(name);
    if (anim != nullptr) {
        anim->Add_Ref();
    }
    return anim;
}

// 0x00832160
void HAnimManagerClass::Create_Asset_List(DynamicVectorClass<StringClass> &list)
{
    // Not used
    HAnimManagerIterator it(*this);
    for (it.First(); !it.Is_Done(); it.Next()) {
        auto *hAnim = it.Get_Current_Anim();
        const auto *name = hAnim->Get_Name();
        const auto *period = strchr(name, '.');
        if (period == nullptr) {
            continue;
        }
        StringClass asset = period + 1;
        list.Add(asset);
    }
}

// 0x00832310
bool HAnimManagerClass::Add_Anim(HAnimClass *new_anim)
{
    captainslog_assert(new_anim != nullptr);
    if (new_anim != nullptr) {
        new_anim->Add_Ref();
    }
    m_animPtrTable->Add(new_anim);
    return true;
}

// 0x00832340
void HAnimManagerClass::Register_Missing(const char *name)
{
    MissingAnimClass *missing = new MissingAnimClass{ name };
    m_missingAnimTable->Add(missing);
}

// 0x00832430
unsigned char HAnimManagerClass::Is_Missing(const char *name)
{
    return m_missingAnimTable->Find(name) != nullptr;
}

// 0x00832450
void HAnimManagerClass::Reset_Missing()
{
    HashTableIteratorClass it(*m_missingAnimTable);
    for (it.First(); !it.Is_Done(); it.Next()) {
        auto *missing = it.Get_Current();
        captainslog_assert(missing != nullptr);
        delete missing;
    }
    m_missingAnimTable->Reset();
}

HAnimClass *HAnimManagerClass::Peek_Anim(const char *name)
{
    auto *value = m_animPtrTable->Find(name);
    if (value == nullptr) {
        return nullptr;
    }
    // TODO: This should be a dynamic_cast<HAnimClass*>
    return static_cast<HAnimClass *>(value);
}

// 0x00832030
void HAnimManagerClass::Free_All_Anims()
{
    HAnimManagerIterator it(*this);
    for (it.First(); !it.Is_Done(); it.Next()) {
        HAnimClass *hanim = it.Get_Current_Anim();
        captainslog_assert(hanim != nullptr);
        hanim->Release_Ref();
    }
    m_animPtrTable->Reset();
}

void HAnimManagerClass::Free_All_Anims_With_Exclusion_List(const W3DExclusionListClass &exclude_list)
{
    HAnimManagerIterator it(*this);

    for (it.First(); !it.Is_Done(); it.Next()) {
        HAnimClass *hanim = it.Get_Current_Anim();

        if (hanim->Num_Refs() == 1 && !exclude_list.Is_Excluded(hanim)) {
            m_animPtrTable->Remove(hanim);
            hanim->Release_Ref();
        }
    }
}

HAnimClass *HAnimManagerIterator::Get_Current_Anim()
{
    if (Get_Current() != nullptr) {
        // TODO: This should be dynamic_cast<HAnimClass*>
        return static_cast<HAnimClass *>(Get_Current());
    }
    return nullptr;
}
