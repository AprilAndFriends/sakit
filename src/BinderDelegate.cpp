/// @file
/// @author  Boris Mikic
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "BinderDelegate.h"

namespace sakit
{
	BinderDelegate::BinderDelegate()
	{
	}

	BinderDelegate::~BinderDelegate()
	{
	}

	void BinderDelegate::onBound(Binder* binder, Host host, unsigned short port)
	{
	}

	void BinderDelegate::onBindFailed(Binder* binder, Host host, unsigned short port)
	{
	}

	void BinderDelegate::onUnbound(Binder* binder, Host host, unsigned short port)
	{
	}

	void BinderDelegate::onUnbindFailed(Binder* binder, Host host, unsigned short port)
	{
	}

}
