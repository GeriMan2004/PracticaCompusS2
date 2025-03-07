include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(Practica2FA_default_library_list )

# Handle files with suffix (s|as|asm|AS|ASM|As|aS|Asm), for group default-XC8
if(Practica2FA_default_default_XC8_FILE_TYPE_assemble)
add_library(Practica2FA_default_default_XC8_assemble OBJECT ${Practica2FA_default_default_XC8_FILE_TYPE_assemble})
    Practica2FA_default_default_XC8_assemble_rule(Practica2FA_default_default_XC8_assemble)
    list(APPEND Practica2FA_default_library_list "$<TARGET_OBJECTS:Practica2FA_default_default_XC8_assemble>")
endif()

# Handle files with suffix S, for group default-XC8
if(Practica2FA_default_default_XC8_FILE_TYPE_assemblePreprocess)
add_library(Practica2FA_default_default_XC8_assemblePreprocess OBJECT ${Practica2FA_default_default_XC8_FILE_TYPE_assemblePreprocess})
    Practica2FA_default_default_XC8_assemblePreprocess_rule(Practica2FA_default_default_XC8_assemblePreprocess)
    list(APPEND Practica2FA_default_library_list "$<TARGET_OBJECTS:Practica2FA_default_default_XC8_assemblePreprocess>")
endif()

# Handle files with suffix [cC], for group default-XC8
if(Practica2FA_default_default_XC8_FILE_TYPE_compile)
add_library(Practica2FA_default_default_XC8_compile OBJECT ${Practica2FA_default_default_XC8_FILE_TYPE_compile})
    Practica2FA_default_default_XC8_compile_rule(Practica2FA_default_default_XC8_compile)
    list(APPEND Practica2FA_default_library_list "$<TARGET_OBJECTS:Practica2FA_default_default_XC8_compile>")
endif()

add_executable(${Practica2FA_default_image_name} ${Practica2FA_default_library_list})

target_link_libraries(${Practica2FA_default_image_name} PRIVATE ${Practica2FA_default_default_XC8_FILE_TYPE_link})

# Add the link options from the rule file.
Practica2FA_default_link_rule(${Practica2FA_default_image_name})


# Post build target to copy built file to the output directory.
add_custom_command(TARGET ${Practica2FA_default_image_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${Practica2FA_default_output_dir}
                    COMMAND ${CMAKE_COMMAND} -E copy ${Practica2FA_default_image_name} ${Practica2FA_default_output_dir}/${Practica2FA_default_original_image_name}
                    BYPRODUCTS ${Practica2FA_default_output_dir}/${Practica2FA_default_original_image_name})
