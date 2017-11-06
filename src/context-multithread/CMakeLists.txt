
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -pthread" )
include_directories(
  model
)

add_executable(ContextWord2Vec word2vec.cpp)
target_link_libraries(ContextWord2Vec ${LIBS})
