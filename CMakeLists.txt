cmake_minimum_required(VERSION 3.15)
project(DCM)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/local/Cellar/libtool/2.4.6_1/include)
include_directories(/usr/local/Cellar/libomp/9.0.0/include)

link_directories(/usr/local/Cellar/libtool/2.4.6_1/lib)
link_directories(/usr/local/Cellar/libomp/9.0.0/lib)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.45.0 COMPONENTS program_options)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    include_directories(Abstraction/src)
    include_directories(Base/src)
    include_directories(CDA/src)
    include_directories(Chronicle/src)
    include_directories(Domain/src)
    include_directories(Parser/src)
    include_directories(Ripper/ripper/code)
    include_directories(Ripper/src)

    add_executable(DCM
            Abstraction/src/Abstraction.cpp
            Abstraction/src/Abstraction.hh
            Base/src/Base.cpp
            Base/src/Base.hh
            Base/src/Event.hh
            Base/src/VerticalEventBase.hh
            CDA/src/CDA.cpp
            CDA/src/CDA.hh
            Chronicle/src/Chronicle.cpp
            Chronicle/src/Chronicle.hh
            Chronicle/src/EMSet.hh
            Chronicle/src/MSetOcc.cpp
            Chronicle/src/MSetOcc.hh
            Chronicle/src/TC.cpp
            Chronicle/src/TC.hh
            Chronicle/src/TCIterator.hh
            Domain/src/interval_pruning.cpp
            Domain/src/interval_pruning.hh
            Domain/src/RealVector.hh
            Domain/src/RealVector.cpp
            Domain/src/DomainElementFactory.hh
            Domain/src/DomainElementFactory.cpp
            Domain/src/DomainElement.hh
            Parser/src/functions.cpp
            Parser/src/functions.hh
            Ripper/ripper/code/add-redundancy.c
            Ripper/ripper/code/concept.c
            Ripper/ripper/code/crossval.c
            Ripper/ripper/code/desref.c
            Ripper/ripper/code/dougsort.c
            Ripper/ripper/code/error.c
            Ripper/ripper/code/extend-rules.c
            Ripper/ripper/code/extras.h
            Ripper/ripper/code/fit.c
            Ripper/ripper/code/gram.c
            Ripper/ripper/code/gramaux.c
            Ripper/ripper/code/intern.c
            Ripper/ripper/code/lex.c
            Ripper/ripper/code/mainaux.c
            Ripper/ripper/code/mdb.h
            Ripper/ripper/code/mdb_count.c
            Ripper/ripper/code/mdb_load.c
            Ripper/ripper/code/mdb_names.c
            Ripper/ripper/code/mdb_opt.c
            Ripper/ripper/code/mdb_util.c
            Ripper/ripper/code/mdl.c
            Ripper/ripper/code/memory.c
            Ripper/ripper/code/model.c
            Ripper/ripper/code/protos.h
            Ripper/ripper/code/ripper.h
            Ripper/ripper/code/rule.c
            Ripper/ripper/code/time.c
            Ripper/ripper/code/trace.c
            Ripper/ripper/code/trace.h
            Ripper/ripper/code/types.c
            Ripper/ripper/code/value.c
            Ripper/ripper/code/vector.c
            Ripper/ripper/code/vector.h
            Ripper/src/ripper.cpp
            Ripper/src/ripper.hh
            Ripper/src/GeneralRule.hh
            Ripper/src/MultiRule.hh
            Ripper/src/SingleRule.hh
            main.cpp)

    target_link_libraries(DCM ${Boost_LIBRARIES})
endif()
