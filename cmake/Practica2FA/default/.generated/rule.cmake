# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/gerar/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(Practica2FA_default_default_XC8_assemble_rule target)
    set(options
        "-c"
        "${MP_EXTRA_AS_PRE}"
        "-mcpu=18F4321"
        "-fno-short-double"
        "-fno-short-float"
        "-memi=wordwrite"
        "-O0"
        "-fasmfile"
        "-maddrqual=ignore"
        "-mwarn=-3"
        "-Wa,-a"
        "-msummary=-psect,-class,+mem,-hex,-file"
        "-ginhx32"
        "-Wl,--data-init"
        "-mno-keep-startup"
        "-mno-download"
        "-mdefault-config-bits"
        "-std=c99"
        "-gdwarf-3"
        "-mstack=compiled:auto:auto:auto")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__18F4321__"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
function(Practica2FA_default_default_XC8_assemblePreprocess_rule target)
    set(options
        "-c"
        "${MP_EXTRA_AS_PRE}"
        "-mcpu=18F4321"
        "-x"
        "assembler-with-cpp"
        "-fno-short-double"
        "-fno-short-float"
        "-memi=wordwrite"
        "-O0"
        "-fasmfile"
        "-maddrqual=ignore"
        "-mwarn=-3"
        "-Wa,-a"
        "-msummary=-psect,-class,+mem,-hex,-file"
        "-ginhx32"
        "-Wl,--data-init"
        "-mno-keep-startup"
        "-mno-download"
        "-mdefault-config-bits"
        "-std=c99"
        "-gdwarf-3"
        "-mstack=compiled:auto:auto:auto")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__18F4321__"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
function(Practica2FA_default_default_XC8_compile_rule target)
    set(options
        "-c"
        "${MP_EXTRA_CC_PRE}"
        "-mcpu=18F4321"
        "-fno-short-double"
        "-fno-short-float"
        "-memi=wordwrite"
        "-O0"
        "-fasmfile"
        "-maddrqual=ignore"
        "-mwarn=-3"
        "-Wa,-a"
        "-msummary=-psect,-class,+mem,-hex,-file"
        "-ginhx32"
        "-Wl,--data-init"
        "-mno-keep-startup"
        "-mno-download"
        "-mdefault-config-bits"
        "-std=c99"
        "-gdwarf-3"
        "-mstack=compiled:auto:auto:auto")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__18F4321__"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
function(Practica2FA_default_link_rule target)
    set(options
        "-Wl,-Map=mem.map"
        "${MP_EXTRA_LD_PRE}"
        "-mcpu=18F4321"
        "-Wl,--defsym=__MPLAB_BUILD=1"
        "-fno-short-double"
        "-fno-short-float"
        "-memi=wordwrite"
        "-O0"
        "-fasmfile"
        "-maddrqual=ignore"
        "-mwarn=-3"
        "-Wa,-a"
        "-msummary=-psect,-class,+mem,-hex,-file"
        "-ginhx32"
        "-Wl,--data-init"
        "-mno-keep-startup"
        "-mno-download"
        "-mdefault-config-bits"
        "-std=c99"
        "-gdwarf-3"
        "-mstack=compiled:auto:auto:auto"
        "-Wl,--memorysummary,memoryfile.xml")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
