
include_directories(
  model
)

add_executable(ContextWord2Vec word2vec.cpp)
target_link_libraries(ContextWord2Vec ${LIBS})
