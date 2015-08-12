#!/bin/sh
find .|egrep "(cpp|hpp)"|xargs astyle -L -A14 -N --indent=spaces=2