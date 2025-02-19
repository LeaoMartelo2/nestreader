#!/bin/bash

set -xe

gcc nest_reader.c -Wall -Wextra -O3 -o nest_reader -s
