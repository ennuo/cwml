if (_PS3_SDK_)
    return()
endif()
set(_PS3_SDK 1)

if (EXISTS "$ENV{SCE_PS3_ROOT}" AND IS_DIRECTORY "$ENV{SCE_PS3_ROOT}")
	string(REGEX REPLACE "\\\\" "/" PS3_SDK $ENV{SCE_PS3_ROOT})
	string(REGEX REPLACE "//" "/" PS3_SDK ${PS3_SDK})
endif()

# Make sure we actually have the PS3 SDK on the system
if (NOT PS3_SDK)
	message(FATAL_ERROR, "PS3 SCE SDK is required in order to build the project for the PS3 platform.")
endif()

# set(REQUIRED_PS3_VERSION "270.001")
# file(READ "${PS3_SDK}/version-SDK" SCE_PS3_VERSION)
# if (NOT "${SCE_PS3_VERSION}" MATCHES "${REQUIRED_PS3_VERSION}")
# 	message(WARNING "Expected PS3 SCE version: ${REQUIRED_PS3_VERSION}")
# endif()
