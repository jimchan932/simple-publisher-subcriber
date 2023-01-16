#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <utility>
#include <memory>
#include <string>
#include <sstream>
#include <cstring>
#include <map>
#include <queue>
#include <vector>
#include <list>
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "pub_sub.grpc.pb.h"

std::string int_to_str(int x)
{
  std::stringstream ss;
  ss << x;
  return ss.str();
}

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using pubsub::PublishMessage;
using pubsub::SubscribeInfo;
using pubsub::ReceivedMessage;
using pubsub::PublishedTime;
using pubsub::PublisherSubscriber;

#define DEFAULT_DURATION 60 // one minute
typedef time_t timestamp;
struct BookReview
{
  int reviewID;
  std::string review;
  std::string publishedTime;
};

typedef std::queue<BookReview > MessageQueue;
typedef std::map<int, MessageQueue> MessagePartition;

class PublisherSubscriberImpl final : public PublisherSubscriber::Service {
 public:
  Status Publish(ServerContext* context, const PublishMessage *pubMessage, PublishedTime *pubTime) override {
    timestamp currentTimestamp;   
    time(&currentTimestamp);    
    struct BookReview bookReview;
    bookReview.publishedTime = ctime(&currentTimestamp);
    bookReview.review = pubMessage->review();
    bookReview.reviewID = pubMessage->reviewid();
    
    switch(pubMessage->topicid())      
    {
      case 1:
	
		  for(auto it = javaBookReviewPartition.begin(); it != javaBookReviewPartition.end(); it++)
		  {
			  it->second.push(bookReview);
		  }
		  javaBookReviewList.push_back(std::make_pair(currentTimestamp,bookReview));
		  break;	
      case 2:
		  for(auto it = pythonBookReviewPartition.begin(); it != pythonBookReviewPartition.end(); it++)
		  {
			  it->second.push(bookReview);
		  }
		  pythonBookReviewList.push_back(std::make_pair(currentTimestamp,bookReview));	
		  break;
      case 3:
		  for(auto it = cppBookReviewPartition.begin(); it != cppBookReviewPartition.end(); it++)
		  {
			  it->second.push(bookReview);
		  }
		  cppBookReviewList.push_back(std::make_pair(currentTimestamp,bookReview));	
		  break;	
    }
    
    pubTime->set_publishedtime(bookReview.publishedTime);
    return Status::OK;
  }
  Status Subscribe(ServerContext* context,
		   const SubscribeInfo* subscribeInfo,
		   ServerWriter<ReceivedMessage>* writer) override {
    std::list<std::pair<timestamp, struct BookReview> >::iterator begin, end;
    int subscriberID;
    
    switch(subscribeInfo->subscriptiontopicid())
    {
    case 1:
      begin = javaBookReviewList.begin();
      end = javaBookReviewList.end();
      subscriberID = num_javaSubscribers++;
      javaBookReviewPartition.insert(MessagePartition::value_type(subscriberID, MessageQueue()));      
      break;
    case 2:
      begin = pythonBookReviewList.begin();
      end = pythonBookReviewList.end();
      subscriberID = num_pythonSubscribers++;
      pythonBookReviewPartition.insert(MessagePartition::value_type(subscriberID, MessageQueue()));
      break;
    case 3:
      begin = cppBookReviewList.begin();
      end = cppBookReviewList.end();
      subscriberID = num_cppSubscribers++;
      cppBookReviewPartition.insert(MessagePartition::value_type(subscriberID, MessageQueue()));      
      break;      
    }
    
    timestamp currentTimestamp;
    time(&currentTimestamp);    
    for(auto it = begin; it != end; it++)
    {
      timestamp createdTimestamp = it->first;
      if(difftime(currentTimestamp, createdTimestamp) > DEFAULT_DURATION)
      {
		  continue;
      }     
      ReceivedMessage recvMsg;      
      recvMsg.set_subscriberid(subscriberID);
      recvMsg.set_publishedtime(it->second.publishedTime);
      recvMsg.set_review(it->second.review);        
      writer->Write(recvMsg);
    }
    return Status::OK;
  }
	
  Status GetSubscriptionMsg(ServerContext* context,
		   const SubscribeInfo* subscriberInfo,
		   ServerWriter<ReceivedMessage>* writer) override
  {
    int subID = subscriberInfo->subscriberid();
    if(subscriberInfo->subscriptiontopicid() == 1)
    {
      
		while(!javaBookReviewPartition[subID].empty())
		{
			ReceivedMessage recvMsg;
			recvMsg.set_subscriberid(subID);
			recvMsg.set_publishedtime(javaBookReviewPartition[subID].front().publishedTime);
			recvMsg.set_review(javaBookReviewPartition[subID].front().review);
			javaBookReviewPartition[subID].pop();
			writer->Write(recvMsg);
		}
    }
    
    else if(subscriberInfo->subscriptiontopicid() == 2)
    {
		
		while(!pythonBookReviewPartition[subID].empty())
		{
			ReceivedMessage recvMsg;
			recvMsg.set_subscriberid(subID);
			recvMsg.set_publishedtime(pythonBookReviewPartition[subID].front().publishedTime);
			recvMsg.set_review(pythonBookReviewPartition[subID].front().review);
			pythonBookReviewPartition[subID].pop();
			writer->Write(recvMsg);
		}
    }
    else
    {
		while(!cppBookReviewPartition[subID].empty())
		{
			ReceivedMessage recvMsg;
			recvMsg.set_subscriberid(subID);
			recvMsg.set_publishedtime(cppBookReviewPartition[subID].front().publishedTime);
			recvMsg.set_review(cppBookReviewPartition[subID].front().review);
			cppBookReviewPartition[subID].pop();
			writer->Write(recvMsg);
		}
    }
    
    return Status::OK;
  }
private:
  // int here is subscriberID
  MessagePartition javaBookReviewPartition;
  MessagePartition pythonBookReviewPartition;
  MessagePartition cppBookReviewPartition;  
  std::list<std::pair<timestamp, BookReview> > javaBookReviewList;
  std::list<std::pair<timestamp, BookReview> > pythonBookReviewList;
  std::list<std::pair<timestamp, BookReview> > cppBookReviewList;
  int num_javaSubscribers;
  int num_cppSubscribers;
  int num_pythonSubscribers;

};

void RunServer()
{
  std::string server_address("0.0.0.0:50051");
  PublisherSubscriberImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv)
{  
  RunServer();

  return 0;
}
