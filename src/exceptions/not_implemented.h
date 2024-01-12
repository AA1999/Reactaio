//
// Created by arshia on 2/22/23.
//

#pragma once

#include <stdexcept>

class not_implemented: std::logic_error {
public:
	not_implemented() : std::logic_error("Function not implemented for the base class."){}
	[[nodiscard]] const char* what() const noexcept override;
};
