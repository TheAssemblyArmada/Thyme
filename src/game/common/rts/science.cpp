#include "science.h"

// ScienceStore *g_theScienceStore = nullptr;

#if 0
FieldParse __ZZN12ScienceStore29friend_parseScienceDefinitionEP3INIE12myFieldParse[6] =
{
    { "PrerequisiteSciences", &INI::parseScienceVector, NULL, 36 },
    { "SciencePurchasePointCost", &INI::parseInt, NULL, 48 },
    { "IsGrantable", &INI::parseBool, NULL, 52 },
    { "DisplayName", &INI::parseAndTranslateLabel, NULL, 16 },
    { "Description", &INI::parseAndTranslateLabel, NULL, 20 },
    { NULL, NULL, NULL, 0 }
};
#endif

void ScienceStore::Reset()
{
    for ( auto it = m_infoVec.begin(); it != m_infoVec.end(); ++it ) {
        if ( *it != nullptr ) {
            *it = reinterpret_cast<ScienceInfo*>((*it)->Delete_Overrides());
        }

        if ( *it == nullptr ) {
            m_infoVec.erase(it);
        }
    }
}
