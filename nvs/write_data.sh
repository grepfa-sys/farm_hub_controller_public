python ~/sdk/esp-idf/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate nvs.csv nvs_part.bin 16384
parttool.py --port /dev/ttyUSB0 write_partition --partition-type=data --partition-subtype=nvs --input "nvs_part.bin"