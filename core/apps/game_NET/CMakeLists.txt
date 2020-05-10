# Compilcation rules for the game_NET application
# 2020 - Lucca Rawlyk

set(game_SRCS
    main.cpp
    net_controller.cpp
)

add_executable(game_NET ${game_SRCS})

set_target_properties(game_NET PROPERTIES OUTPUT_NAME "game_NET")

target_link_libraries(game_NET
                      apg
                      ${QT_LIBRARIES}
                     )
