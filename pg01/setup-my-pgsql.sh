#!/bin/sh

set -eu

Run() {
    echo "$(date --rfc-3339=ns): " "$@"
    "$@"
}

Download() {
    dlpath=https://get.enterprisedb.com/postgresql/postgresql-9.6.2-3-linux-x64-binaries.tar.gz

    if ! [ -d pgsql ]; then
        if ! [ -f ${dlpath##*/} ]; then
            Run wget $dlpath
        fi
        Run tar -xf ${dlpath##*/}
    fi
}

InitDB() {
    if ! [ -d pgdata ]; then
        mkdir pgdata
        Run pgsql/bin/initdb -D pgdata
    fi
}

SetConfigSingle() {
    var=$1
    val=$2
    sed -re "s/^(([[:blank:]]*#)?($var[[:blank:]]*=.*))/#\3\n$var = $val/" -i pgdata/postgresql.conf
}

UpdateConfig() {
    SetConfigSingle port 15432
    SetConfigSingle unix_socket_directories .
    SetConfigSingle tcp_keepalives_idle 60
    SetConfigSingle log_line_prefix "'%m '"
}

PrintUsage() {
    echo "${0##*/} [--start|--stop]"
    exit "${1:-1}"
}

################################################################################

cd "${0%/*}"

cmd="-l pg.log start"
while [ 0 -ne $# ]; do
    case $1 in
        --start)
            ;;
        --stop)
            cmd=stop
            ;;
        --test-update-config)
            UpdateConfig
            exit 0
            ;;
        --help|-h)
            PrintUsage
            ;;
        *)
            extra=""
            if [ 1 -lt $# ]; then
                extra="in $*"
            fi
            echo "Unknown command $1 $extra"
            PrintUsage 1
            ;;
    esac
    shift
done

Download
InitDB
UpdateConfig

Run pgsql/bin/pg_ctl -D pgdata $cmd
