
include_directories(
  model
)

add_executable(FEATWord2Vec featword2vec.cpp)
target_link_libraries(FEATWord2Vec ${LIBS})
