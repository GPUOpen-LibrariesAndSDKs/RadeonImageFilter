
#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define RIF_VERSION_MAJOR      1
#define RIF_VERSION_MINOR      5
#define RIF_VERSION_REVISION   1
#define RIF_COMMIT_INFO        0xefd92438



#ifndef RIF_COMMIT_INFO
#define RIF_VERSION_BUILD               0
#else
#define RIF_VERSION_BUILD               RIF_COMMIT_INFO
#endif


#define VER_FILE_DESCRIPTION_STR    "Radeon Image Filters"
#define VER_FILE_VERSION            RIF_VERSION_MAJOR, RIF_VERSION_MINOR, RIF_VERSION_REVISION, RIF_VERSION_BUILD
#define VER_FILE_VERSION_STR        STRINGIZE(RIF_VERSION_MAJOR)        \
                                    "." STRINGIZE(RIF_VERSION_MINOR)    \
                                    "." STRINGIZE(RIF_VERSION_REVISION) \
                                    "." STRINGIZE(RIF_VERSION_BUILD)    \

#define VER_PRODUCTNAME_STR         "RadeonImageFilters64"
#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_ORIGINAL_FILENAME_STR   VER_PRODUCTNAME_STR ".dll"
#define VER_INTERNAL_NAME_STR       VER_ORIGINAL_FILENAME_STR
#define VER_COPYRIGHT_STR           "Copyright (C) 2019 by Advanced Micro Devices"

#ifdef _DEBUG
  #define VER_VER_DEBUG             VS_FF_DEBUG
#else
  #define VER_VER_DEBUG             0
#endif

#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               VER_VER_DEBUG
#define VER_FILETYPE                VFT_DLL
