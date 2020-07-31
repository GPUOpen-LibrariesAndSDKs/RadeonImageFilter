
#define RIF_VERSION_MAJOR      1
#define RIF_VERSION_MINOR      5
#define RIF_VERSION_REVISION   4
#define RIF_COMMIT_INFO        0x05d31e03



#ifndef RIF_COMMIT_INFO
#define RIF_VERSION_BUILD               0
#else
#define RIF_VERSION_BUILD               RIF_COMMIT_INFO
#endif
