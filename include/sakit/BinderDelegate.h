/// @file
/// @version 1.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a binder delegate.

#ifndef SAKIT_BINDER_DELEGATE_H
#define SAKIT_BINDER_DELEGATE_H

#include "Host.h"
#include "sakitExport.h"

namespace sakit
{
	class Binder;

	class sakitExport BinderDelegate
	{
	public:
		BinderDelegate();
		virtual ~BinderDelegate();

		virtual void onBound(Binder* binder, Host localHost, unsigned short localPort);
		virtual void onBindFailed(Binder* binder, Host localHost, unsigned short localPort);
		virtual void onUnbound(Binder* binder, Host localHost, unsigned short localPort);
		virtual void onUnbindFailed(Binder* binder, Host localHost, unsigned short localPort);

	};

}
#endif
