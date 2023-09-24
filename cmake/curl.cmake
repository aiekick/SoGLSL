if (USE_NETWORK)
	set(HTTP_ONLY ON CACHE BOOL "" FORCE)
	set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
	set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
	set(CURL_BROTLI OFF CACHE BOOL "" FORCE)
	set(CURL_CA_FALLBACK OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_ALTSVC ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_COOKIES ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_CRYPTO_AUTH OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_DICT ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_DOH ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_FILE OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_FTP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_GETOPTIONS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_GOPHER ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_HSTS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_HTTP OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_HTTP_AUTH OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_IMAP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_LDAPS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_LIBCURL_OPTION ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_MIME ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_MQTT ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_NETRC ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_NTLM ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_OPENSSL_AUTO_LOAD_CONFIG OFF CACHE BOOL "" FORCE)
	set(CURL_DISABLE_PARSEDATE ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_POP3 ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_PROGRESS_METER ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_PROXY ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_RTSP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_SHUFFLE_DNS ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_SMB ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_SMTP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_SOCKETPAIR ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_TELNET ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_TFTP ON CACHE BOOL "" FORCE)
	set(CURL_DISABLE_VERBOSE_STRINGS ON CACHE BOOL "" FORCE)
	set(CURL_ENABLE_EXPORT_TARGET ON CACHE BOOL "" FORCE)
	set(CURL_ENABLE_SSL ON CACHE BOOL "" FORCE)
	set(CURL_HIDDEN_SYMBOLS ON CACHE BOOL "" FORCE)
	set(CURL_LTO OFF CACHE BOOL "" FORCE)
	set(CURL_USE_BEARSSL OFF CACHE BOOL "" FORCE)
	set(CURL_USE_GSSAPI OFF CACHE BOOL "" FORCE)
	set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
	set(CURL_USE_LIBSSH OFF CACHE BOOL "" FORCE)
	set(CURL_USE_LIBSSH2 OFF CACHE BOOL "" FORCE)
	set(CURL_USE_MBEDTLS OFF CACHE BOOL "" FORCE)
	set(CURL_USE_NSS OFF CACHE BOOL "" FORCE)
	set(CURL_USE_OPENSSL ON CACHE BOOL "" FORCE)
	set(CURL_USE_SCHANNEL OFF CACHE BOOL "" FORCE)
	set(CURL_USE_WOLFSSL OFF CACHE BOOL "" FORCE)
	set(CURL_WERROR OFF CACHE BOOL "" FORCE)
	set(CURL_ZLIB AUTO CACHE STRING "" FORCE)
	set(CURL_ZSTD OFF CACHE BOOL "" FORCE)
	set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)

	if(MSVC)
		set(CURL_STATIC_CRT OFF CACHE BOOL "" FORCE)
		set(OPENSSL_MSVC_STATIC_RT ON CACHE BOOL "" FORCE)
	endif()
	
	add_definitions(-DCURL_STATICLIB)
	add_definitions(-DUSE_OPENSSL)
	add_definitions(-DUSE_NETWORK)

	set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "" FORCE)
	set(LLVM_USE_CRT_MINSIZEREL MT CACHE STRING "" FORCE)
	set(LLVM_USE_CRT_RELEASE MT CACHE STRING "" FORCE)
	set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "" FORCE)
	set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")

	##EXCLUDE_FROM_ALL reject install for this target
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/curl EXCLUDE_FROM_ALL)

	set(CURL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/curl/include)
	include_directories(${CURL_INCLUDE_DIR}/curl)

	set(CURL_LIBRARIES CURL::libcurl)

	set_target_properties(libcurl PROPERTIES FOLDER 3rdparty)
endif()