project "assimp"
  kind "StaticLib"
  language "C++"
  cppdialect "C++20"
  staticruntime "on"
  optimize "speed"

  targetdir ("%{wks.location}/Build/" .. outputdir .. "/Dependencies/%{prj.name}")
  objdir ("%{wks.location}/Build/Intermediate/" .. outputdir .. "/Dependencies/%{prj.name}")

  defines 
  {
      -- "SWIG",
      "ASSIMP_BUILD_NO_OWN_ZLIB",

      "ASSIMP_BUILD_NO_X_IMPORTER",
      "ASSIMP_BUILD_NO_3DS_IMPORTER",
      "ASSIMP_BUILD_NO_MD3_IMPORTER",
      "ASSIMP_BUILD_NO_MDL_IMPORTER",
      "ASSIMP_BUILD_NO_MD2_IMPORTER",
      -- "ASSIMP_BUILD_NO_PLY_IMPORTER",
      "ASSIMP_BUILD_NO_ASE_IMPORTER",
      -- "ASSIMP_BUILD_NO_OBJ_IMPORTER",
      "ASSIMP_BUILD_NO_AMF_IMPORTER",
      "ASSIMP_BUILD_NO_HMP_IMPORTER",
      "ASSIMP_BUILD_NO_SMD_IMPORTER",
      "ASSIMP_BUILD_NO_MDC_IMPORTER",
      "ASSIMP_BUILD_NO_MD5_IMPORTER",
      "ASSIMP_BUILD_NO_STL_IMPORTER",
      "ASSIMP_BUILD_NO_LWO_IMPORTER",
      "ASSIMP_BUILD_NO_DXF_IMPORTER",
      "ASSIMP_BUILD_NO_NFF_IMPORTER",
      "ASSIMP_BUILD_NO_RAW_IMPORTER",
      "ASSIMP_BUILD_NO_OFF_IMPORTER",
      "ASSIMP_BUILD_NO_AC_IMPORTER",
      "ASSIMP_BUILD_NO_BVH_IMPORTER",
      "ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
      "ASSIMP_BUILD_NO_IRR_IMPORTER",
      "ASSIMP_BUILD_NO_Q3D_IMPORTER",
      "ASSIMP_BUILD_NO_B3D_IMPORTER",
      -- "ASSIMP_BUILD_NO_COLLADA_IMPORTER",
      "ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
      "ASSIMP_BUILD_NO_CSM_IMPORTER",
      "ASSIMP_BUILD_NO_3D_IMPORTER",
      "ASSIMP_BUILD_NO_LWS_IMPORTER",
      "ASSIMP_BUILD_NO_OGRE_IMPORTER",
      "ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
      "ASSIMP_BUILD_NO_MS3D_IMPORTER",
      "ASSIMP_BUILD_NO_COB_IMPORTER",
      "ASSIMP_BUILD_NO_BLEND_IMPORTER",
      "ASSIMP_BUILD_NO_Q3BSP_IMPORTER",
      "ASSIMP_BUILD_NO_NDO_IMPORTER",
      "ASSIMP_BUILD_NO_IFC_IMPORTER",
      "ASSIMP_BUILD_NO_XGL_IMPORTER",
      --"ASSIMP_BUILD_NO_FBX_IMPORTER",
      "ASSIMP_BUILD_NO_ASSBIN_IMPORTER",
      -- "ASSIMP_BUILD_NO_GLTF_IMPORTER",
      "ASSIMP_BUILD_NO_C4D_IMPORTER",
      "ASSIMP_BUILD_NO_3MF_IMPORTER",
      "ASSIMP_BUILD_NO_X3D_IMPORTER",
      "ASSIMP_BUILD_NO_MMD_IMPORTER",
      
      "ASSIMP_BUILD_NO_STEP_EXPORTER",
      "ASSIMP_BUILD_NO_SIB_IMPORTER",

      -- "ASSIMP_BUILD_NO_MAKELEFTHANDED_PROCESS",
      -- "ASSIMP_BUILD_NO_FLIPUVS_PROCESS",
      -- "ASSIMP_BUILD_NO_FLIPWINDINGORDER_PROCESS",
      -- "ASSIMP_BUILD_NO_CALCTANGENTS_PROCESS",
      "ASSIMP_BUILD_NO_JOINVERTICES_PROCESS",
      -- "ASSIMP_BUILD_NO_TRIANGULATE_PROCESS",
      "ASSIMP_BUILD_NO_GENFACENORMALS_PROCESS",
      -- "ASSIMP_BUILD_NO_GENVERTEXNORMALS_PROCESS",
      "ASSIMP_BUILD_NO_REMOVEVC_PROCESS",
      "ASSIMP_BUILD_NO_SPLITLARGEMESHES_PROCESS",
      "ASSIMP_BUILD_NO_PRETRANSFORMVERTICES_PROCESS",
      "ASSIMP_BUILD_NO_LIMITBONEWEIGHTS_PROCESS",
      -- "ASSIMP_BUILD_NO_VALIDATEDS_PROCESS",
      "ASSIMP_BUILD_NO_IMPROVECACHELOCALITY_PROCESS",
      "ASSIMP_BUILD_NO_FIXINFACINGNORMALS_PROCESS",
      "ASSIMP_BUILD_NO_REMOVE_REDUNDANTMATERIALS_PROCESS",
      "ASSIMP_BUILD_NO_FINDINVALIDDATA_PROCESS",
      "ASSIMP_BUILD_NO_FINDDEGENERATES_PROCESS",
      "ASSIMP_BUILD_NO_SORTBYPTYPE_PROCESS",
      "ASSIMP_BUILD_NO_GENUVCOORDS_PROCESS",
      "ASSIMP_BUILD_NO_TRANSFORMTEXCOORDS_PROCESS",
      "ASSIMP_BUILD_NO_FINDINSTANCES_PROCESS",
      "ASSIMP_BUILD_NO_OPTIMIZEMESHES_PROCESS",
      "ASSIMP_BUILD_NO_OPTIMIZEGRAPH_PROCESS",
      "ASSIMP_BUILD_NO_SPLITBYBONECOUNT_PROCESS",
      "ASSIMP_BUILD_NO_DEBONE_PROCESS",
      "ASSIMP_BUILD_NO_EMBEDTEXTURES_PROCESS",
      "ASSIMP_BUILD_NO_GLOBALSCALE_PROCESS",
  }

  files 
  {
      "%{ENGINE_PATH}/Code/Dependencies/assimp/include/**",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/Assimp.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/FBX/**.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/BaseImporter.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ColladaLoader.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ColladaParser.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/CreateAnimMesh.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/PlyParser.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/PlyLoader.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/BaseProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/EmbedTexturesProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ConvertToLHProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/DefaultIOStream.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/DefaultIOSystem.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/DefaultLogger.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/GenVertexNormalsProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/Importer.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ImporterRegistry.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/MaterialSystem.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/PostStepRegistry.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ProcessHelper.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/scene.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ScenePreprocessor.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ScaleProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/SGSpatialSort.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/SkeletonMeshBuilder.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/SpatialSort.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/TriangulateProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ValidateDataStructure.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/Version.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/VertexTriangleAdjacency.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ObjFileImporter.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ObjFileMtlImporter.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ObjFileParser.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/glTFImporter.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/glTF2Importer.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/MakeVerboseFormat.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/CalcTangentsProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/ScaleProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code/EmbedTexturesProcess.cpp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/irrXML/*",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/zlib/zutil.c",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/zlib/inffast.c",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/zlib/inflate.c",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/zlib/inftrees.c",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/zlib/adler32.c",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/zlib/crc32.c",
  }

  includedirs 
  {
      "%{ENGINE_PATH}/Code/Dependencies/assimp",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/code",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/include",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/irrXML",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/zlib",
      "%{ENGINE_PATH}/Code/Dependencies/assimp/contrib/rapidjson/include",
  }


  filter "system:windows"
    systemversion "latest"
    defines { "_CRT_SECURE_NO_WARNINGS" }

  filter "system:macosx"
    systemversion "latest"
    defines { "ASSIMP_BUILD_MACOS" }
    links { "zlib" } -- Use system-provided zlib

  filter "system:linux"
    pic "On" -- Enable PIC for shared library support
    defines { "ASSIMP_BUILD_LINUX" }
    links { "zlib", "pthread" }

  filter "configurations:Debug"
    runtime "Debug"
    symbols "on"

  filter "configurations:Release"
    runtime "Release"
    optimize "on"
