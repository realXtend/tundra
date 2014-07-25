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

array_contains ()
{
  local e
  for e in "${@:2}"; do [[ "$e" == "$1" ]] && return 0; done
  return 1
}
