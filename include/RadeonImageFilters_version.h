
#define RIF_VERSION_MAJOR      1
#define RIF_VERSION_MINOR      6
#define RIF_VERSION_REVISION   1
#define RIF_COMMIT_INFO        0xe4886697



#ifndef RIF_COMMIT_INFO
#define RIF_VERSION_BUILD               0
#else
#define RIF_VERSION_BUILD               RIF_COMMIT_INFO
#endif


#define RIF_MAKE_VERSION(major, minor, revision, build) \
    (((uint64_t)(major) << 56) | ((uint64_t)(minor) << 48) | ((uint64_t)(revision) << 32) | build)

#define RIF_API_VERSION RIF_MAKE_VERSION(RIF_VERSION_MAJOR, RIF_VERSION_MINOR, RIF_VERSION_REVISION, RIF_VERSION_BUILD)

#define STRINGIFY2(s) #s
#define STRINGIFY(s) STRINGIFY2(s)

#define RIF_API_VERSION_STRING STRINGIFY(RIF_VERSION_MAJOR) "." STRINGIFY(RIF_VERSION_MINOR) "." STRINGIFY(RIF_VERSION_REVISION) "." STRINGIFY(RIF_VERSION_BUILD)
