REM Firmware
esptool.exe --chip esp32 --port COM16 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_qio_80m.bin 0x10000 AfterburnerV3.1.5.bin 0x8000 Afterburner.partitions.bin
REM SPIFFS
esptool.exe --chip esp32 --port COM16 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_size detect 0x3d0000 spiffs.bin
 
