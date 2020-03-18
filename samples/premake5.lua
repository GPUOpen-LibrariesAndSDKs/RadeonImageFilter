function fileExists(name)
    local f=io.open(name,"r")
    if f~=nil then io.close(f) return true else return false end
end

newoption {
    trigger     = "metal",
    description = "Enable Metal backend (MacOs only)"
}

solution "Samples"

configurations { "Debug", "Release" }
language "C++"
flags { "NoMinimalRebuild", "EnableSSE", "EnableSSE2" }

if( _OPTIONS["metal"]) then
    defines{ "RIF_USE_METAL=1" }
    print ">> Building RadeonProImageProcessing to use metal backend";
end

-- define common includes
includedirs {".","ImageTools", "../include" }

platforms {"x64"}

--make configuration specific definitions
configuration "Debug"
    defines { "_DEBUG" }
    flags { "Symbols" }
    targetsuffix "D"
configuration "Release"
    defines { "NDEBUG" }
    flags { "Optimize" }

configuration {"x64", "Debug"}
    libdirs {"../bin/debug/x64"}
    links {"RadeonImageFilters64D"}
configuration {"x64", "Release"}
    libdirs {"../bin/release/x64"}
    links {"RadeonImageFilters64"}
configuration {}

-- perform OS specific initializations
local targetName;
if os.is("macosx") then
    targetName = "osx"
end

if os.is("windows") then
    targetName = "win"
    defines{ "WIN32" }
    buildoptions { "/MP"  } --multiprocessor build
    defines {"_CRT_SECURE_NO_WARNINGS"}
    systemversion "10.0.17134.0"
elseif os.is("linux") then
    buildoptions {"-fvisibility=hidden -std=c++14 -fPIC"}
elseif os.is("macosx") then
    buildoptions {"-fvisibility=hidden -std=c++11 -fPIC"}
end

if _OPTIONS["use_opencl"] then
    defines{"USE_OPENCL=1"}
end

if _OPTIONS["enable_fp16"] then
    defines{"FP16_ENABLED=1"}
end

if os.is("windows") then
    links { "opengl32" }
end
if os.is("linux") then
    links { "RadeonImageFilters64", "GL", "pthread", "dl" }
    -- linkoptions {"-Wl,-rpath=$(TARGETDIR)"}
end
if os.is("macosx") then
    linkoptions{ "-framework OpenCL", "-framework Cocoa", "-framework OpenGL", "-framework IOKit" }
end


targetdir "./bin"

configuration "Debug"

if os.is("windows") then
    if _OPTIONS["rif_dev"] then
        prebuildcommands { "{COPY} ../bin/debug/x64/RadeonImageFilters64D.dll %{cfg.targetdir}" }
    else
        prebuildcommands { "{COPY} ../bin/release/x64/RadeonImageFilters64.dll %{cfg.targetdir}" }
    end
else
    prebuildcommands { "{COPY} ../bin/debug/x64/libRadeonImageFilters64D.so %{cfg.targetdir}" }
end

configuration "Release"

if os.is("windows") then
    prebuildcommands { "{COPY} ../bin/release/x64/RadeonImageFilters64.dll %{cfg.targetdir}" }
else
    prebuildcommands { "{COPY} ../bin/release/x64/libRadeonImageFilters64.so %{cfg.targetdir}" }
end

configuration {}

if os.is("linux") then
    dofile ("../OpenCLSearch.lua" )
    links {"GL", "pthread", "dl", "OpenCL"}
end

if fileExists("./BilateralDenoise/BilateralDenoise.lua") then
    dofile("./BilateralDenoise/BilateralDenoise.lua")
end

if fileExists("./MLAA/MLAA.lua") then
    dofile("./MLAA/MLAA.lua")
end

if fileExists("./GaussianBlur/GaussianBlur.lua") then
    dofile("./GaussianBlur/GaussianBlur.lua")
end

if fileExists("./ToneMapping/ToneMapping.lua") then
    dofile("./ToneMapping/ToneMapping.lua")
end

if fileExists("./EdgeDetection/EdgeDetection.lua") then
    dofile("./EdgeDetection/EdgeDetection.lua")
end

if fileExists("./MedianDenoise/MedianDenoise.lua") then
    dofile("./MedianDenoise/MedianDenoise.lua")
end

if fileExists("./UserDefined/UserDefined.lua") then
    dofile("./UserDefined/UserDefined.lua")
end

if fileExists("./Denoisers/Denoisers.lua") then
    dofile("./Denoisers/Denoisers.lua")
end

if fileExists("./PhotoToneMapping/PhotoToneMapping.lua") then
    dofile("./PhotoToneMapping/PhotoToneMapping.lua")
end

if fileExists("./Bloom/Bloom.lua") then
    dofile("./Bloom/Bloom.lua")
end

if os.is("windows") or os.is("linux") then
    if fileExists("./AIDenoiser/AIDenoiser.lua") then
        dofile("./AIDenoiser/AIDenoiser.lua")

        if os.is("windows") then
            if _OPTIONS["directx12"] then
                configuration "Debug"
                    postbuildcommands { "{COPY} ../3rdparty/lib/windows/RadeonML-1.5.4/Debug/RadeonML-DirectML-d.dll %{cfg.targetdir}" }
                configuration "Release"
                    postbuildcommands { "{COPY} ../3rdparty/lib/windows/RadeonML-1.5.4/Release/RadeonML-DirectML.dll %{cfg.targetdir}" }
            end

            configuration "Debug"
                postbuildcommands { "{COPY} ../3rdparty/lib/windows/RadeonML-1.5.4/Debug/RadeonML-MIOpen-d.dll %{cfg.targetdir}" }
            configuration "Release"
                postbuildcommands { "{COPY} ../3rdparty/lib/windows/RadeonML-1.5.4/Release/RadeonML-MIOpen.dll %{cfg.targetdir}" }
        end
        postbuildcommands { "{COPY} ../models ./models" }
    end
end

if fileExists("./OpenImageDenoiser/OpenImageDenoiser.lua") then
    if os.is("windows") then
        configuration "Debug"
            postbuildcommands { "{COPY} ../bin/debug/x64/RadeonImageFilters64D.dll %{cfg.targetdir}" }
        configuration "Release"
            postbuildcommands { "{COPY} ../bin/release/x64/RadeonImageFilters64.dll %{cfg.targetdir}" }
        configuration {}
        postbuildcommands { "{COPY} ../3rdparty/lib/windows/oidn-0.9.0/*.dll %{cfg.targetdir}" }
    end

    dofile("./OpenImageDenoiser/OpenImageDenoiser.lua")
end
