include(TargetExports)

add_library(proxydll SHARED proxydll.cpp hooker.cpp hooker.h mapview.cpp mapview.h sha1.cpp sha1.h)
target_link_libraries(proxydll PRIVATE base)
set_target_properties(proxydll PROPERTIES OUTPUT_NAME avifil32 PDB_NAME proxydll)
target_compile_definitions(proxydll PRIVATE $<$<CONFIG:DEBUG>:GAME_DEBUG>)
target_exports(proxydll SYMBOLS 
    AVIStreamWrite=FakeAVIStreamWrite
    AVIFileInit=FakeAVIFileInit
    AVIFileOpen=FakeAVIFileOpen
    AVIFileOpenA=FakeAVIFileOpenA
    AVIFileReadData=FakeAVIFileReadData
    AVIFileRelease=FakeAVIFileRelease
    AVIFileExit=FakeAVIFileExit
    AVIFileCreateStreamA=FakeAVIFileCreateStreamA
    AVIStreamSetFormat=FakeAVIStreamSetFormat
    AVIStreamRelease=FakeAVIStreamRelease
)
