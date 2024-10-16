set(extra_src Mysql.c Mysql_dump.c Mysql_speed.c)

add_library(extra OBJECT ${extra_src})

install(TARGETS extra RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})