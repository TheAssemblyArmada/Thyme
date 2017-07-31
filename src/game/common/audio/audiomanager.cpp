////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: AUDIOMANAGER.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Base class for managing the audio engine.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "audiomanager.h"

#ifdef THYME_STANDALONE
AudioManager *g_theAudio = nullptr;
#endif
