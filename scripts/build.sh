#! /usr/bin/env bash

set -o errexit
set -o nounset
set -o pipefail

gcc -I../include/ -o ../bin/msh ../src/*.c
alias msh=../bin/msh
