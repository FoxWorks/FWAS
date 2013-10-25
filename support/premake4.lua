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
-- FoxWorks Model Editor
--------------------------------------------------------------------------------
project "fwas_core"
--   uuid "C84AD4D2-2D63-1842-871E-30B7C71BEA58"
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
--     "../external/evds/addons/evds_train_wheels.c",
--     "../external/evds/addons/evds_train_wheels.h",
--     "../external/evds/addons/evds_nrlmsise-00.c",
--     "../external/evds/addons/evds_nrlmsise-00.h",
--     "../external/nrlmsise-00/nrlmsise-00.c",
--     "../external/nrlmsise-00/nrlmsise-00_data.c" }
   }
   links { "evds","simc" } --"rdrs","ivss"
