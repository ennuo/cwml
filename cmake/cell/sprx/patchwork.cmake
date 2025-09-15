add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PS3_PRX_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/patchwork.prx -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/patchwork.prx.stripped
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PS3_MAKE_FSELF} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/patchwork.prx.stripped ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/patchwork.sprx
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/patchwork.sprx"
	"${GAME_INSTALL_PATH}/cwml/plugins")