add_executable(peninsula
    main.cpp
    stages.hpp
    stages.cpp
)

target_link_libraries(peninsula PRIVATE core)

set_property(TARGET peninsula PROPERTY CXX_STANDARD 23)