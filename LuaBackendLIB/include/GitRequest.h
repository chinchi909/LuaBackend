#pragma once

#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <string>

#include <json/json.hpp>

#pragma comment(lib, "wininet.lib")

#define CURRENT_VERSION 1.15
#define GITHUB_LINK L"api.github.com"
#define REQUEST_URI L"/repos/Topaz-Reality/LuaBackend/releases/latest"

using namespace std;
using namespace nlohmann;

class GitRequest
{
public:
	static int GetVersion()
	{
		HINTERNET hSession = InternetOpen(L"Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		HINTERNET hConnect = InternetConnect(hSession, GITHUB_LINK, 0, L"", L"", INTERNET_SERVICE_HTTP, 0, 0);
		HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", REQUEST_URI, NULL, NULL, NULL, 0, 0);

		if (!hRequest)
			return -2;

		else
		{
			BOOL bRequestSent = HttpSendRequest(hRequest, NULL, 0, NULL, 0);

			if (!bRequestSent)
				return -1;

			else
			{
				string strResponse;
				const int nBuffSize = 1024;
				char buff[nBuffSize];

				BOOL bKeepReading = true;
				DWORD dwBytesRead = -1;

				while (bKeepReading && dwBytesRead != 0)
				{
					bKeepReading = InternetReadFile(hRequest, buff, nBuffSize, &dwBytesRead);
					strResponse.append(buff, dwBytesRead);
				}

				auto _json = json::parse(strResponse);
				auto _tag = _json["tag_name"].get<string>();

				InternetCloseHandle(hRequest);
				InternetCloseHandle(hConnect);
				InternetCloseHandle(hSession);

				auto _tagNumber = stof(_tag.erase(0, 1));

				if (_tagNumber > CURRENT_VERSION)
					return 1;

				else
					return 0;
			}
		}
	}
};

