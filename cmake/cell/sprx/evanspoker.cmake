add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PS3_PRX_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/evanspoker.prx -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/evanspoker.prx.stripped
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PS3_MAKE_FSELF} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/evanspoker.prx.stripped ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/evanspoker.sprx
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/evanspoker.sprx"
	"E:/emu/rpcs3/dev_hdd0/game/BCET70002/USRDIR")