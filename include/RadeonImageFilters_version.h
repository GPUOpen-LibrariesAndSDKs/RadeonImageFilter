
#define RIF_VERSION_MAJOR      1
#define RIF_VERSION_MINOR      5
#define RIF_VERSION_REVISION   3
#define RIF_COMMIT_INFO        0x47b72111



#ifndef RIF_COMMIT_INFO
#define RIF_VERSION_BUILD               0
#else
#define RIF_VERSION_BUILD               RIF_COMMIT_INFO
#endif
