cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(picocalc_sd_boot C CXX ASM)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR} CACHE PATH "Install prefix set to project root" FORCE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(i2ckbd)
add_subdirectory(lcdspi)
add_subdirectory(lib/pico-vfs)

# add_subdirectory(lib/tinyusb/src)
add_definitions(-DCFG_TUSB_CONFIG_FILE="tusb_config.h")



add_executable(picocalc_sd_boot_${board_name}
  main.c
  key_event.c
  text_directory_ui.c
)

target_sources(picocalc_sd_boot_${board_name} PRIVATE
  usb_msc/usb_msc.c
  usb_msc/usb_descriptors.c
)

# ${CMAKE_CURRENT_LIST_DIR}/lib/tinyusb/src
target_include_directories(picocalc_sd_boot_${board_name} PRIVATE
${CMAKE_CURRENT_LIST_DIR}/usb_msc
)

target_link_libraries(picocalc_sd_boot_${board_name} PUBLIC
  pico_stdlib
  hardware_sync
  hardware_flash
  hardware_irq
  hardware_adc
  hardware_pwm
  hardware_i2c
  hardware_spi
  hardware_dma
  hardware_exception
  hardware_pio
  pico_multicore
  i2ckbd
  lcdspi
  blockdevice_sd
  filesystem_fat
  filesystem_vfs
  pico_unique_id
  tinyusb_device
  tinyusb_board
  )
  # pico_usb_device

  # target_link_libraries(dev_hid_composite PUBLIC pico_stdlib pico_unique_id tinyusb_device tinyusb_board)


# pico_enable_stdio_usb(picocalc_sd_boot_${board_name} 1)
pico_enable_stdio_uart(picocalc_sd_boot_${board_name} 0)

pico_add_extra_outputs(picocalc_sd_boot_${board_name})

target_link_options(picocalc_sd_boot_${board_name} PRIVATE -Wl,--print-memory-usage)

# Define the output directory relative to the project root directory
set(output_dir prebuild_output/${board_name})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/picocalc_sd_boot_${board_name}.elf
  DESTINATION ${output_dir}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/picocalc_sd_boot_${board_name}.uf2
  DESTINATION ${output_dir}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/picocalc_sd_boot_${board_name}.bin
  DESTINATION ${output_dir}
)


# This function sets the custom linker script for SD card applications
# The memmap_sdcard_app.ld script configures the application to start at
# FLASH origin = 0x10000000 + 256k, which aligns with the SD_BOOT_FLASH_OFFSET
# defined in main.c (256 * 1024). This ensures the bootloader (first 256KB)
# is preserved when flashing the application.
function(enable_sdcard_app target)
  #pico_set_linker_script(${target} ${CMAKE_SOURCE_DIR}/memmap_sdcard_app.ld)
  if(${PICO_PLATFORM} STREQUAL "rp2040")
    pico_set_linker_script(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/memmap_default_rp2040.ld)
  elseif(${PICO_PLATFORM} MATCHES "rp2350")
    pico_set_linker_script(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/memmap_default_rp2350.ld)
  endif()
endfunction()
