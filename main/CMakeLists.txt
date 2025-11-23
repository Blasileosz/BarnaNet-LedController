# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)

idf_component_register(
	SRCS ${app_sources}
	# https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html#embedding-binary-data
	EMBED_TXTFILES ../certs/DigiCertGlobalRootG2.crt.pem ../certs/BB0.crt ../certs/BB0.key
)
