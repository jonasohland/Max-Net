macro(get_version_tag)

    if(use_version_tags)

        execute_process(COMMAND "git" "describe" "--tags" 
                        OUTPUT_VARIABLE VERSION_TAG 
                        ERROR_VARIABLE VERSION_TAG
                        ERROR_STRIP_TRAILING_WHITESPACE
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
                    
    else()

        set(VERSION_TAG "x.x")

    endif()

endmacro(get_version_tag)