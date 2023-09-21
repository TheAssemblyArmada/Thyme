add_library(w3d_renderer STATIC
    animobj.cpp
    aabtree.cpp
    aabtreebuilder.cpp
    assetmgr.cpp
    bitmaphandler.cpp
    bmp2d.cpp
    boxrobj.cpp
    bwrender.cpp
    camera.cpp
    coltest.cpp
    colorspace.cpp
    composite.cpp
    dazzle.cpp
    ddsfile.cpp
    dx8caps.cpp
    dx8fvf.cpp
    dx8indexbuffer.cpp
    dx8polygonrenderer.cpp
    dx8renderer.cpp
    dx8texman.cpp
    dx8vertexbuffer.cpp
    dx8wrapper.cpp
    dynamesh.cpp
    hanimmgr.cpp
    hcanim.cpp
    hlod.cpp
    hrawanim.cpp
    htree.cpp
    htreemgr.cpp
    intersec.cpp
    light.cpp
    linegrp.cpp
    lightenv.cpp
    line3d.cpp
    loaders.cpp
    matinfo.cpp
    mapper.cpp
    matpass.cpp
    matrixmapper.cpp
    mesh.cpp
    meshgeometry.cpp
    meshmatdesc.cpp
    meshmdl.cpp
    missingtexture.cpp
    motchan.cpp
    nullrobj.cpp
    part_buf.cpp
    part_emt.cpp
    part_ldr.cpp
    pivot.cpp
    pointgr.cpp
    pot.cpp
    projector.cpp
    render2d.cpp
    render2dsentence.cpp
    rendobj.cpp
    rinfo.cpp
    scene.cpp
    segline.cpp
    seglinerenderer.cpp
    shader.cpp
    sortingrenderer.cpp
    streak.cpp
    streakrender.cpp
    surfaceclass.cpp
    synctextureloadtasklist.cpp
    texproject.cpp
    texture.cpp
    texturebase.cpp
    textureloader.cpp
    textureloadtask.cpp
    textureloadtasklist.cpp
    thumbnail.cpp
    thumbnailmanager.cpp
    vertmaterial.cpp
    w3d.cpp
    w3d_util.cpp
    w3dexclusionlist.cpp
    w3dformat.cpp
)

target_include_directories(w3d_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(w3d_renderer PUBLIC w3d_math w3d_saveload)

if(D3D8_FOUND)
    target_link_libraries(w3d_renderer PUBLIC d3d8)
endif()

if(USE_FREETYPE)
    target_link_libraries(w3d_renderer PRIVATE Freetype::Freetype)
    target_compile_definitions(w3d_renderer PUBLIC -DBUILD_WITH_FREETYPE)
endif()

if(USE_FONTCONFIG)
    target_link_libraries(w3d_renderer PRIVATE Fontconfig::Fontconfig)
    target_compile_definitions(w3d_renderer PUBLIC -DBUILD_WITH_FONTCONFIG)
endif()