idf_component_register(SRCS "i2cdev.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver
                    REQUIRES esp_idf_lib_helpers)

