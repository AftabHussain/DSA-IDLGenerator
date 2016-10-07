BUILD_DSA=1
BUILD_LLVM=0

DSA_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )"
ROOT="$( cd "${DSA_DIR}" && cd .. && pwd )"
LOCKPWN_DIR="${ROOT}/lockpwn"
LLVM_DIR="${ROOT}/llvm"

#source ${DSA_DIR}/bin/versions

INSTALL_PREFIX=
CONFIGURE_INSTALL_PREFIX=
CMAKE_INSTALL_PREFIX=

#SMACKENV=${ROOT}/smack.environment
WGET="wget --no-verbose --method=GET"
DEPENDENCIES="git cmake python-yaml python-psutil unzip wget"

function get-platform-trim {
  local s=$(echo "$1" | tr -d '[ \t]' | tr 'A-Z' 'a-z')
  echo $s
}

# ================================================================
# Get the platform root name.
# ================================================================
function get-platform-root {
  if which uname >/dev/null 2>&1 ; then
    if uname -o >/dev/null 2>&1 ; then
      # Linux distro
      uname -o | tr 'A-Z' 'a-z'
    elif uname -s >/dev/null 2>&1 ; then
      # Solaris variant
      uname -s | tr 'A-Z' 'a-z'
    else
      echo "unknown"
    fi
  else
    echo "unknown"
  fi
}

# ================================================================
# Get the platform identifier.
# ================================================================
function get-platform {
  plat=$(get-platform-root)
  case "$plat" in
    "gnu/linux")
      d=$(get-platform-trim "$(lsb_release -i)" | awk -F: '{print $2;}')
      r=$(get-platform-trim "$(lsb_release -r)" | awk -F: '{print $2;}')
      m=$(get-platform-trim "$(uname -m)")
      if [[ "$d" == "redhatenterprise"* ]] ; then
        # Need a little help for Red Hat because
        # they don't make the minor version obvious.
        d="rhel_${d:16}"  # keep the tail (e.g., es or client)
        x=$(get-platform-trim "$(lsb_release -c)" | \
          awk -F: '{print $2;}' | \
          sed -e 's/[^0-9]//g')
        r="$r.$x"
      fi
      echo "linux-$d-$r-$m"
      ;;
    "cygwin")
      x=$(get-platform-trim "$(uname)")
      echo "linux-$x"
      ;;
    "sunos")
      d=$(get-platform-trim "$(uname -v)")
      r=$(get-platform-trim "$(uname -r)")
      m=$(get-platform-trim "$(uname -m)")
      echo "sunos-$d-$r-$m"
      ;;
    "unknown")
      echo "unk-unk-unk-unk"
      ;;
    *)
      echo "$plat-unk-unk-unk"
      ;;
  esac
}

function puts {
  echo -e "\033[35m*** DSA BUILD: ${1} ***\033[0m"
}

################################################################################
#
# END HELPER FUNCTIONS
#
################################################################################

# Exit on error
set -e

distro=$(get-platform)
puts "Detected distribution: $distro"

# Set platform-dependent flags
case "$distro" in
linux-opensuse*)
  DEPENDENCIES+=" llvm-clang llvm-devel gcc-c++ make"
  DEPENDENCIES+=" ncurses-devel zlib-devel"
  ;;

linux-ubuntu-14*)
  DEPENDENCIES+=" clang-3.6 llvm-3.6 libz-dev libedit-dev"
  ;;

linux-ubuntu-12*)
  DEPENDENCIES+=" g++-4.8 autoconf automake bison flex libtool gettext gdb"
  DEPENDENCIES+=" libglib2.0-dev libfontconfig1-dev libfreetype6-dev libxrender-dev"
  DEPENDENCIES+=" libtiff-dev libjpeg-dev libgif-dev libpng-dev libcairo2-dev"
  BUILD_DSA=1
  INSTALL_PREFIX="/usr/local"
  CONFIGURE_INSTALL_PREFIX="--prefix=${INSTALL_PREFIX}"
  CMAKE_INSTALL_PREFIX="-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}"
  ;;

linux-cygwin*)
  BUILD_DSA=1
  ;;

*)
  puts "Distribution ${distro} not supported. Manual installation required."
  exit 1
  ;;
esac

# Parse command line options
while [[ $# > 0 ]]
do
  case "$1" in
  --prefix)
    puts "Using install prefix $2"
    INSTALL_PREFIX="${2%/}"
    CONFIGURE_INSTALL_PREFIX="--prefix=$2"
    CMAKE_INSTALL_PREFIX="-DCMAKE_INSTALL_PREFIX=$2"
    echo export PATH=${INSTALL_PREFIX}/bin:$PATH
    shift
    shift
    ;;

  *)
    puts "Unknown option: $1"
    exit 1
    ;;
  esac
done

if [ ${INSTALL_DEPENDENCIES} -eq 1 ]
then
  puts "Installing required packages"

  case "$distro" in
  linux-opensuse*)
    sudo zypper --non-interactive install ${DEPENDENCIES}
    ;;

  linux-ubuntu-14*)
    # Adding LLVM repository
    sudo add-apt-repository "deb http://llvm-apt.ecranbleu.org/apt/trusty/ llvm-toolchain-trusty-3.6 main"
    ${WGET} -O - http://llvm-apt.ecranbleu.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo apt-get update
    sudo apt-get install -y ${DEPENDENCIES}
    sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-3.6 20
    sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-3.6 20
    sudo update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-3.6 20
    sudo update-alternatives --install /usr/bin/llvm-link llvm-link /usr/bin/llvm-link-3.6 20
    sudo update-alternatives --install /usr/bin/llvm-dis llvm-dis /usr/bin/llvm-dis-3.6 20
    ;;

  linux-ubuntu-12*)
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo add-apt-repository -y ppa:andykimpe/cmake
    sudo apt-get update
    sudo apt-get install -y ${DEPENDENCIES}
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 20
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 20
    sudo update-alternatives --config gcc
    sudo update-alternatives --config g++
    ;;

  linux-cygwin*)
    ;;

  *)
    puts "Distribution ${distro} not supported. Dependencies must be installed manually."
    exit 1
    ;;
  esac

  puts "Installed required packages"
fi

if [ ${BUILD_DSA} -eq 1 ]
then
  puts "Building DSA"

  mkdir -p ${DSA_DIR}/build
  cd ${DSA_DIR}/build
  cmake ${CMAKE_INSTALL_PREFIX} -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug ..
  make
  sudo make install

  puts "Built DSA"
fi

exit $res
