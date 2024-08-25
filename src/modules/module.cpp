//
// Created by arshia on 8/12/24.
//

#include "module.h"


namespace reactaio {
	module::module_map module::submodules() const {
		return {};
	}

	module::dependency_list module::dependencies() const {
		return {};
	}

	void module::reload() {
		stop();
		start();
	}
}
