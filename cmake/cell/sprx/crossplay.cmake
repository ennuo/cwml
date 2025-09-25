add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PS3_PRX_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crossplay.prx -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crossplay.prx.stripped
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PS3_MAKE_FSELF} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crossplay.prx.stripped ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crossplay.sprx
)

if (DEFINED GAME_INSTALL_PATH)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crossplay.sprx"
		"${GAME_INSTALL_PATH}/cwml/plugins")
endif()