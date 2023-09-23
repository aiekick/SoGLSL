#ifndef HEADER_CURL_IDN_H
#define HEADER_CURL_IDN_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

#ifdef USE_WIN32_IDN
bool Curl_win32_idn_to_ascii(const char *in, char **out);
#endif /* USE_WIN32_IDN */
bool Curl_is_ASCII_name(const char *hostname);
CURLcode Curl_idnconvert_hostname(struct hostname *host);
#if defined(USE_LIBIDN2) || defined(USE_WIN32_IDN)
#define USE_IDN
void Curl_free_idnconverted_hostname(struct hostname *host);
char *Curl_idn_decode(const char *input);
#ifdef USE_LIBIDN2
#define Curl_idn_free(x) idn2_free(x)
#else
#define Curl_idn_free(x) free(x)
#endif

#else
#define Curl_free_idnconverted_hostname(x)
#define Curl_idn_decode(x) NULL
#endif
#endif /* HEADER_CURL_IDN_H */
