#!/bin/sh


THIS=$(dirname $(readlink -f $0))


${THIS}/bin/nom ${THIS}/prelude.mk -DNOM_ARG1="$1"


