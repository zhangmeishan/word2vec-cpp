
include_directories(
  model
)

add_executable(SUBWord2Vec subword2vec.cpp)
target_link_libraries(SUBWord2Vec ${LIBS})
