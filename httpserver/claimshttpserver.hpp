
#ifndef HTTP_CLAMISHTTPSERVER_HPP
#define HTTP_CLAMISHTTPSERVER_HPP


#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "server.hpp"

namespace httpserver {

int hargc;
char* hargv[5];

void httpserver_init(){
	hargc = 5;
	hargv[0] = "";
	hargv[1] = "219.228.147.167";
	hargv[2] = "8097";
	hargv[3] = "20";
	hargv[4] = "/home/hcs/local/test/";
}
int init(int iargc, char* iargv[]){
	try{
		if(iargc != 5){
      		std::cerr << "The httpserver cannot be started.\nUsage: http_server <address> <port> <threads> <doc_root>\n";
		}
		std::size_t num_threaders = boost::lexical_cast<std::size_t>(iargv[3]);
		httpserver::server s(iargv[1], iargv[2], iargv[4], num_threaders);
		s.run();
	//std::cout<<"the main support multithreaded.\n";
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}
	return 0;
}





}



#endif
