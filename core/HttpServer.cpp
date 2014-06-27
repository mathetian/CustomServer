// Copyright (c) 2014 The SealedServer Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "HttpServer.h"

namespace sealedserver
{

HttpServer::HttpServer(int port) : port_(port), errflag_(false)
{
	acceptor_ = new HttpAcceptor<HttpRequest>(this, pool_.getRandomLoop(), port);
}

HttpServer::~HttpServer()
{
	pool_.stop();

	delete acceptor_;
	acceptor_ = NULL;
}

void HttpServer::start()
{
	pool_.run();
}

void HttpServer::stop()
{
	pool_.stop();
}

void HttpServer::add(const string &url, Callback callback, void *arg)
{
	calls_[url] = make_pair(callback, arg);
}

void HttpServer::error(Callback callback, void *arg)
{
	error_   = make_pair(callback, arg);
	errflag_ = true;
}

bool HttpServer::process(HttpRequest *conn)
{
	string query = conn -> getQuery();

	if(calls_.find(query) ==  calls_.end())
	{
		conn -> initResponse(404);

		if(errflag_)
		{
			Callback callback = error_.first;
			void    *arg      = error_.second;

			callback(conn, arg);
		}
		else
		{
			conn -> notFound();
		}
	}
	else
	{
		conn -> initResponse(200);
		
		Callback callback = calls_[query].first;
		void    *arg      = calls_[query].second;
		callback(conn, arg);
	}
}

};