// NoodlesPlate Copyright (C) 2017-2023 Stephane Cuillerdier aka Aiekick
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifdef USE_NETWORK
#include "FeedbackSender.h"

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include <ctools/cTools.h>

  /*
   * For an SMTP example using the multi interface please see smtp-multi.c.
   */

   /* The libcurl options want plain addresses, the viewable headers in the mail
	* can very well get a full name as well.
	*/
#define FROM_ADDR    "<user@funparadigm.com>"
#define TO_ADDR      "<admin@funparadigm.com>"
#define CC_ADDR      "<info@example.org>"

#define FROM_MAIL "Sender Person " FROM_ADDR
#define TO_MAIL   "A Receiver " TO_ADDR
#define CC_MAIL   "John CC Smith " CC_ADDR

static const char *payload_text[] = {
  "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
  "To: " TO_MAIL "\r\n",
  "From: " FROM_MAIL "\r\n",
  "Cc: " CC_MAIL "\r\n",
  "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
  "rfcpedant.example.org>\r\n",
  "Subject: SMTP example message\r\n",
  "\r\n", /* empty line to divide headers from body, see RFC5322 */
  "The body of the message starts here.\r\n",
  "\r\n",
  "It could be a lot of lines, could be MIME encoded, whatever.\r\n",
  "Check RFC5322.\r\n",
  nullptr
};

struct upload_status {
	int lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct upload_status *upload_ctx = (struct upload_status *)userp;
	const char *data;

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	data = payload_text[upload_ctx->lines_read];

	if (data) {
		const size_t len = strlen(data);
		memcpy(ptr, data, len);
		upload_ctx->lines_read++;

		return len;
	}

	return 0;
}

FeedbackSender::FeedbackSender()
{

}

FeedbackSender::~FeedbackSender()
{

}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

void FeedbackSender::SendFeedbackMail()
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = nullptr;
	struct upload_status upload_ctx;

	upload_ctx.lines_read = 0;

	curl = curl_easy_init();
	if (curl) 
	{
		/* This is the URL for your mailserver */
		curl_easy_setopt(curl, CURLOPT_URL, "smtp://mail.gandi.net");

		/* Note that this option isn't strictly required, omitting it will result
		 * in libcurl sending the MAIL FROM command with empty sender data. All
		 * autoresponses should have an empty reverse-path, and should be directed
		 * to the address in the reverse-path which triggered them. Otherwise,
		 * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
		 * details.
		 */
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_ADDR);

		/* Add two recipients, in this particular case they correspond to the
		 * To: and Cc: addressees in the header, but they could be any kind of
		 * recipient. */
		recipients = curl_slist_append(recipients, TO_ADDR);
		recipients = curl_slist_append(recipients, CC_ADDR);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		/* We're using a callback function to specify the payload (the headers and
		 * body of the message). You could just use the CURLOPT_READDATA option to
		 * specify a FILE pointer to read from. */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* Send the message */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		/* Free the list of recipients */
		curl_slist_free_all(recipients);

		/* curl won't send the QUIT command until you call cleanup, so you should
		 * be able to re-use this connection for additional messages (setting
		 * CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
		 * curl_easy_perform() again. It may not be a good idea to keep the
		 * connection open for a very long time though (more than a few minutes
		 * may result in the server timing out the connection), and you do want to
		 * clean up in the end.
		 */
		curl_easy_cleanup(curl);
	}
}

void FeedbackSender::SendIssueMail()
{

}

void FeedbackSender::SendCongratMail()
{

}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string FeedbackSender::getXml(const std::string& /*vOffset*/, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	//str += vOffset + "<file type=\"" + itMap->first + "\" path=\"" + *itLst + "\"/>\n";

	return str;
}

bool FeedbackSender::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strName == "file")
	{
		//std::string type;
		
		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			//std::string attName = attr->Name();
		//	std::string attValue = attr->Value();

			//if (attName == "type")
			//	type = attValue;
		}
	}

	return false;
}
#endif // #ifdef USE_NETWORK