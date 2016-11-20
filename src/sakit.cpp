/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "PlatformSocket.h"
#include "sakit.h"
#include "Socket.h"

#ifndef _WIN32
#include <unistd.h>
#endif

namespace sakit
{
	hstr logTag = "sakit";
	float timeout = 10.00f;
	float retryFrequency = 0.01f;
	int bufferSize = 65536;
	harray<Base*> connections;
	hmutex connectionsMutex;
	hmutex updateMutex;
	hmap<unsigned int, hstr> mapping;
	/// @note Used for optimization to avoid hstr::fromUnicode() calls.
	hmap<hstr, hstr> reverseMapping;
	hthread* _updateThread;

	void _asyncUpdate(hthread* thread);
	void _internalUpdate(float timeDelta);

	bool isInitialized()
	{
		return mapping.size() > 0;
	}
	
	void init(bool threadedUpdate)
	{
		bufferSize = 65536;
		hlog::write(logTag, "Initializing Socket Abstraction Kit.");
		PlatformSocket::platformInit();
		// all 254 HTML entities as per HTML 4.0 specification
		mapping[0x22u] = "quot";
		mapping[0x26u] = "amp";
		mapping[0x27u] = "apos";
		mapping[0x3Cu] = "lt";
		mapping[0x3Eu] = "gt";
		mapping[0xA0u] = "nbsp";
		mapping[0xA1u] = "iexcl";
		mapping[0xA2u] = "cent";
		mapping[0xA3u] = "pound";
		mapping[0xA4u] = "curren";
		mapping[0xA5u] = "yen";
		mapping[0xA6u] = "brvbar";
		mapping[0xA7u] = "sect";
		mapping[0xA8u] = "uml";
		mapping[0xA9u] = "copy";
		mapping[0xAAu] = "ordf";
		mapping[0xABu] = "laquo";
		mapping[0xACu] = "not";
		mapping[0xADu] = "shy";
		mapping[0xAEu] = "reg";
		mapping[0xAFu] = "macr";
		mapping[0xB0u] = "deg";
		mapping[0xB1u] = "plusmn";
		mapping[0xB2u] = "sup2";
		mapping[0xB3u] = "sup3";
		mapping[0xB4u] = "acute";
		mapping[0xB5u] = "micro";
		mapping[0xB6u] = "para";
		mapping[0xB7u] = "middot";
		mapping[0xB8u] = "cedil";
		mapping[0xB9u] = "sup1";
		mapping[0xBAu] = "ordm";
		mapping[0xBBu] = "raquo";
		mapping[0xBCu] = "frac14";
		mapping[0xBDu] = "frac12";
		mapping[0xBEu] = "frac34";
		mapping[0xBFu] = "iquest";
		mapping[0xC0u] = "Agrave";
		mapping[0xC1u] = "Aacute";
		mapping[0xC2u] = "Acirc";
		mapping[0xC3u] = "Atilde";
		mapping[0xC4u] = "Auml";
		mapping[0xC5u] = "Aring";
		mapping[0xC6u] = "AElig";
		mapping[0xC7u] = "Ccedil";
		mapping[0xC8u] = "Egrave";
		mapping[0xC9u] = "Eacute";
		mapping[0xCAu] = "Ecirc";
		mapping[0xCBu] = "Euml";
		mapping[0xCCu] = "Igrave";
		mapping[0xCDu] = "Iacute";
		mapping[0xCEu] = "Icirc";
		mapping[0xCFu] = "Iuml";
		mapping[0xD0u] = "ETH";
		mapping[0xD1u] = "Ntilde";
		mapping[0xD2u] = "Ograve";
		mapping[0xD3u] = "Oacute";
		mapping[0xD4u] = "Ocirc";
		mapping[0xD5u] = "Otilde";
		mapping[0xD6u] = "Ouml";
		mapping[0xD7u] = "times";
		mapping[0xD8u] = "Oslash";
		mapping[0xD9u] = "Ugrave";
		mapping[0xDAu] = "Uacute";
		mapping[0xDBu] = "Ucirc";
		mapping[0xDCu] = "Uuml";
		mapping[0xDDu] = "Yacute";
		mapping[0xDEu] = "THORN";
		mapping[0xDFu] = "szlig";
		mapping[0xE0u] = "agrave";
		mapping[0xE1u] = "aacute";
		mapping[0xE2u] = "acirc";
		mapping[0xE3u] = "atilde";
		mapping[0xE4u] = "auml";
		mapping[0xE5u] = "aring";
		mapping[0xE6u] = "aelig";
		mapping[0xE7u] = "ccedil";
		mapping[0xE8u] = "egrave";
		mapping[0xE9u] = "eacute";
		mapping[0xEAu] = "ecirc";
		mapping[0xEBu] = "euml";
		mapping[0xECu] = "igrave";
		mapping[0xEDu] = "iacute";
		mapping[0xEEu] = "icirc";
		mapping[0xEFu] = "iuml";
		mapping[0xF0u] = "eth";
		mapping[0xF1u] = "ntilde";
		mapping[0xF2u] = "ograve";
		mapping[0xF3u] = "oacute";
		mapping[0xF4u] = "ocirc";
		mapping[0xF5u] = "otilde";
		mapping[0xF6u] = "ouml";
		mapping[0xF7u] = "divide";
		mapping[0xF8u] = "oslash";
		mapping[0xF9u] = "ugrave";
		mapping[0xFAu] = "uacute";
		mapping[0xFBu] = "ucirc";
		mapping[0xFCu] = "uuml";
		mapping[0xFDu] = "yacute";
		mapping[0xFEu] = "thorn";
		mapping[0xFFu] = "yuml";
		mapping[0x152u] = "OElig";
		mapping[0x153u] = "oelig";
		mapping[0x160u] = "Scaron";
		mapping[0x161u] = "scaron";
		mapping[0x178u] = "Yuml";
		mapping[0x192u] = "fnof";
		mapping[0x2C6u] = "circ";
		mapping[0x2DCu] = "tilde";
		mapping[0x391u] = "Alpha";
		mapping[0x392u] = "Beta";
		mapping[0x393u] = "Gamma";
		mapping[0x394u] = "Delta";
		mapping[0x395u] = "Epsilon";
		mapping[0x396u] = "Zeta";
		mapping[0x397u] = "Eta";
		mapping[0x398u] = "Theta";
		mapping[0x399u] = "Iota";
		mapping[0x39Au] = "Kappa";
		mapping[0x39Bu] = "Lambda";
		mapping[0x39Cu] = "Mu";
		mapping[0x39Du] = "Nu";
		mapping[0x39Eu] = "Xi";
		mapping[0x39Fu] = "Omicron";
		mapping[0x3A0u] = "Pi";
		mapping[0x3A1u] = "Rho";
		mapping[0x3A3u] = "Sigma";
		mapping[0x3A4u] = "Tau";
		mapping[0x3A5u] = "Upsilon";
		mapping[0x3A6u] = "Phi";
		mapping[0x3A7u] = "Chi";
		mapping[0x3A8u] = "Psi";
		mapping[0x3A9u] = "Omega";
		mapping[0x3B1u] = "alpha";
		mapping[0x3B2u] = "beta";
		mapping[0x3B3u] = "gamma";
		mapping[0x3B4u] = "delta";
		mapping[0x3B5u] = "epsilon";
		mapping[0x3B6u] = "zeta";
		mapping[0x3B7u] = "eta";
		mapping[0x3B8u] = "theta";
		mapping[0x3B9u] = "iota";
		mapping[0x3BAu] = "kappa";
		mapping[0x3BBu] = "lambda";
		mapping[0x3BCu] = "mu";
		mapping[0x3BDu] = "nu";
		mapping[0x3BEu] = "xi";
		mapping[0x3BFu] = "omicron";
		mapping[0x3C0u] = "pi";
		mapping[0x3C1u] = "rho";
		mapping[0x3C2u] = "sigmaf";
		mapping[0x3C3u] = "sigma";
		mapping[0x3C4u] = "tau";
		mapping[0x3C5u] = "upsilon";
		mapping[0x3C6u] = "phi";
		mapping[0x3C7u] = "chi";
		mapping[0x3C8u] = "psi";
		mapping[0x3C9u] = "omega";
		mapping[0x3D1u] = "thetasym";
		mapping[0x3D2u] = "upsih";
		mapping[0x3D6u] = "piv";
		mapping[0x2002u] = "ensp";
		mapping[0x2003u] = "emsp";
		mapping[0x2009u] = "thinsp";
		mapping[0x200Cu] = "zwnj";
		mapping[0x200Du] = "zwj";
		mapping[0x200Eu] = "lrm";
		mapping[0x200Fu] = "rlm";
		mapping[0x2013u] = "ndash";
		mapping[0x2014u] = "mdash";
		mapping[0x2018u] = "lsquo";
		mapping[0x2019u] = "rsquo";
		mapping[0x201Au] = "sbquo";
		mapping[0x201Cu] = "ldquo";
		mapping[0x201Du] = "rdquo";
		mapping[0x201Eu] = "bdquo";
		mapping[0x2020u] = "dagger";
		mapping[0x2021u] = "Dagger";
		mapping[0x2022u] = "bull";
		mapping[0x2026u] = "hellip";
		mapping[0x2030u] = "permil";
		mapping[0x2032u] = "prime";
		mapping[0x2033u] = "Prime";
		mapping[0x2039u] = "lsaquo";
		mapping[0x203Au] = "rsaquo";
		mapping[0x203Eu] = "oline";
		mapping[0x2044u] = "frasl";
		mapping[0x20ACu] = "euro";
		mapping[0x2111u] = "image";
		mapping[0x2118u] = "weierp";
		mapping[0x211Cu] = "real";
		mapping[0x2122u] = "trade";
		mapping[0x2135u] = "alefsym";
		mapping[0x2190u] = "larr";
		mapping[0x2191u] = "uarr";
		mapping[0x2192u] = "rarr";
		mapping[0x2193u] = "darr";
		mapping[0x2194u] = "harr";
		mapping[0x21B5u] = "crarr";
		mapping[0x21D0u] = "lArr";
		mapping[0x21D1u] = "uArr";
		mapping[0x21D2u] = "rArr";
		mapping[0x21D3u] = "dArr";
		mapping[0x21D4u] = "hArr";
		mapping[0x2200u] = "forall";
		mapping[0x2202u] = "part";
		mapping[0x2203u] = "exist";
		mapping[0x2205u] = "empty";
		mapping[0x2207u] = "nabla";
		mapping[0x2208u] = "isin";
		mapping[0x2209u] = "notin";
		mapping[0x220Bu] = "ni";
		mapping[0x220Fu] = "prod";
		mapping[0x2211u] = "sum";
		mapping[0x2212u] = "minus";
		mapping[0x2217u] = "lowast";
		mapping[0x221Au] = "radic";
		mapping[0x221Du] = "prop";
		mapping[0x221Eu] = "infin";
		mapping[0x2220u] = "ang";
		mapping[0x2227u] = "and";
		mapping[0x2228u] = "or";
		mapping[0x2229u] = "cap";
		mapping[0x222Au] = "cup";
		mapping[0x222Bu] = "int";
		mapping[0x2234u] = "there4";
		mapping[0x223Cu] = "sim";
		mapping[0x2245u] = "cong";
		mapping[0x2248u] = "asymp";
		mapping[0x2260u] = "ne";
		mapping[0x2261u] = "equiv";
		mapping[0x2264u] = "le";
		mapping[0x2265u] = "ge";
		mapping[0x2282u] = "sub";
		mapping[0x2283u] = "sup";
		mapping[0x2284u] = "nsub";
		mapping[0x2286u] = "sube";
		mapping[0x2287u] = "supe";
		mapping[0x2295u] = "oplus";
		mapping[0x2297u] = "otimes";
		mapping[0x22A5u] = "perp";
		mapping[0x22C5u] = "sdot";
		mapping[0x22EEu] = "vellip";
		mapping[0x2308u] = "lceil";
		mapping[0x2309u] = "rceil";
		mapping[0x230Au] = "lfloor";
		mapping[0x230Bu] = "rfloor";
		mapping[0x2329u] = "lang";
		mapping[0x232Au] = "rang";
		mapping[0x25CAu] = "loz";
		mapping[0x2660u] = "spades";
		mapping[0x2663u] = "clubs";
		mapping[0x2665u] = "hearts";
		mapping[0x2666u] = "diams";
		foreach_map (unsigned int, hstr, it, mapping)
		{
			reverseMapping[it->second] = hstr::fromUnicode(it->first);
		}
		if (threadedUpdate)
		{
			_updateThread = new hthread(&_asyncUpdate, "SAKit async update");
			_updateThread->start();
		}
	}
	
	hstr getHostName()
	{
#ifdef _WIN32
#ifndef _WINRT
		char buff[MAX_COMPUTERNAME_LENGTH + 1] = {0};
		DWORD size = MAX_COMPUTERNAME_LENGTH;		
		GetComputerNameA(buff, &size);
		return buff;
#else
		Windows::Foundation::Collections::IVectorView<HostName^>^ vector = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
		if (vector->Size == 0)
		{
			return "";
		}
		return _HL_PSTR_TO_HSTR(vector->First()->Current->DisplayName);
#endif
#else
		char buff[65] = {0};
		gethostname(buff, 64);
		return buff;
#endif
	}

	void destroy()
	{
		hlog::write(logTag, "Destroying Socket Abstraction Kit.");
		mapping.clear();
		if (_updateThread != NULL)
		{
			_updateThread->join();
			delete _updateThread;
			_updateThread = NULL;
		}
		PlatformSocket::platformDestroy();
		if (connections.size() > 0)
		{
			hlog::warn(logTag, "Not all sockets/servers have been destroyed!");
		}
	}

	void _internalUpdate(float timeDelta)
	{
		hmutex::ScopeLock lock(&connectionsMutex);
		harray<Base*> _connections = sakit::connections;
		lock.release();
		foreach (Base*, it, _connections)
		{
			(*it)->update(timeDelta);
		}
	}

	void update(float timeDelta)
	{
		if (_updateThread != NULL)
		{
			hlog::warn(logTag, "Calling update() does nothing when threaded update is active!");
			return;
		}
		_internalUpdate(timeDelta);
	}

	void _asyncUpdate(hthread* thread)
	{
		hmutex::ScopeLock lock;
		while (thread->isRunning())
		{
			lock.acquire(&updateMutex);
			_internalUpdate(0.001f);
			lock.release();
			hthread::sleep(1.0f);
		}
	}

	int getBufferSize()
	{
		return bufferSize;
	}

	void setBufferSize(int value)
	{
		bufferSize = value;
	}

	float getGlobalTimeout()
	{
		return timeout;
	}

	float getGlobalRetryFrequency()
	{
		return retryFrequency;
	}

	void setGlobalTimeout(float globalTimeout, float globalRetryFrequency)
	{
		timeout = globalTimeout;
		retryFrequency = hclamp(globalRetryFrequency, 0.000001f, globalTimeout); // frequency can't be larger than the timeout itself
	}

	harray<NetworkAdapter> getNetworkAdapters()
	{
		return PlatformSocket::getNetworkAdapters();
	}

	Host resolveHost(Host domain)
	{
		return PlatformSocket::resolveHost(domain);
	}

	Host resolveIp(Host ip)
	{
		return PlatformSocket::resolveIp(ip);
	}

	unsigned short resolveServiceName(chstr serviceName)
	{
		return PlatformSocket::resolveServiceName(serviceName);
	}

	hstr encodeHtmlEntities(chstr string)
	{
		harray<hstr> result; // adding stuff to an array is much faster than concatenating strings
		std::basic_string<unsigned int> chars = string.uStr();
		unsigned int start = 0;
		hstr entity;
		for_itert (unsigned int, i, 0, chars.size())
		{
			entity = mapping.tryGet(chars[i], "");
			if (entity != "")
			{
				result += hstr::fromUnicode(chars.substr(start, i - start).c_str()) + "&" + entity + ";";
				start = i + 1;
			}
			else if (chars[i] > 0xFF)
			{
				result += hstr::fromUnicode(chars.substr(start, i - start).c_str()) + hsprintf("&%d;", chars[i]);
				start = i + 1;
			}
		}
		if (start < chars.size())
		{
			result += hstr::fromUnicode(chars.substr(start, chars.size() - start).c_str());
		}
		return result.joined("");
	}

	hstr decodeHtmlEntities(chstr string)
	{
		harray<hstr> result; // adding stuff to an array is much faster than concatenating strings
		hstr entity;
		int start = 0;
		int index = 0;
		while (start <= string.size())
		{
			index = string.indexOf('&', start);
			if (index < 0)
			{
				break;
			}
			result += string(start, index - start);
			start = index + 1;
			index = string.indexOf(';', start);
			if (index < 0)
			{
				result += '&';
				break;
			}
			entity = string(start, index - start);
			start = index + 1;
			if (entity.startsWith('#'))
			{
				if (entity[1] != 'x')
				{
					result += hstr::fromUnicode((unsigned int)entity(1, -1));
				}
				else
				{
					result += hstr::fromUnicode(entity(2, -1).unhex());
				}
			}
			else if (reverseMapping.hasKey(entity))
			{
				result += reverseMapping[entity];
			}
			else
			{
				result += "&" + entity + ";"; // not decoded
			}
		}
		result += string(start, -1);
		return result.joined("");
	}

}
