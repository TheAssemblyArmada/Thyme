# Work out what macro provides best function names.
if(NOT CURRENT_FUNCTION)
    include(CheckSymbolExists)
    # Available on GCC like compilers
    check_symbol_exists(__PRETTY_FUNCTION__ "" HAVE_PRETTY_FUNCTION_MACRO)
    if(NOT HAVE_PRETTY_FUNCTION_MACRO)
        # Available on MSVC
        check_symbol_exists(__FUNCSIG__ "" HAVE_FUNCSIG_MACRO)
        if(NOT HAVE_FUNCSIG_MACRO)
            # Available on Watcom and others
            check_symbol_exists(__FUNCTION__ "" HAVE_FUNCTION_MACRO)
            if(NOT HAVE_FUNCTION_MACRO)
                # Part of the standard so should exist.
                check_symbol_exists(__func__ "" HAVE_FUNC_MACRO)
                if(NOT HAVE_FUNCTION_MACRO)
                    message(FATAL_ERROR "No function name macro found, please report!")
                else()
                    set(CURRENT_FUNCTION __func__)
                endif()
            else()
                set(CURRENT_FUNCTION __FUNCTION__)
            endif()
        else()
            set(CURRENT_FUNCTION __FUNCSIG__)
        endif()
    else()
        set(CURRENT_FUNCTION __PRETTY_FUNCTION__)
    endif()
endif()

# Defines __CURRENT_FUNCTION__ to preferred compiler specific macro for target.
function(target_current_function_define target)
    target_compile_definitions(${target} PRIVATE __CURRENT_FUNCTION__=${CURRENT_FUNCTION})
endfunction()