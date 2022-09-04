idf_component_register(SRCS "blufi_example_main.c"
                            "blufi_security.c"
                            "blufi_init.c""kvdb_basic_sample.c""kvdb_type_blob_sample.c""kvdb_type_string_sample.c""tsdb_sample.c""../../../FlashDB-master/src/fdb.c" "../../../FlashDB-master/src/fdb_file.c" "../../../FlashDB-master/src/fdb_kvdb.c" "../../../FlashDB-master/src/fdb_tsdb.c" "../../../FlashDB-master/src/fdb_utils.c""../../../FlashDB-master/port/fal/src/fal_flash.c" "../../../FlashDB-master/port/fal/src/fal_partition.c" "../../../FlashDB-master/port/fal/src/fal_rtt.c" "../../../FlashDB-master/port/fal/src/fal.c"
                                       INCLUDE_DIRS ".""../../../FlashDB-master/port/fal/inc""../components/FlashDB/inc" "../../../FlashDB-master/inc" 
                   
              )
