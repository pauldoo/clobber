default_type := 'Debug'
#default_type := 'RelWithDebInfo'

export CC := '/usr/bin/gcc-14'
export CXX := '/usr/bin/g++-14'

go type=default_type:
    rm -rf ./build/{{type}}
    cmake -S src -B build/{{type}} -D CMAKE_BUILD_TYPE={{type}}
    cmake --build build/{{type}}
    ./build/{{type}}/clobber

