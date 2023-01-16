#include <ctime>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#define MST (-7)
#define UTC (0)
#define CCT (+8)
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/server_credentials.h>
#include "pub_sub.grpc.pb.h"
#include "pub_sub.grpc.pb.h"
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using pubsub::PublishMessage;
using pubsub::SubscribeInfo;
using pubsub::ReceivedMessage;
using pubsub::PublisherSubscriber;
class SubscriberClient
{
public:
  SubscriberClient(std::shared_ptr<Channel> channel, int topicid) : 
    stub_(PublisherSubscriber::NewStub(channel))
  {
    topicID = topicid;
  }
  void Subscribe()  {
    ClientContext context;
    ReceivedMessage recvMsg;
    SubscribeInfo subscribeInfo;    
    subscribeInfo.set_subscriptiontopicid(topicID);
    std::unique_ptr<ClientReader<ReceivedMessage> > reader(
	stub_->Subscribe(&context, subscribeInfo));
    while(reader->Read(&recvMsg))
    {
      std::cout << recvMsg.publishedtime() << ": ";
      std::cout << recvMsg.review();
      }
    Status status = reader->Finish();
    if(status.ok())
    {
      std::cout << "Subscribe success" << std::endl;
    }
    subscriberID = recvMsg.subscriberid();
  }
  void GetSubscriptionMsg()  {
    ClientContext context;
    ReceivedMessage recvMsg;
    SubscribeInfo subscribeInfo;    
    subscribeInfo.set_subscriptiontopicid(topicID);
    subscribeInfo.set_subscriberid(subscriberID);
    std::unique_ptr<ClientReader<ReceivedMessage> > reader(
	stub_->GetSubscriptionMsg(&context, subscribeInfo));
    while(reader->Read(&recvMsg))
    {
      std::cout << recvMsg.publishedtime() << ": ";
      std::cout << recvMsg.review() << std::endl;
    }
    Status status = reader->Finish();
    if(status.ok())
    {
      std::cout << "Received new subscription messages success" << std::endl;
    }
  } 
private:
  int subscriberID;
  int topicID;
  std::unique_ptr<PublisherSubscriber::Stub > stub_;
};

int main()
{
  using namespace std::chrono_literals;
  std::cout << "Enter subscription topic: (1) Java (2) Python (3) C++\n";
  int topic;
  std::cin >> topic;
  SubscriberClient client(
			  grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()), topic); // receive java book review
  client.Subscribe();
   
  for(;;)
  {
    std::this_thread::sleep_for(10000ms);
    client.GetSubscriptionMsg();
    
  }
  
  return 0;
}
