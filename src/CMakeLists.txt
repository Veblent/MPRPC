aux_source_directory(./ SRC_LIST)

# muduo源码安装生成的是静态链接库 .a
# add_library(mprpc SHARED ${SRC_LIST})

# add_executable(zkDemo test_zk.cc zookeeperutil.cc logger.cc mprpcapplication.cc mprpcconfig.cc)
# target_link_libraries(zkDemo mprpc protobuf muduo_net muduo_base pthread zookeeper_mt)


add_library(mprpc ${SRC_LIST})
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)