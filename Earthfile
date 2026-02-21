VERSION 0.8

# Earthly build for samp-log-core
#
# Targets:
#   +configure         - Configure CMake project
#   +build             - Build shared library artifact
#   +package           - Build and export compiled artifacts
#   +clean             - Remove build directory
#
# Notes:
# - This project uses Conan via cmake/conan.cmake during configure.
# - Linux x86 build is used to match the CMake/Conan ARCH x86 setting.

configure:
    FROM ubuntu:20.04
    WORKDIR /src

    RUN dpkg --add-architecture i386
    RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential \
        gcc-multilib \
        g++-multilib \
        libc6-dev-i386 \
        libstdc++6:i386 \
        cmake \
        make \
        python3 \
        python3-pip \
        git \
        pkg-config \
        && rm -rf /var/lib/apt/lists/*

    # Conan 1.x is needed for conan.cmake's conan_cmake_run flow.
    RUN pip3 install --no-cache-dir "conan<2"

    COPY . .

    RUN mkdir -p build
    RUN conan profile new default --detect --force || true
    RUN conan profile update settings.arch=x86 default
    RUN conan profile update settings.compiler.libcxx=libstdc++11 default || true

    RUN cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_FLAGS="-m32" \
        -DCMAKE_CXX_FLAGS="-m32" \
        -DCMAKE_SHARED_LINKER_FLAGS="-m32" \
        -DCMAKE_EXE_LINKER_FLAGS="-m32"

    SAVE ARTIFACT build AS LOCAL build-configured

build:
    FROM +configure
    WORKDIR /src

    RUN cmake --build build --config Release -- -j"$(nproc)"

    # Main output expected by src/CMakeLists.txt
    RUN test -d build/artifact
    SAVE ARTIFACT build/artifact AS LOCAL artifact

package:
    FROM +build
    WORKDIR /src

    # Export commonly useful files alongside binaries
    RUN mkdir -p /out
    RUN cp -r build/artifact /out/artifact
    RUN cp -r include /out/include
    RUN cp -f LICENSE /out/LICENSE
    RUN cp -f README.md /out/README.md

    SAVE ARTIFACT /out AS LOCAL package

clean:
    FROM ubuntu:22.04
    WORKDIR /src
    COPY . .
    RUN rm -rf build
    SAVE ARTIFACT . AS LOCAL cleaned-source
