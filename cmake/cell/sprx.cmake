add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PS3_PRX_STRIP} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cwml.prx -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cwml.prx.stripped
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PS3_MAKE_FSELF} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cwml.prx.stripped ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cwml.sprx
)

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND wsl --exec rm -f ../../bin/cwml.pahole.h
# )

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND wsl --exec pahole --compile -a -A -d -I ../../bin/cwml.prx > ../../bin/cwml.pahole.h
# )


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cwml.sprx"
	${GAME_INSTALL_PATH})