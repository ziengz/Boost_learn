// grpc_server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <grpcpp/health_check_service_interface.h>  //健康检查服务
#include <grpcpp/ext/proto_server_reflection_plugin.h>  //反射功能
#include "demo.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using hello::Greeter;
using hello::HelloReply;
using hello::HelloRequest;

class GreeterServiceImpl final :public Greeter::Service {
    Status SayHello(ServerContext* context, const HelloRequest* request, HelloReply* reply)override {
        std::string prefix("grpc server has received: ");
        reply->set_message(prefix + request->message());
        std::cout << prefix + request->message() << std::endl;
        return Status::OK;
    }
};

void RunServer(){
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;
    //启用默认健康检查的服务
    //允许外部工具检查服务器是否健康
    grpc::EnableDefaultHealthCheckService(true);

    //启用反射服务
    //允许客户端动态发现可用的服务和方法
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    ServerBuilder builder;
    //监听给定地址                           表示不使用加密
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);  //注册服务实力
    std::unique_ptr<Server>server(builder.BuildAndStart());
    
    std::cout << "服务器已启动，监听地址为：" << server_address << std::endl;
    server->Wait();
    
}

int main()
{
    RunServer();
    return 0;
}

