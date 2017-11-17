
include_directories(
  model
)

add_executable(CHNWord2Vec chnword2vec.cpp)
target_link_libraries(CHNWord2Vec ${LIBS})
