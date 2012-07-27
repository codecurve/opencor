#ifdef FRONTENDSTART
__FRONTENDSTART = DIAG_START_FRONTEND,
#undef FRONTENDSTART
#endif

DIAG(err_fe_cannot_link_module, CLASS_ERROR, diag::MAP_FATAL, "cannot link module '%0': %1", 0, true, false, false, false, 0)
DIAG(err_fe_dependency_file_requires_MT, CLASS_ERROR, diag::MAP_ERROR, "-dependency-file requires at least one -MT or -MQ option", 0, true, false, false, false, 0)
DIAG(err_fe_error_backend, CLASS_ERROR, diag::MAP_FATAL, "error in backend: %0", 0, true, false, false, false, 0)
DIAG(err_fe_error_opening, CLASS_ERROR, diag::MAP_ERROR, "error opening '%0': %1", 0, true, false, false, false, 0)
DIAG(err_fe_error_reading, CLASS_ERROR, diag::MAP_ERROR, "error reading '%0'", 0, true, false, false, false, 0)
DIAG(err_fe_error_reading_stdin, CLASS_ERROR, diag::MAP_ERROR, "error reading stdin", 0, true, false, false, false, 0)
DIAG(err_fe_expected_clang_command, CLASS_ERROR, diag::MAP_ERROR, "expected a clang compiler command", 0, true, false, false, false, 0)
DIAG(err_fe_expected_compiler_job, CLASS_ERROR, diag::MAP_ERROR, "unable to handle compilation, expected exactly one compiler job in '%0'", 0, true, false, false, false, 0)
DIAG(err_fe_inline_asm, CLASS_ERROR, diag::MAP_ERROR, "%0", 0, true, false, false, false, 11)
DIAG(err_fe_invalid_code_complete_file, CLASS_ERROR, diag::MAP_FATAL, "cannot locate code-completion file %0", 0, true, false, false, false, 0)
DIAG(err_fe_invalid_plugin_name, CLASS_ERROR, diag::MAP_ERROR, "unable to find plugin '%0'", 0, true, false, false, false, 0)
DIAG(err_fe_pth_file_has_no_source_header, CLASS_ERROR, diag::MAP_ERROR, "PTH file '%0' does not designate an original source header file for -include-pth", 0, true, false, false, false, 0)
DIAG(err_fe_remap_missing_from_file, CLASS_ERROR, diag::MAP_FATAL, "could not remap from missing file '%0'", 0, true, false, false, false, 0)
DIAG(err_fe_remap_missing_to_file, CLASS_ERROR, diag::MAP_FATAL, "could not remap file '%0' to the contents of file '%1'", 0, true, false, false, false, 0)
DIAG(err_fe_stdout_binary, CLASS_ERROR, diag::MAP_FATAL, "unable to change standard output to binary", 0, true, false, false, false, 0)
DIAG(err_fe_unable_to_create_target, CLASS_ERROR, diag::MAP_ERROR, "unable to create target: '%0'", 0, true, false, false, false, 0)
DIAG(err_fe_unable_to_interface_with_target, CLASS_ERROR, diag::MAP_ERROR, "unable to interface with target machine", 0, true, false, false, false, 0)
DIAG(err_fe_unable_to_load_pch, CLASS_ERROR, diag::MAP_ERROR, "unable to load PCH file", 0, true, false, false, false, 0)
DIAG(err_fe_unable_to_load_plugin, CLASS_ERROR, diag::MAP_ERROR, "unable to load plugin '%0': '%1'", 0, true, false, false, false, 0)
DIAG(err_fe_unable_to_open_logfile, CLASS_ERROR, diag::MAP_ERROR, "unable to open logfile file '%0': '%1'", 0, true, false, false, false, 0)
DIAG(err_fe_unable_to_open_output, CLASS_ERROR, diag::MAP_ERROR, "unable to open output file '%0': '%1'", 0, true, false, false, false, 0)
DIAG(err_fe_unable_to_rename_temp, CLASS_ERROR, diag::MAP_ERROR, "unable to rename temporary '%0' to output file '%1': '%2'", 0, true, false, false, false, 0)
DIAG(err_missing_module, CLASS_ERROR, diag::MAP_FATAL, "no module named '%0' declared in module map file '%1'", 0, true, false, false, false, 0)
DIAG(err_missing_module_name, CLASS_ERROR, diag::MAP_FATAL, "no module name provided; specify one with -fmodule-name=", 0, true, false, false, false, 0)
DIAG(err_missing_umbrella_header, CLASS_ERROR, diag::MAP_FATAL, "cannot open umbrella header '%0': %1", 0, true, false, false, false, 0)
DIAG(err_module_map_not_found, CLASS_ERROR, diag::MAP_FATAL, "module map file '%0' not found", 0, true, false, false, false, 0)
DIAG(err_module_map_temp_file, CLASS_ERROR, diag::MAP_FATAL, "unable to write temporary module map file '%0'", 0, true, false, false, false, 0)
DIAG(err_module_unavailable, CLASS_ERROR, diag::MAP_ERROR, "module '%0' requires feature '%1'", 0, true, false, false, false, 0)
DIAG(err_no_submodule, CLASS_ERROR, diag::MAP_ERROR, "no submodule named %0 in module '%1'", 0, true, false, false, false, 0)
DIAG(err_no_submodule_suggest, CLASS_ERROR, diag::MAP_ERROR, "no submodule named %0 in module '%1'; did you mean '%2'?", 0, true, false, false, false, 0)
DIAG(err_relocatable_without_isysroot, CLASS_ERROR, diag::MAP_ERROR, "must specify system root with -isysroot when building a relocatable PCH file", 0, true, false, false, false, 0)
DIAG(err_verify_inconsistent_diags, CLASS_ERROR, diag::MAP_ERROR, "'%0' diagnostics %select{expected|seen}1 but not %select{seen|expected}1: %2", 0, true, false, false, false, 0)
DIAG(err_verify_invalid_content, CLASS_ERROR, diag::MAP_ERROR, "invalid expected %0: %1", 0, true, false, false, false, 0)
DIAG(err_verify_missing_end, CLASS_ERROR, diag::MAP_ERROR, "cannot find end ('}}') of expected %0", 0, true, false, false, false, 0)
DIAG(err_verify_missing_start, CLASS_ERROR, diag::MAP_ERROR, "cannot find start ('{{') of expected %0", 0, true, false, false, false, 0)
DIAG(note_fe_inline_asm_here, CLASS_NOTE, diag::MAP_FATAL, "instantiated into assembly here", 0, true, false, false, false, 0)
DIAG(note_fixit_applied, CLASS_NOTE, diag::MAP_FATAL, "FIX-IT applied suggested code changes", 0, true, false, false, false, 0)
DIAG(note_fixit_failed, CLASS_NOTE, diag::MAP_FATAL, "FIX-IT unable to apply suggested code changes", 0, true, false, false, false, 0)
DIAG(note_fixit_in_macro, CLASS_NOTE, diag::MAP_FATAL, "FIX-IT unable to apply suggested code changes in a macro", 0, true, false, false, false, 0)
DIAG(note_fixit_unfixed_error, CLASS_NOTE, diag::MAP_FATAL, "FIX-IT detected an error it cannot fix", 0, true, false, false, false, 0)
DIAG(note_incompatible_analyzer_plugin_api, CLASS_NOTE, diag::MAP_FATAL, "current API version is '%0', but plugin was compiled with version '%1'", 0, true, false, false, false, 0)
DIAG(warn_fe_cc_log_diagnostics_failure, CLASS_WARNING, diag::MAP_WARNING, "unable to open CC_LOG_DIAGNOSTICS file: %0 (using stderr)", 0, true, false, false, false, 0)
DIAG(warn_fe_cc_print_header_failure, CLASS_WARNING, diag::MAP_WARNING, "unable to open CC_PRINT_HEADERS file: %0 (using stderr)", 0, true, false, false, false, 0)
DIAG(warn_fe_macro_contains_embedded_newline, CLASS_WARNING, diag::MAP_WARNING, "macro '%0' contains embedded newline, text after the newline is ignored.", 0, true, false, false, false, 0)
DIAG(warn_fe_serialized_diag_failure, CLASS_WARNING, diag::MAP_WARNING, "unable to open file %0 for serializing diagnostics (%1)", 227, true, false, false, false, 0)
DIAG(warn_fixit_no_changes, CLASS_NOTE, diag::MAP_FATAL, "FIX-IT detected errors it could not fix; no output will be generated", 0, true, false, false, false, 0)
DIAG(warn_incompatible_analyzer_plugin_api, CLASS_WARNING, diag::MAP_WARNING, "checker plugin '%0' is not compatible with this version of the analyzer", 11, true, false, false, false, 0)
DIAG(warn_missing_submodule, CLASS_WARNING, diag::MAP_WARNING, "missing submodule '%0'", 127, true, false, false, false, 0)
DIAG(warn_unknown_analyzer_checker, CLASS_WARNING, diag::MAP_WARNING, "no analyzer checkers are associated with '%0'", 0, true, false, false, false, 0)
DIAG(warn_unknown_negative_warning_option, CLASS_WARNING, diag::MAP_IGNORE, "unknown warning option '%0'?", 278, true, false, false, false, 0)
DIAG(warn_unknown_negative_warning_option_suggest, CLASS_WARNING, diag::MAP_IGNORE, "unknown warning option '%0'; did you mean '%1'?", 278, true, false, false, false, 0)
DIAG(warn_unknown_warning_option, CLASS_WARNING, diag::MAP_WARNING, "unknown warning option '%0'", 278, true, false, false, false, 0)
DIAG(warn_unknown_warning_option_suggest, CLASS_WARNING, diag::MAP_WARNING, "unknown warning option '%0'; did you mean '%1'?", 278, true, false, false, false, 0)
DIAG(warn_unknown_warning_specifier, CLASS_WARNING, diag::MAP_WARNING, "unknown %0 warning specifier: '%1'", 278, true, false, false, false, 0)