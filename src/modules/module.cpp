//
// Created by arshia on 8/12/24.
//

#include "module.h"


namespace reactaio {
	module::submodules_t reactaio::module::submodules() const {
		return {};
	}

	void module::reload() {
		stop();
		start();
	}
}
