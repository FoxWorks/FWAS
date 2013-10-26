SIMC_STANDALONE = false
EVDS_STANDALONE = false
RDRS_STANDALONE = false
IVSS_STANDALONE = false
solution "fwas"
   debugdir "../debug"
   dofile("./../external/simc/support/premake4_common.lua")
   dofile("./../external/simc/support/premake4.lua")
   dofile("./../external/evds/support/premake4.lua")
--   dofile("./../external/rdrs/support/premake4.lua")
--   dofile("./../external/ivss/support/premake4.lua")
   
-- Create working directory
if not os.isdir("../debug") then os.mkdir("../debug") end



--------------------------------------------------------------------------------
-- FoxWorks Aerospace Editor Core
--------------------------------------------------------------------------------
project "fwas_core"
   uuid "F86FC6CB-09A3-E34C-B355-ADEAE5EF050A"
   kind "StaticLib"
   language "C"

   includedirs {
     "../external/simc/include",
     "../external/evds/include",
     "../external/rdrs/include",
     "../external/evds/addons",
   }
   files {
     "../source/core/**.c",
     "../source/core/**.h",

--     "../external/evds/addons/evds_antenna.c",
--     "../external/evds/addons/evds_antenna.h",
     "../external/evds/addons/evds_train_wheels.c",
     "../external/evds/addons/evds_train_wheels.h",
--     "../external/evds/addons/evds_nrlmsise-00.c",
--     "../external/evds/addons/evds_nrlmsise-00.h",
--     "../external/nrlmsise-00/nrlmsise-00.c",
--     "../external/nrlmsise-00/nrlmsise-00_data.c" }
   }



--------------------------------------------------------------------------------
-- X-Plane client
--------------------------------------------------------------------------------
project "fwas_x-plane"
   uuid "F79599D0-3591-C346-A926-822445264228"
   kind "SharedLib"
   language "C"
   
   -- Select proper X-Plane plugin name
   targetdir "../debug"
   targetextension ".xpl"
   implibextension ".imp"
   targetsuffix ""
   configuration { "windows" }
      targetname "win"
      defines { "IBM" }
   configuration { "linux" }
      targetname "lin"
      defines { "LIN" }
   configuration { "macosx" }
      targetname "mac"
      defines { "APL" }
   configuration {}

   -- Include the source code
   includedirs {
     "../external/simc/include",
     "../external/evds/include",
     "../external/rdrs/include",
     "../external/evds/addons",
     "../source/core",
   }
   files {
     "../source/client/x-plane/xp_*.c",
     "../source/client/x-plane/xp_*.h",
   }
   links { "fwas_core","evds","simc" } --"rdrs","ivss"
   
   -- OpenGL required under windows
   configuration { "windows" }
      links { "opengl32" }
   
   -- Include X-Plane SDK
   configuration { "x32" }
      includedirs {
        "../external/xpsdk201/CHeaders/Widgets",
        "../external/xpsdk201/CHeaders/XPLM",
      }
      libdirs {
        "../external/xpsdk201/Libraries/Win",
      }
      links { "XPLM", "XPWidgets" }
      
   configuration { "x64" }
      includedirs {
        "../external/xpsdk212/CHeaders/Widgets",
        "../external/xpsdk212/CHeaders/XPLM",
      }
      libdirs {
        "../external/xpsdk212/Libraries/Win",
      }
      links { "XPLM_64", "XPWidgets_64" }
