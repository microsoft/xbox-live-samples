//--------------------------------------------------------------------------------------
// AsyncOperationPromiseType.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <winrt\Windows.Foundation.h>

template <typename T, typename... Arguments>
struct coroutine_traits<winrt::Windows::Foundation::IAsyncOperation<T>, Arguments...>
{
	struct promise_type
	{
		promise<T> _promise;
		winrt::Windows::Foundation::IAsyncOperation<T> get_return_object() { return _promise.get_future(); }

		auto initial_suspend() { return suspend_never{}; }
		auto final_suspend() { return suspend_always{}; }

		template<typename U>
		void return_value(U&& value) { _promise.set_value(std::forward<U>(value)); }

		void set_exception(winrt::hresult_error ex) { _promise.set_exception(std::move(ex)); }
	};
};
