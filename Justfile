default_type := 'Debug'
#default_type := 'RelWithDebInfo'

export CC := '/usr/bin/gcc-14'
export CXX := '/usr/bin/g++-14'

go type=default_type: (build type)
    ./build/{{type}}/clobber

debug type=default_type: (build type)
    gdb ./build/{{type}}/clobber

build type=default_type:
    cmake --build build/{{type}}

configure type=default_type:
    rm -rf ./build/{{type}}
    cmake -S src -B build/{{type}} -D CMAKE_BUILD_TYPE={{type}}

clean:
    rm -rf ./build