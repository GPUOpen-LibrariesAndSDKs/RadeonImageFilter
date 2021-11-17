add_custom_target(InstallHeaders)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    DESTINATION .
    FILES_MATCHING PATTERN "*.h"
)
