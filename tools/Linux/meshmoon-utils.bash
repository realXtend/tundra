#!/bin/bash

print_title()
{
    echo
    echo $1
    local len=$((${#1}+1))
    seq -s- ${len} | tr -d '[:digit:]'
}

dir_exists()
{
    test -d "$1"
    return $?
}

file_exists()
{
    test -f "$1"
    return $?
}

rm_r_quiet()
{
    echo "    Removing $1"
    rm -rf $1
}

rm_quiet()
{
    echo "    Removing $1"
    rm -f $1
}

array_contains()
{
  local e
  for e in "${@:2}"; do [[ "$e" == "$1" ]] && return 0; done
  return 1
}

is_built()
{
    echo
    echo $1

    # Directory exists?
    test -d "$DEPS_SRC/$1"
    if [ $? -ne 0 ] ; then
        echo "Error: Sources not found from $DEPS_SRC/$1"
        exit 1
    fi

    # Build done marker file exists?
    test -f "$DEPS_SRC/$1/meshmoon-builder.json"
    result=$?
    if [ $result -eq 0 ] ; then
        echo "    Build      OK"
        cd $DEPS
    else
        echo "    Building, please wait..."
        cd $DEPS_SRC/$1
    fi
    return $result
}

mark_built()
{
    touch $DEPS_SRC/$1/meshmoon-builder.json
}
