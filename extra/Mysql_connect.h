/*
 * Mysql_connect.h: connect to MySQL / MariaDB for the -mysql* functions.
 *
 * Passing the password on the command line shows it in the process list and in
 * the argument list printed by wgrib2 on a fatal error. If the password
 * argument is "-", the credentials are read from the option file group
 * [wgrib2] of ~/.my.cnf (or /etc/my.cnf) instead:
 *
 *   [wgrib2]
 *   user=...        (used if the user argument is also "-")
 *   password=...
 *   socket=...      (optional, used for host "localhost")
 *
 * With any other password the behaviour is unchanged.
 */
#ifndef WGRIB2_MYSQL_CONNECT_H
#define WGRIB2_MYSQL_CONNECT_H

#ifdef USE_MYSQL

#include <string.h>
#include <mysql.h>

static MYSQL *wgrib2_mysql_connect(MYSQL *conn, const char *server, const char *user,
        const char *password, const char *database) {

    const char *u = user;
    const char *p = password;

    if (password != NULL && strcmp(password, "-") == 0) {
        mysql_options(conn, MYSQL_READ_DEFAULT_GROUP, "wgrib2");
        p = NULL;
        if (user != NULL && strcmp(user, "-") == 0) u = NULL;
    }
    return mysql_real_connect(conn, server, u, p, database, 0, NULL, 0);
}

#endif
#endif
