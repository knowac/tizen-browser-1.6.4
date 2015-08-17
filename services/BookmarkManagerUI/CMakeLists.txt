project(BookmarkManagerUI)

set(BookmarkManagerUI_SRCS
    BookmarkManagerUI.cpp
    AddNewFolderPopup.cpp
    )

set(BookmarkManagerUI_HEADERS
    BookmarkManagerUI.h
    AddNewFolderPopup.h
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)

add_library(${PROJECT_NAME} SHARED ${BookmarkManagerUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

#please do not add edc/ directory
set(edcFiles
    BookmarkManagerUI.edc
    AddNewFolderPopup.edc
    )

foreach(edec ${edcFiles})
    string(REPLACE ".edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
               ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
               ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)
