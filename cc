#!/bin/bash
set -ue

GOOS=${GOOS:-linux}
GOARCH=${GOARCH:-amd64}
CGO_ENABLED=${CGO_ENABLED:-1}

# If we're not `go build` or `go install`, abort
if [ "$0" = "/bin/go" ] \
	&& [ "$1" != "build" ] \
	&& [ "$1" != "install" ]; then
	exec /usr/local/go/bin/go "$@"
fi

if [ "$CGO_ENABLED" = "1" ]; then
	LINKMODEFLAGS="-linkmode external -extldflags \"-static\""
fi

case "$GOOS-$GOARCH-$(basename "$0")" in
## Linux
linux-arm-cc)
	export CC=/usr/local/musl-arm/bin/musl-gcc
	exec "$CC" "$@"
;;
linux-arm-c++)
	export CXX=arm-linux-gnueabi-g++-8
	exec "$CXX" "$@"
;;
linux-arm64-cc)
	export CC=/usr/local/musl-aarch64/bin/musl-gcc
	exec "$CC" "$@"
;;
linux-arm64-c++)
	export CXX=aarch64-linux-gnu-g++-8
	exec "$CXX" "$@"
;;
linux-386-cc)
	export CC=gcc
	exec "$CC" "$@"
;;
linux-386-c++)
	export CC=g++
	exec "$CC" "$@"
;;
linux-amd64-cc)
	export CC=/usr/local/musl-x86_64/bin/musl-gcc
	exec "$CC" "$@"
;;
linux-amd64-c++)
	export CXX=g++
	exec "$CXX" "$@"
;;
linux-*-go)
	LDFLAGS="${LDFLAGS:-} ${LINKMODEFLAGS:-}"
;;
## Windows
windows-amd64-cc)
	export CC=x86_64-w64-mingw32-gcc
	exec "${CC}" "$@"
;;
windows-386-cc)
	export CC=i686-w64-mingw32-gcc
	exec "${CC}" "$@"
;;
windows-*-go)
;;
## Darwin
darwin-*-go)
;;
## Freebsd
freebsd-386-cc)
	export LD_LIBRARY_PATH=/freebsd/lib:/freebsd/lib32
	export CC=i386-pc-freebsd12-gcc
	exec "${CC}" "$@"
;;
freebsd-386-c++)
	export LD_LIBRARY_PATH=/freebsd/lib:/freebsd/lib32
	export CXX=i386-pc-freebsd12-gpp
	exec "${CXX}" "$@"
;;
freebsd-amd64-cc)
	export LD_LIBRARY_PATH=/freebsd/lib:/freebsd/lib32
	export CC=x86_64-pc-freebsd12-gcc
	exec "${CC}" "$@"
;;
freebsd-amd64-c++)
	export LD_LIBRARY_PATH=/freebsd/lib:/freebsd/lib32
	# FINDME should this be CPP?
	export CXX=x86_64-pc-freebsd12-gpp
	exec "${CXX}" "$@"
;;
freebsd-*-go)
	LDFLAGS="${LDFLAGS:-} ${LINKMODEFLAGS:-}"
;;
js-wasm-go)
	exec /usr/local/go/bin/go "$@"
;;
*)
	echo "There is not compiler for GOOS=${GOOS} GOARCH=${GOARCH} in the cc file!" >&2
	exit 1
;;
esac

# $0 must be go, or we should have gotten this far
# The first arg should be 'build' or 'install', or we wouldn't have gotten this far
SUBCOMMAND=$1

shift
declare -a adjusted_args=()
found_ldflags=0

# loop over all arguments searching for -ldflags
while (( $# )); do
	# there are two cases -ldflags XYZ and -ldflags=XYZ
	# when we find it, we'll inject LDFLAGS environment variable
	if [ "$1" = "-ldflags" ]; then
		shift
		adjusted_args[${#adjusted_args[@]}]="-ldflags"
		adjusted_args[${#adjusted_args[@]}]="$LDFLAGS $1"
		found_ldflags=1
	elif [ "${1:0:9}" = "-ldflags=" ]; then
		adjusted_args[${#adjusted_args[@]}]="-ldflags"
		adjusted_args[${#adjusted_args[@]}]="$LDFLAGS ${1:9}"
		found_ldflags=1
	else
		adjusted_args[${#adjusted_args[@]}]=$1
	fi
	shift
done

if [ "$found_ldflags" = 0 ]; then
	# we didn't find -ldflags, hence we'll prepend environment LDFLAGS instead.
	echo /usr/local/go/bin/go $SUBCOMMAND -ldflags "${LDFLAGS:-}" "${adjusted_args[@]}"
	exec /usr/local/go/bin/go $SUBCOMMAND -ldflags "${LDFLAGS:-}" "${adjusted_args[@]}"
else
	# otherwise, LDFLAGS and -ldflags have been merged already
	echo /usr/local/go/bin/go $SUBCOMMAND "${adjusted_args[@]}"
	exec /usr/local/go/bin/go $SUBCOMMAND "${adjusted_args[@]}"
fi
