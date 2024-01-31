/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D In Game UI
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once
#include "always.h"
#include "ingameui.h"
class RenderObjClass;
class HAnimClass;

class W3DInGameUI : public InGameUI
{
public:
    W3DInGameUI();
    virtual ~W3DInGameUI() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Draw() override;
    virtual View *Create_View() override;
    virtual void Draw_Selection_Region();
    virtual void Draw_Move_Hints(View *view);
    virtual void Draw_Attack_Hints(View *view);
    virtual void Draw_Place_Angle(View *view);

private:
    RenderObjClass *m_moveHintRenderObj[MAX_MOVE_HINTS];
    HAnimClass *m_moveHintAnim[MAX_MOVE_HINTS];
    RenderObjClass *m_locater01;
    RenderObjClass *m_locater02;
};
