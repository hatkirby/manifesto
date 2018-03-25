#include <yaml-cpp/yaml.h>
#include <twitter.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: manifesto [configfile]" << std::endl;
    return -1;
  }

  std::string configfile(argv[1]);
  YAML::Node config = YAML::LoadFile(configfile);

  twitter::auth auth;
  auth.setConsumerKey(config["consumer_key"].as<std::string>());
  auth.setConsumerSecret(config["consumer_secret"].as<std::string>());
  auth.setAccessKey(config["access_key"].as<std::string>());
  auth.setAccessSecret(config["access_secret"].as<std::string>());

  twitter::client client(auth);

  long loc = 0;
  if (config["current_location"])
  {
    loc = config["current_location"].as<long>();
  }

  std::list<std::string> tweets;
  {
    std::ifstream corpus(config["corpus"].as<std::string>());
    corpus.ignore(loc);

    char buffer[140];
    while (corpus)
    {
      corpus.read(buffer, 140);
      tweets.push_back(std::string(buffer, 140));
    }
  }

  while (!tweets.empty())
  {
    try
    {
      std::string nextTweet = tweets.front();
      client.updateStatus(nextTweet);

      std::cout << "Tweeted!" << std::endl;

      loc += 140;
      {
        std::ofstream fout(configfile);
        config["current_location"] = loc;
        fout << config;
      }

      tweets.pop_front();
    } catch (const twitter::twitter_error& e)
    {
      std::cout << "Twitter error: " << e.what() << std::endl;
    }

    std::cout << "Waiting..." << std::endl;

    std::this_thread::sleep_for(std::chrono::hours(6));
  }
}
