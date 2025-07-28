# linux-net-code
本仓库为作者阅读以下书籍的积累的代码
《unix网络编程第一卷》
《linux高性能服务器编程》

yaml-cpp引入 \
mkdir src/netproxy/3rd\
cd src/netproxy/3rd\
git clone https://gitee.com/mirrors/yaml-cpp.git\
cd yaml-cpp \
mkdir build\
cd build\
cmake -DYAML_BUILD_SHARED_LIBS=on ..\
make