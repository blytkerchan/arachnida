/** \mainpage Arachnida/Spin: Dutch for spider.
 * Spin is a small, simple to use, full-featured HTTP(S) server. Below, the
 * \em entire source code of the "Hello, world!" equivalent of an HTTP server
 * is reproduced. I.e., the Mesothelae mini-server serves up only two pages but 
 * has been tested with both Mozilla Firefox and Microsoft Internet Explorer as 
 * clients. It replies to a request for / with a redirect to /indes.html, which 
 * it serves up with a bit of HTP containing "Hello, world!". Connections are 
 * handled asynchronously and requests are put in the request handler, where the 
 * main thread will find them calling getNextRequest.
 * \include Mesothelae/main.cpp */