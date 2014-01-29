/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
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

		virtual void onBound(Binder* binder, Host host, unsigned short port) = 0;
		virtual void onBindFailed(Binder* binder, Host host, unsigned short port) = 0;
		virtual void onUnbound(Binder* binder, Host host, unsigned short port) = 0;
		virtual void onUnbindFailed(Binder* binder, Host host, unsigned short port) = 0;

	};

}
#endif
