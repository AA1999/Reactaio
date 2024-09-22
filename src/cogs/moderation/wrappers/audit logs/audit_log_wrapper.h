//
// Created by arshia on 3/4/24.
//

#pragma once

#include "../recursive_wrapper.h"

#include <vector>
#include <dpp/dpp.h>

/**
 * @brief Functor wrapper for audit logs management.
 */
class audit_log_wrapper final: public recursive_wrapper {
	shared_vector<dpp::audit_entry> audit_entries_;

	/**
	 * 	@brief Checks if the user issuing the wrapper has the sufficient permission.
	 */
	void check_permissions() override;

	/**
	 * 	@brief The main function that manages every internal working of the wrapper and the three processes that are performed.
	 */
	void wrapper_function() override;

	/**
	 * @brief Captures the audit logs for the guild.
	 */
	void recursive_call(dpp::snowflake after = 1) override;

	/**
	 * @brief Sends the resulting response to the wrapper message object as embed(s).
	 */
	void process_response();

public:
	audit_log_wrapper() = delete;
	~audit_log_wrapper() override = default;
	using recursive_wrapper::recursive_wrapper;
};