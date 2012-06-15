#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <cassert>
#include <map>
#include "onullstream.hpp"

extern "C" {
#include "daemon.h"
}

using namespace std;

namespace {

inline bool IsEOF(char c)
{
	return c == '\n';
}

inline bool IsLWS(char c)
{
	return c == ' ' || c == '\t';
}

inline bool IsQuote(char c)
{
	return c == '\"';
}

bool GetTokens(
	istream &InputStream,
	ostream &LogStream,
	int &LineNumber,
	vector<string> &Result)
{
	enum {
		STATE_DEFAULT,
		STATE_TOKEN,
		STATE_QUOTE,
		STATE_QUOTE_MARK,
		STATE_ERROR,
		STATE_END
	} state = STATE_DEFAULT;

	vector<string> tokens;
	string curToken;
	int pos = 0;
	
	while (1) {
		char c;
		if (!InputStream.get(c)) {
			if (InputStream.eof()) {
				state = STATE_END;
			} else {
				state = STATE_ERROR;
			}
			goto out;
		}

		if (pos == 0)
			++LineNumber;

		++pos;

		do {
			switch (state) {
			case STATE_DEFAULT:
				if (IsEOF(c)) {
					state = STATE_END;
					goto out;
				} else if (IsLWS(c)) {
					// do nothing
				} else if (IsQuote(c)) {
					state = STATE_QUOTE;
				} else {
					state = STATE_TOKEN;
					curToken.push_back(c);
				}
				break;
			case STATE_TOKEN:
				if (IsLWS(c) || IsEOF(c)) {
					state = STATE_DEFAULT;
					tokens.push_back(string());
					curToken.swap(tokens.back());
				} else if (IsQuote(c)) {
					state = STATE_ERROR;
					goto out;
				} else {
					curToken.push_back(c);
				}
				break;
			case STATE_QUOTE:
				if (IsQuote(c)) {
					state = STATE_QUOTE_MARK;
				} else if (IsEOF(c)) {
					state = STATE_ERROR;
					goto out;
				} else {
					curToken.push_back(c);
				}
				break;
			case STATE_QUOTE_MARK:
				if (IsQuote(c)) {
					state = STATE_QUOTE;
					curToken.push_back(c);
				} else if (IsLWS(c) || IsEOF(c)) {
					state = STATE_DEFAULT;
					tokens.push_back(string());
					curToken.swap(tokens.back());
				} else {
					state = STATE_ERROR;
					goto out;
				}
				break;
			default:
				assert(!"never be here");
			}
		} while (IsEOF(c));
	}

out:
	if (state == STATE_ERROR) {
		LogStream << "Error parsing line " << LineNumber << " at pos " << pos << endl;
		return false;
	}

	if (!curToken.empty()) {
		tokens.push_back(string());
		curToken.swap(tokens.back());
	}

	tokens.swap(Result);
	return true;
}

void PrintStatus(
	ostream &LogStream,
	CSTATUS Status)
{
	LogStream << "Error status " << Status << endl;
}

map<pair<string, int>, DM_ENDPOINT *> Endpoints;

void ParseConfig(
	istream &InputStream,
	ostream &OutputStream,
	ostream &LogStream)
{
	vector<pair<string, DM_NODE *> > nodes;
	vector<pair<string, int> > siteEps;
	vector<string> siteHosts;
	bool siteDefaultHost = false;
	int lineNumber = 0;

	while (InputStream) {
		vector<string> tokens;

		OutputStream << "icyhttpd";
		if (!nodes.empty()) {
			int index = 0;

			for (vector<pair<string, DM_NODE *> >::const_iterator iter = nodes.begin();
				iter != nodes.end(); ++iter)
				OutputStream << (index++ ? "/" : ":") << iter->first;
		}
		OutputStream << ">";

		if (GetTokens(InputStream, LogStream, lineNumber, tokens)) {
			if (tokens.empty())
				continue;

			string command;
			transform(tokens.front().begin(), tokens.front().end(),
				back_inserter(command), ptr_fun(::tolower));

			if (command == "site") {
				if (!nodes.empty()) {
					LogStream << "You must end the current site before creating a new one." << endl;
					continue;
				}

				DM_NODE *node;
				CSTATUS status = DmCreateNode(&node);
				if (!SUCCESS(status)) {
					PrintStatus(LogStream, status);
					continue;
				}

				nodes.push_back(make_pair(string("site"), node));

			} else if (command == "bindep") {

				if (tokens.size() < 3) {
					LogStream << "Parameter count incorrect." << endl;
					continue;
				}

				istringstream iss(tokens[2]);
				int port;
				if (!(iss >> port)) {
					LogStream << "Error parsing port number." << endl;
					continue;
				}

				if (nodes.empty()) {
					LogStream << "You must create a site first." << endl;
					continue;
				}

				DM_ENDPOINT *endpoint = Endpoints[make_pair(tokens[1], port)];
				if (endpoint == NULL) {
					CSTATUS status = DmCreateEndpoint(&endpoint, tokens[1].c_str(), port);
					if (!SUCCESS(status)) {
						PrintStatus(LogStream, status);
						continue;
					}
					Endpoints[make_pair(tokens[1], port)] = endpoint;
				}

				// Bind every existing hosts
				for (vector<string>::const_iterator iter = siteHosts.begin();
					iter != siteHosts.end(); ++iter)
				{
					CSTATUS status = DmAttachNodeEndpoint(endpoint, nodes.front().second, iter->c_str());
					if (!SUCCESS(status)) {
						LogStream << "Failed to bind host \'" << *iter << "\' with the current endpoint." << endl;
					}
				}

				if (siteDefaultHost) {
					CSTATUS status = DmAttachNodeEndpoint(endpoint, nodes.front().second, NULL);
					if (!SUCCESS(status)) {
						LogStream << "Failed to bind default host with the current endpoint." << endl;
					}
				}

				siteEps.push_back(make_pair(tokens[1], port));
				
			} else if (command == "bindhost") {

				if (tokens.size() < 2) {
					LogStream << "Parameter count incorrect." << endl;
					continue;
				}

				if (nodes.empty()) {
					LogStream << "You must create a site first." << endl;
					continue;
				}

				// Bind every existing endpoints
				for (vector<pair<string, int> >::const_iterator iter = siteEps.begin();
					iter != siteEps.end(); ++iter)
				{
					CSTATUS status = DmAttachNodeEndpoint(Endpoints[*iter], nodes.front().second, tokens[1].c_str());
					if (!SUCCESS(status)) {
						LogStream << "Failed to bind endpoint \'" << iter->first << ":" << iter->second << "\' with the current host." << endl;
					}
				}

				siteHosts.push_back(tokens[1]);
			} else if (command == "binddefaulthost") {

				if (nodes.empty()) {
					LogStream << "You must create a site first." << endl;
					continue;
				}

				// Bind every existing endpoints
				for (vector<pair<string, int> >::const_iterator iter = siteEps.begin();
					iter != siteEps.end(); ++iter)
				{
					CSTATUS status = DmAttachNodeEndpoint(Endpoints[*iter], nodes.front().second, NULL);
					if (!SUCCESS(status)) {
						LogStream << "Failed to bind endpoint \'" << iter->first << ":" << iter->second << "\' with the default host." << endl;
					}
				}

				siteDefaultHost = true;

			} else if (command == "docroot") {

				if (tokens.size() < 2) {
					LogStream << "Parameter count incorrect." << endl;
					continue;
				}

				if (nodes.empty()) {
					LogStream << "You must create a node first." << endl;
					continue;
				}

				size_t len;
				CSTATUS status = AnsiToUtf16GetSize(&len, tokens[1].c_str());
				if (!SUCCESS(status)) {
					PrintStatus(LogStream, status);
					continue;
				}

				vector<wchar_t> buffer(len);
				status = AnsiToUtf16(&buffer.front(), buffer.size(), tokens[1].c_str());
				if (!SUCCESS(status)) {
					PrintStatus(LogStream, status);
					continue;
				}

				status = DmSetDocumentRootNode(nodes.back().second, &buffer.front());
				if (!SUCCESS(status)) {
					PrintStatus(LogStream, status);
					continue;
				}

			} else if (command == "addhandler") {

				if (tokens.size() < 3) {
					LogStream << "Parameter count incorrect." << endl;
					continue;
				}

				if (nodes.empty()) {
					LogStream << "You must create a node first." << endl;
					continue;
				}

				DM_HANDLER *handler;
				const char *param = (tokens.size() == 3) ? "" : tokens[3].c_str();
				CSTATUS status = DmCreateHandler(&handler, tokens[2].c_str(), param);
				if (!SUCCESS(status)) {
					PrintStatus(LogStream, status);
					continue;
				}

				const char *ext = (tokens[1] == "*") ? NULL : tokens[1].c_str();
				status = DmAttachHandlerNode(nodes.back().second, handler, ext);
				if (!SUCCESS(status)) {
					PrintStatus(LogStream, status);
					continue;
				}
			
			} else if (command == "siteend") {
				if (nodes.empty()) {
					LogStream << "You must create a node first." << endl;
					continue;
				} else if (nodes.size() > 1) {
					LogStream << "You must end existing virtual nodes first." << endl;
					continue;
				}

				if (siteEps.empty()) {
					LogStream << "Hint: you haven't bind any endpoints." << endl;
				} else if (siteHosts.empty() && !siteDefaultHost) {
					LogStream << "Hint: you haven't bind any hosts." << endl;
				}

				nodes.pop_back();
				siteEps.clear();
				siteHosts.clear();
				siteDefaultHost = false;
			} else {
				LogStream << "Unknown command \'" << command << "\'" << endl;
				continue;
			}
		}
	}
}

}

extern "C"
void DmConfigParserEntry(void)
{
	ifstream fin("icyhttpd.conf");
	if (!fin.is_open()) {
		cerr << "Cannot open \'icyhttpd.conf\'" << endl;
		exit(1);
	}

	ParseConfig(fin, onullstream(), cerr);
}
