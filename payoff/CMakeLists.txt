add_library(payoff INTERFACE
        FixingDate.h
        PayoffNode.h
        PLVisitor.h
)
target_include_directories(payoff INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(payoff INTERFACE numerics)
