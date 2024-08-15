//
// Created by arshia on 8/12/24.
//

#include "module.h"


namespace reactaio {
	module::submodules_t module::submodules() const {
		return {};
	}

	module::dependency_t module::dependencies() const {
		return {};
	}

	void module::reload() {
		stop();
		start();
	}
}
