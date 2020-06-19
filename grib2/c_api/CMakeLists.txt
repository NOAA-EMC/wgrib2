

set(c_api_src
  grb2_mk_inv.c
  grb2_cmd.c
  grb2_inq.c
  )

add_library(wgrib2_api OBJECT ${c_api_src})
