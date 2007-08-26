#! /bin/bash 
case "$1" in
	-c)
		CC=gcc
		shift 1
	;;
	-cxx)
		CC=g++
		shift 1
	;;
	*)
		CC=gcc
	;;
esac
DIR="$1"
shift 1
case "$DIR" in 
"" | ".")
	$CC -M -MP "$@" | sed -e 's@^\(.*\)\.o:@\1.d \1.lo:@'
	;;
*)
	$CC -M -MP "$@" | sed -e "s@^\(.*\)\.o:@$DIR/\1.d $DIR/\1.lo:@"
	;;
esac

