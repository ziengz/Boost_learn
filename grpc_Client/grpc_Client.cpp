#include <iostream>
#include <grpcpp/grpcpp.h>
#include "demo.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using hello::HelloReply;
using hello::Greeter;
using hello::HelloRequest;

class Client
{
public:
	Client(std::shared_ptr<Channel>channel) :
		stub_(Greeter::NewStub(channel)) {

	}
	std::string SayHello(std::string name)
	{
		HelloReply reply;
		HelloRequest request;
		ClientContext context;
		request.set_message(name);

		Status status = stub_->SayHello(&context, request, &reply);
		if (status.ok()) {
			return reply.message();
		}
		else {
			return "failed " + status.error_message();
		}
	}
private:
	std::unique_ptr<Greeter::Stub>stub_;
};

int main()
{
	auto channel = grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials());
	Client client(channel);
	std::string result = client.SayHello("hello，i'm zzp");
	std::cout << "Get result is " << result << std::endl;
	system("pause");
}