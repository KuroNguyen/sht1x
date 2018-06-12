deps_config := \
	/home/kuro/esp/esp-idf/components/app_trace/Kconfig \
	/home/kuro/esp/esp-idf/components/aws_iot/Kconfig \
	/home/kuro/esp/esp-idf/components/bt/Kconfig \
	/home/kuro/esp/esp-idf/components/esp32/Kconfig \
	/home/kuro/esp/esp-idf/components/ethernet/Kconfig \
	/home/kuro/esp/esp-idf/components/fatfs/Kconfig \
	/home/kuro/esp/esp-idf/components/freertos/Kconfig \
	/home/kuro/esp/esp-idf/components/heap/Kconfig \
	/home/kuro/esp/esp-idf/components/libsodium/Kconfig \
	/home/kuro/esp/esp-idf/components/log/Kconfig \
	/home/kuro/esp/esp-idf/components/lwip/Kconfig \
	/home/kuro/esp/esp-idf/components/mbedtls/Kconfig \
	/home/kuro/esp/esp-idf/components/openssl/Kconfig \
	/home/kuro/esp/esp-idf/components/pthread/Kconfig \
	/home/kuro/esp/esp-idf/components/spi_flash/Kconfig \
	/home/kuro/esp/esp-idf/components/spiffs/Kconfig \
	/home/kuro/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/kuro/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/kuro/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/kuro/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/kuro/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/kuro/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
