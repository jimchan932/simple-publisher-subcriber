#include <ctime>
#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>


using namespace std;

std::string readParagraph(std::istream &in)
{
  std::string result, line;

  // read and concatenate lines until two newlines are read
  while(std::getline(in, line)) 
    if(line.empty()) break;
    else result += line + ' ';

  // get rid of that last space
  result.erase(result.length() - 1);

  return result;
}

#define MST (-7)
#define UTC (0)
#define CCT (+8)
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/server_credentials.h>
#include "pub_sub.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using pubsub::PublishMessage;
using pubsub::PublishedTime;
using pubsub::PublisherSubscriber;

PublishMessage makePublishMessage(int topicID, int reviewID, std::string review)
{
  PublishMessage publishMsg;
  publishMsg.set_topicid(topicID);
  publishMsg.set_reviewid(reviewID);
  publishMsg.set_review(review);
  return publishMsg;
}

class PublisherClient
{
public:
  PublisherClient(std::shared_ptr<Channel> channel) :
    stub_(PublisherSubscriber::NewStub(channel)) { }
  void Publish(int topicID, std::string review)  {
    PublishMessage publishMsg = makePublishMessage(topicID, numOfReviews++,review);
    PublishedTime publishTime;
    PublishOnce(publishMsg, &publishTime);
  }
private:
  bool PublishOnce(const PublishMessage &publishMsg, PublishedTime *pubTime)
  {
    ClientContext context;
    Status status = stub_->Publish(&context, publishMsg, pubTime);
    std::cout << "Publication time: ";
    std::cout << pubTime->publishedtime() << "\n";
  }
  
  std::unique_ptr<PublisherSubscriber::Stub > stub_;

  int numOfReviews;
};

int main()
{  
  PublisherClient client(
			 grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  int topicID;
  std::cout << "Enter type of book review you want to publish" << std::endl;
  std::cout << "(1) Java\n(2) Python\n(3) C++\n";
  std::cin >> topicID;

  std::cout << "Enter file name: ";
  std::string filename;
  std::cin >> filename;
  std::string review;
  if(topicID == 1)
  {
    std::string filePath("../javaBookReview/");
    std::ifstream file(filePath + filename);
    std::cout << "Reading paragraph from file...\n";
    review = readParagraph(file);
    file.close();

  }
  else if(topicID == 2)
  {
    std::string filePath("../pythonBookReview/");
    std::ifstream file(filePath + filename);
    std::cout << "Reading paragraph from file...\n";
    review = readParagraph(file);
    file.close();
      
  }
  else
  {
    std::string filePath("../cppBookReview/");       
    std::ifstream file(filePath + filename);
    std::cout << "Reading paragraph from file...\n";
    review = readParagraph(file);
    file.close();
  }
  
  client.Publish(topicID, review);
  return 0;
}
