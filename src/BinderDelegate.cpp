/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "BinderDelegate.h"

namespace sakit
{
	BinderDelegate::BinderDelegate()
	{
	}

	BinderDelegate::~BinderDelegate()
	{
	}

	void BinderDelegate::onBound(Binder* binder, Host localHost, unsigned short localPort)
	{
	}

	void BinderDelegate::onBindFailed(Binder* binder, Host localHost, unsigned short localPort)
	{
	}

	void BinderDelegate::onUnbound(Binder* binder, Host localHost, unsigned short localPort)
	{
	}

	void BinderDelegate::onUnbindFailed(Binder* binder, Host localHost, unsigned short localPort)
	{
	}

}
