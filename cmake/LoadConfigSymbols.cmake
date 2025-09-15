if (NOT DEFINED GAME_CONFIG_PATH)
    set(GAME_CONFIG_PATH "${CMAKE_SOURCE_DIR}/config/${GAME_CONFIG}")
    set(GAME_PORT_PATH "${GAME_CONFIG_PATH}/ports.cnf")
    set(GAME_SYMBOL_PATH "${GAME_CONFIG_PATH}/symbols.yml")
    set(GAME_DEFINE_PATH "${GAME_CONFIG_PATH}/defines.txt")
    set(GAME_TOC_PATH "${GAME_CONFIG_PATH}/toc.yml")
    file(READ "${GAME_CONFIG_PATH}/install.txt" GAME_INSTALL_PATH)
endif()

file(STRINGS "${GAME_SYMBOL_PATH}" SYMBOLS_FILE)
while(SYMBOLS_FILE)
    # Some regex like (^[^\s]*):(\s*.*)[^\s] maybe?
    list(POP_FRONT SYMBOLS_FILE LINE)

    if (LINE MATCHES "#" OR LINE STREQUAL "")
        continue()
    endif()


    string(REPLACE ": " ";" LIST ${LINE})
    list(GET LIST 0 SYMBOL_NAME)
    list(GET LIST 1 SYMBOL_VALUE)

    add_link_options("-Wl,--defsym,${SYMBOL_NAME}=${SYMBOL_VALUE}")
endwhile()

file(STRINGS "${GAME_TOC_PATH}" TOC_FILE)
while(TOC_FILE)
    # Some regex like (^[^\s]*):(\s*.*)[^\s] maybe?
    list(POP_FRONT TOC_FILE LINE)
    string(REPLACE ": " ";" LIST ${LINE})
    list(GET LIST 0 SYMBOL_NAME)
    list(GET LIST 1 SYMBOL_VALUE)

    add_compile_definitions(${SYMBOL_NAME}=${SYMBOL_VALUE})
endwhile()

file(STRINGS "${GAME_DEFINE_PATH}" DEFINES_FILE)
while (DEFINES_FILE)
    list(POP_FRONT DEFINES_FILE DEFINE_NAME)
    add_compile_definitions(${DEFINE_NAME}=1)
endwhile()

file(STRINGS "${GAME_PORT_PATH}" PORTS_FILE)
while(PORTS_FILE)
    list(POP_FRONT PORTS_FILE LINE)


    if (LINE MATCHES "#" OR LINE STREQUAL "")
        continue()
    endif()

    string(REPLACE "." "_" LINE ${LINE})
    string(REPLACE " " ";" LINE ${LINE})

    list(GET LINE 0 PORT_TOC)
    list(GET LINE 1 PORT_ADDRESS)
    list(GET LINE 2 PORT_NAME)

    add_compile_definitions(
        PORT_${PORT_NAME}_TOC=${PORT_TOC}
        PORT_${PORT_NAME}_ADDRESS=${PORT_ADDRESS}
    )
endwhile()