set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)   # <-- Use /MD or /MDd
set(VCPKG_LIBRARY_LINKAGE static) # <-- Static .lib libraries
set(VCPKG_SYSTEM_NAME windows)

# Optional, useful for reproducibility:
set(VCPKG_PLATFORM_TOOLSET v143)
