#!/bin/bash
set -e
usage()
{
cat << EOF 1>&2; exit 1;
Synopsis: $0 [OPTIONS]

Options default values are in parentheses.

  -a asanmode   <(OFF)|ON> address sanitizer support
  -m mode       <(Debug)|Release|RelWithDebInfo> compile mode
  -n arch       <(generic)|native> target architecture mode
  -p boolean    <(true)|false> will build in parallel (make -jn) if true.
                Necessary to be able to build with no parallelism as it
                 currently fails on some machines.
  -v version    <(val)|rev> version number is set either to the value set by
                config files or to the short git sha1
  -G Generator  <(Ninja)|Unix|MSYS|NMake|VS> which cmake generator to use.
EOF
exit 1
}

[ -z "$NIND_DIST" ] && echo "Need to set NIND_DIST" && exit 1;

asan="OFF"
arch="generic"
cmake_mode="Debug"
j="0"
version="val"
resources="build"
parallel="true"
CMAKE_GENERATOR="Ninja"

while getopts ":a:m:n:p:v:G:" o; do
    case "${o}" in
        a)
            asan=${OPTARG}
            [[ "x$asan" == "xON" || "x$asan" == "xOFF" ]] || usage
            ;;
        m)
            cmake_mode=${OPTARG}
            [[ "x$cmake_mode" == "xDebug" || "x$cmake_mode" == "xRelease" || "x$cmake_mode" == "xRelWithDebInfo" ]] || usage
            ;;
        n)
            arch=${OPTARG}
            [[ "x$arch" == "xnative" || "x$arch" == "xgeneric" ]] || usage
            ;;
        p)
            parallel=${OPTARG}
            [[ "$parallel" == "true" || "$parallel" == "false" ]] || usage
            ;;
        v)
            version=$OPTARG
            [[ "$version" == "val" ||  "$version" == "rev" ]] || usage
            ;;
        G)
          CMAKE_GENERATOR=${OPTARG}
          echo "CMAKE_GENERATOR=$CMAKE_GENERATOR"
          [[     "$CMAKE_GENERATOR" == "Ninja"  || 
                 "$CMAKE_GENERATOR" == "Unix"  ||
                 "$CMAKE_GENERATOR" == "MSYS"  ||
                 "$CMAKE_GENERATOR" == "NMake" ||
                 "$CMAKE_GENERATOR" == "VS"
          ]] || usage
          ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if type git && git rev-parse --git-dir; then
    current_branch=`git rev-parse --abbrev-ref HEAD`
    current_revision=`git rev-parse --short HEAD`
    current_timestamp=`git show -s --format=%ct HEAD`
else
    # use default values
    current_branch="default"
    current_revision="default"
    current_timestamp=1
fi
current_project=`basename $PWD`
current_project_name="`head -n1 CMakeLists.txt`"
build_prefix=build/$current_branch
source_dir=$PWD

if [[ $version = "rev" ]]; then
  release="$current_timestamp-$current_revision"
else
  release="1"
fi

if [[ $parallel == "false" ]]; then
  j="1"
fi

if [[ "$j" == "0" ]]; then
  if [[ $CMAKE_GENERATOR == "VS" ]]; then
    j=`WMIC CPU Get NumberOfCores | head -n 2 | tail -n 1 | sed -n "s/\s//gp"`
  elif [[ $CMAKE_GENERATOR == "Unix" || $CMAKE_GENERATOR == "Ninja" ]]; then
    j=`grep -c ^processor /proc/cpuinfo`
  fi
fi
if [[ "$j" == "1" ]]; then
  echo "Linear build"
else
  echo "Parallel build on $j processors"
fi

# export VERBOSE=1

if [[ $arch == "native" ]]; then
  WITH_ARCH="ON"
else
  WITH_ARCH="OFF"
fi

if [[ $CMAKE_GENERATOR == "Unix" ]]; then
  make_cmd="make -j$j"
  make_test="make test"
  make_install="make install"
  generator="Unix Makefiles"
elif [[ $CMAKE_GENERATOR == "Ninja" ]]; then
  make_cmd="ninja -j $j"
  make_test=""
  make_install="ninja install"
  generator="Ninja"
elif [[ $CMAKE_GENERATOR == "MSYS" ]]; then
  make_cmd="make -j$j"
  make_test="make test"
  make_install="make install"
  generator="MSYS Makefiles"
elif [[ $CMAKE_GENERATOR == "NMake" ]]; then
  make_cmd="nmake && exit 0"
  make_test="nmake test"
  make_install="nmake install"
  generator="NMake Makefiles"
elif [[ $CMAKE_GENERATOR == "VS" ]]; then
  make_cmd="""
  pwd &&
  cmake --build . --config $cmake_mode
  """
  make_test=""
  make_install=""
  generator="Visual Studio 14 2015 Win64"
else
  make_cmd="make -j$j"
fi


echo "version='$version' release='$release'"
mkdir -p $build_prefix/$mode/$current_project
pushd $build_prefix/$mode/$current_project

if [ $CMAKE_GENERATOR == "Unix" ] && [ "x$cmake_mode" == "xRelease" ] ;
then
  if compgen -G "*/src/*-build/*.rpm" > /dev/null; then
    rm -f */src/*-build/*.rpm
  fi
  if compgen -G "*/src/*-build/*.deb" > /dev/null; then
    rm -f */src/*-build/*.deb
  fi
fi

echo "Launching cmake from $PWD"
cmake -G "$generator"  \
  -DCMAKE_BUILD_TYPE:STRING=$cmake_mode \
  -DCMAKE_INSTALL_PREFIX:PATH=$NIND_DIST \
  -DWITH_ARCH=$WITH_ARCH \
  -DWITH_ASAN=$asan \
  $source_dir

echo "Running command:"
eval $make_cmd
result=$?

if [ "$result" != "0" ]; then
  popd
  exit $result
fi

eval $make_test && eval $make_install
result=$?

popd

exit $result
