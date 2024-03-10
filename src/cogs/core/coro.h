//
// Created by arshia on 3/9/24.
//

#pragma once

#include <dpp/dpp.h>
#include <coroutine>
#include <stdexcept>

namespace reactaio::coro {
	struct coro_return {
		struct promise_t {
			coro_return get_return_object() {
				return {};
			}

			std::suspend_never initial_suspend() {
				return {};
			}

			
		};
	};
}