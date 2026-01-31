@page all_commands Command Line Options
| Command | Type | Arguments | Description |
|---------|------|-----------|-------------|
| [-else](@ref f_else) | else | | else, -if ... -else ... -endif |
| [-elseif](@ref f_if) | elif | X | elseif X (POSIX regular expression) conditional on match, -if ... -elseif ... -endif |
| [-elseif_fs](@ref f_if_fs) | elif | X | elseif X (fixed string) conditional execution |
| [-elseif_n](@ref f_if_n) | elif | X | elseif (inv numbers in range), X=(start:end:step) |
| [-elseif_rec](@ref f_if_rec) | elif | X | elseif (record numbers in range), X=(start:end:step) |
| [-elseif_reg](@ref f_if_reg) | elif | X | elseif rpn registers defined, X = A, A:B, A:B:C, etc A = register number |
| [-endif](@ref f_endif) | endif |  | terminates if block |
| [-if](@ref f_if) | if | X | if X (POSIX regular expression), conditional execution on match |
| [-if_delayed_error](@ref f_if_delayed_error) | if |  | if delayed error |
| [-if_fs](@ref f_if_fs) | if | X | if X (fixed string), conditional execution on match |
| [-if_n](@ref f_if_n) | if | X | if (inv numbers in range), X=(start:end:step) |
| [-if_rec](@ref f_if_rec) | if | X | if (record numbers in range), X=(start:end:step) |
| [-if_reg](@ref f_if_reg) | if | X | if rpn registers defined, X = A, A:B, A:B:C, etc A = register number |
| [-not_if](@ref f_not_if) | if | X | not_if X (regular expression), conditional execution on not match |
| [-not_if_fs](@ref f_not_if_fs) | if | X | if X (fixed string) does not match, conditional execution up to next output/fi |
| [-0xSec](@ref f_0xSec) | inv | X | Hex dump of section X (0..8) |
| [-aerosol_size](@ref f_aerosol_size) | inv |  | optical properties of an aerosol |
| [-aerosol_wavelength](@ref f_aerosol_wavelength) | inv |  | optical properties of an aerosol |
| [-bitmap](@ref f_bitmap) | inv |  | bitmap mode |
| [-center](@ref f_center) | inv |  | center |
| [-checksum](@ref f_checksum) | inv | X | CRC checksum of section X (0..8), whole message (X = -1/message) or (X=data) |
| [-cluster](@ref f_cluster) | inv |  | cluster identifier |
| [-cluster_info](@ref f_cluster_info) | inv |  | cluster information |
| [-code_table_0.0](@ref f_code_table_0_0) | inv |  | code table 0.0 discipline |
| [-code_table_1.0](@ref f_code_table_1_0) | inv |  | code table 1.0 master table version |
| [-code_table_1.1](@ref f_code_table_1_1) | inv |  | code table 1.1 local table version |
| [-code_table_1.2](@ref f_code_table_1_2) | inv |  | code table 1.2 significance of reference time |
| [-code_table_1.3](@ref f_code_table_1_3) | inv |  | code table 1.3 production status of processed data |
| [-code_table_1.4](@ref f_code_table_1_4) | inv |  | code table 1.4 type of processed data |
| [-code_table_1.5](@ref f_code_table_1_5) | inv |  | Identification template number |
| [-code_table_1.6](@ref f_code_table_1_6) | inv |  | calendar |
| [-code_table_3.0](@ref f_code_table_3_0) | inv |  | code table 3.0 Source of grid definition |
| [-code_table_3.1](@ref f_code_table_3_1) | inv |  | code table 3.1 Grid definition template number |
| [-code_table_3.11](@ref f_code_table_3_11) | inv |  | code table 3.11 regional/global thinned/reduced grid |
| [-code_table_3.15](@ref f_code_table_3_15) | inv |  | code table 3.15 Physical meaning of vertical coordinate |
| [-code_table_3.2](@ref f_code_table_3_2) | inv |  | code table 3.2 Size (radius) and Shape of Earth |
| [-code_table_3.20](@ref f_code_table_3_20) | inv |  | code table 3.20 Type of Horizontal line |
| [-code_table_3.21](@ref f_code_table_3_21) | inv |  | code table 3.21 Vertical Dimension coordinate values defn |
| [-code_table_3.6](@ref f_code_table_3_6) | inv |  | code table 3.6 Spectral data representation type |
| [-code_table_3.7](@ref f_code_table_3_7) | inv |  | code table 3.7 Spectral data representation mode |
| [-code_table_3.8](@ref f_code_table_3_8) | inv |  | code table 3.8 Grid point position |
| [-code_table_4.0](@ref f_code_table_4_0) | inv |  | code table 4.0 Product Definition Template Number |
| [-code_table_4.1](@ref f_code_table_4_1) | inv |  | code table 4.1 |
| [-code_table_4.10](@ref f_code_table_4_10) | inv |  | code table 4.10 statistical processing .. first occurence |
| [-code_table_4.11](@ref f_code_table_4_11) | inv |  | code table 4.11 (first) type of time intervals |
| [-code_table_4.11s](@ref f_code_table_4_11s) | inv |  | code table 4.11 (all) type of time intervals |
| [-code_table_4.15](@ref f_code_table_4_15) | inv |  | code table 4.15 type of areal statistical processing |
| [-code_table_4.16](@ref f_code_table_4_16) | inv |  | code table 4.16 quality control value |
| [-code_table_4.2](@ref f_code_table_4_2) | inv |  | code table 4.2 |
| [-code_table_4.230](@ref f_code_table_4_230) | inv |  | code table 4.230 chemical constituent type |
| [-code_table_4.233](@ref f_code_table_4_233) | inv |  | code table 4.233 aerosol type |
| [-code_table_4.235](@ref f_code_table_4_235) | inv |  | code table 4.235 Wind-generated wave spectral description |
| [-code_table_4.240](@ref f_code_table_4_240) | inv |  | code table 4.240 Type of distribution function |
| [-code_table_4.241](@ref f_code_table_4_241) | inv |  | code table 4.241 coverage attributes |
| [-code_table_4.242](@ref f_code_table_4_242) | inv |  | code table 4.242 tile classification |
| [-code_table_4.3](@ref f_code_table_4_3) | inv |  | code table 4.3 Type of Generating Process |
| [-code_table_4.4](@ref f_code_table_4_4) | inv |  | code table 4.4 (first) |
| [-code_table_4.5a](@ref f_code_table_4_5a) | inv |  | code table 4.5 (1st value) |
| [-code_table_4.5b](@ref f_code_table_4_5b) | inv |  | code table 4.5 (2nd value) |
| [-code_table_4.6](@ref f_code_table_4_6) | inv |  | code table 4.6 ensemble type |
| [-code_table_4.7](@ref f_code_table_4_7) | inv |  | code table 4.7 derived forecast |
| [-code_table_4.8](@ref f_code_table_4_8) | inv |  | code table 4.7 derived forecast |
| [-code_table_4.9](@ref f_code_table_4_9) | inv |  | code table 4.9 Probability Type |
| [-code_table_4.91](@ref f_code_table_4_91) | inv |  | code table 4.91 type of interval |
| [-code_table_4.91b](@ref f_code_table_4_91b) | inv |  | code table 4.91 type of interval (2nd copy) |
| [-code_table_5.0](@ref f_code_table_5_0) | inv |  | code table 5.0 data representation number |
| [-code_table_5.1](@ref f_code_table_5_1) | inv |  | code table 5.1 type of original field values |
| [-code_table_5.4](@ref f_code_table_5_4) | inv |  | code table 5.4 group splitting method |
| [-code_table_5.5](@ref f_code_table_5_5) | inv |  | code table 5.5 missing value management for complex packing |
| [-code_table_5.6](@ref f_code_table_5_6) | inv |  | code table 5.5 complex packing spatial differencing |
| [-code_table_5.7](@ref f_code_table_5_7) | inv |  | code table 5.7 precision in IEEE packing |
| [-code_table_6.0](@ref f_code_table_6_0) | inv |  | code table 6.0 Bitmap indicator |
| [-ctl_ens](@ref f_ctl_ens) | inv |  | ens info for g2ctl/GrADS |
| [-ctl_inv](@ref f_ctl_inv) | inv |  | ctl inventory dump for g2ctl/GrADS |
| [-cyclic](@ref f_cyclic) | inv |  | is grid cyclic? (not for thinned grids) |
| [-disc](@ref f_disc) | inv |  | discipline (code table 0.0) |
| [-domain](@ref f_domain) | inv |  | find rectangular domain for g2ctl/GrADS plots |
| [-end_ft](@ref f_end_ft) | inv |  | verf time = reference_time + forecast_time + stat. proc time (YYYYMMDDHH) (same as -vt) |
| [-end_FT](@ref f_end_FT) | inv |  | verf time = reference_time + forecast_time + stat. proc time (YYYYMMDDHHMMSS) (same as -VT) |
| [-ens](@ref f_ens) | inv |  | ensemble information |
| [-ext_name](@ref f_ext_name) | inv |  | extended name, var+qualifiers |
| [-flag_table_3.10](@ref f_flag_table_3_10) | inv |  | flag table 3.10 scanning mode for one diamond |
| [-flag_table_3.3](@ref f_flag_table_3_3) | inv |  | flag table 3.3, resolution and component flags |
| [-flag_table_3.4](@ref f_flag_table_3_4) | inv |  | flag table 3.4, scanning mode |
| [-flag_table_3.5](@ref f_flag_table_3_5) | inv |  | flag table 3.5 projection center |
| [-flag_table_3.9](@ref f_flag_table_3_9) | inv |  | flag table 3.9 numbering order of diamonds seen from corresponding pole |
| [-ftime](@ref f_ftime) | inv |  | either ftime1 or ftime2 dep on version_ftime |
| [-ftime1](@ref f_ftime1) | inv |  | forecast time |
| [-ftime2](@ref f_ftime2) | inv |  | timestamp -- will replace -ftime in the future TESTING |
| [-ftn_api_fn0](@ref f_ftn_api_fn0) | inv |  | n npnts nx ny msg_no submsg i11,5(1x,i11) |
| [-full_name](@ref f_full_name) | inv |  | extended name, var+misc+lev (depreciated) |
| [-gdt](@ref f_gdt) | inv |  | contents of Grid Definition Template (g2c) |
| [-geolocation](@ref f_geolocation) | inv |  | package (proj4,gctpc,internal,not_used) to get lat/lon of grid points |
| [-get_byte](@ref f_get_byte) | inv | X Y Z | get bytes in Section X, Octet Y, number of bytes Z (decimal format) |
| [-get_hex](@ref f_get_hex) | inv | X Y Z | get bytes in Section X, Octet Y, number of bytes Z (bytes in hexadecimal format) |
| [-get_ieee](@ref f_get_ieee) | inv | X Y Z | get ieee float in Section X, Octet Y, number of floats Z |
| [-get_int](@ref f_get_int) | inv | X Y Z | get 4-byte ints in Section X, Octet Y, number of ints Z |
| [-get_int2](@ref f_get_int2) | inv | X Y Z | get 2-byte ints in Section X, Octet Y, number of ints Z |
| [-grib_max_bits](@ref f_grib_max_bits) | inv |  | maximum bits used in grib encoding |
| [-grid](@ref f_grid) | inv |  | grid definition |
| [-grid_id](@ref f_grid_id) | inv |  | show values from grid_id |
| [-hybrid](@ref f_hybrid) | inv |  | shows vertical coordinate parameters from Sec4 (assuming 2 var per level) |
| [-ij](@ref f_ij) | inv | X Y | value of field at grid(X,Y) X=1,..,nx Y=1,..,ny (WxText enabled) |
| [-ijlat](@ref f_ijlat) | inv | X Y | lat,lon and grid value at grid(X,Y) X=1,..,nx Y=1,..,ny (WxText enabled) |
| [-ilat](@ref f_ilat) | inv | X | lat,lon and grid value at Xth grid point, X=1,..,npnts (WxText enabled) |
| [-JMA](@ref f_JMA) | inv |  | inventory for JMA locally defined PDT |
| [-lev](@ref f_lev) | inv |  | level (code table 4.5) |
| [-lev0](@ref f_lev0) | inv |  | level for g2ctl/GrADS |
| [-ll2i](@ref f_ll2i) | inv | X Y | x=lon y=lat, converts to (i), 1..ndata |
| [-ll2ij](@ref f_ll2ij) | inv | X Y | x=lon y=lat, converts lon-lat to (i,j) using gctpc |
| [-lon](@ref f_lon) | inv | X Y | value at grid point nearest lon=X lat=Y (WxText enabled) |
| [-match_inv](@ref f_match_inv) | inv |  | inventory used by -match, -not, -if and -not_if |
| [-Match_inv](@ref f_Match_inv) | inv |  | same as -match_inv except d=YYYYMMDDHH <-> D=YYYYMMDDHHmmss |
| [-scaling](@ref f_scaling) | inv |  | scaling for packing (old format) |
| [-scan](@ref f_scan) | inv |  | scan order of grid |
| [-Sec0](@ref f_Sec0) | inv |  | contents of section0 |
| [-Sec3](@ref f_Sec3) | inv |  | contents of section 3 (Grid Definition Section) |
| [-Sec4](@ref f_Sec4) | inv |  | Sec 4 values (Product definition section) |
| [-Sec5](@ref f_Sec5) | inv |  | Sec 5 values (Data representation section) |
| [-Sec6](@ref f_Sec6) | inv |  | show bit-map section |
| [-Sec_len](@ref f_Sec_len) | inv |  | length of various grib sections |
| [-spatial_proc](@ref f_spatial_proc) | inv |  | show spacial processing, pdt=4.15 |
| [-spectral_bands](@ref f_spectral_bands) | inv |  | spectral bands for satellite, pdt=4.31 or 4.32 |
| [-spectral_bands_extname](@ref f_spectral_bands_extname) | inv |  | spectral bands for satellite, pdt=4.31 or 4.32, concise name for ExtName |
| [-start_ft](@ref f_start_ft) | inv |  | verf time = reference_time + forecast_time (YYYYMMDDHH) : no stat. proc time |
| [-start_FT](@ref f_start_FT) | inv |  | verf time = reference_time + forecast time (YYYYMMDDHHMMSS) - no stat. proc time |
| [-stats](@ref f_stats) | inv |  | statistical summary of data values |
| [-subcenter](@ref f_subcenter) | inv |  | subcenter |
| [-t](@ref f_t) | inv |  | reference time YYYYMMDDHH, -v2 for alt format |
| [-T](@ref f_T) | inv |  | reference time YYYYMMDDHHMMSS |
| [-table](@ref f_table) | inv |  | parameter table |
| [-timer](@ref f_timer) | inv |  | reads OpenMP timer |
| [-unix_time](@ref f_unix_time) | inv |  | print unix timestamp for rt & vt |
| [-V](@ref f_V) | inv |  | diagnostic output |
| [-var](@ref f_var) | inv |  | short variable name |
| [-varX](@ref f_varX) | inv |  | raw variable name - discipline mastertab localtab center parmcat parmnum |
| [-vector_dir](@ref f_vector_dir) | inv |  | grid or earth relative winds |
| [-verf](@ref f_verf) | inv |  | simple inventory using verification time |
| [-vt](@ref f_vt) | inv |  | verf time = reference_time + forecast time, -v2 for alt format |
| [-VT](@ref f_VT) | inv |  | verf time = reference_time + forecast time (YYYYMMDDHHMMSS) |
| [-warn_old_g2](@ref f_warn_old_g2) | inv |  | warn if old g2lib would have problem |
| [-wave_partition](@ref f_wave_partition) | inv |  | ocean surface wave partition (pdt=4.52) |
| [-YY](@ref f_YY) | inv |  | reference time YYYY |
| [-inv_f77](@ref f_inv_f77) | inv> | X Y Z | match inventory written to Z with character*(Y) and X=(bin,ieee) |
| [-last](@ref f_last) | inv> | X | write last inv item to file X |
| [-last0](@ref f_last0) | inv> | X | write last inv item to beginning of file X |
| [-nl_out](@ref f_nl_out) | inv> | X | write new line in file X |
| [-print_out](@ref f_print_out) | inv> | X Y | prints string (X) in file (Y) |
| [-s_out](@ref f_s_out) | inv> | X | simple inventory written to X |
| [-big_endian](@ref f_big_endian) | misc |  | sets ieee output to big endian (default is big endian) |
| [-box_ave](@ref f_box_ave) | misc | X Y Z | box average X=odd integer (lon) Y=odd integer (lat) critical_weight |
| [-check_pdt_size](@ref f_check_pdt_size) | misc | X | check pdt size X=1 enable/default, X=0 disable |
| [-colon](@ref f_colon) | misc | X | replace item deliminator (:) with X |
| [-config](@ref f_config) | misc |  | shows the configuration |
| [-count](@ref f_count) | misc |  | prints count, number times this -count was processed |
| [-end](@ref f_end) | misc |  | stop after first (sub)message (save time) |
| [-error_final](@ref f_error_final) | misc | X Y Z | error if at end X=count Y=ne,eq,le,lt,gt,ge Z=integer |
| [-export_lonlat](@ref f_export_lonlat) | misc | X | save lon-lat data in binary file |
| [-fix_CFSv2_fcst](@ref f_fix_CFSv2_fcst) | misc | X Y Z | fixes CFSv2 monthly fcst X=daily or 00/06/12/18 Y=pert no. Z=number ens fcsts v1.0 |
| [-fix_ncep](@ref f_fix_ncep) | misc |  | fix ncep PDT=8 headers produced by cnvgrib |
| [-gctpc](@ref f_gctpc) | misc | X | X=0,1 use gctpc library (default=1) |
| [-grid_changes](@ref f_grid_changes) | misc |  | prints number of grid changes |
| [-grid_def](@ref f_grid_def) | misc |  | read lon and lat data from grib file -- experimental |
| [-h](@ref f_h) | misc |  | help, shows common options |
| [-header](@ref f_header) | misc |  | f77 header or nx-ny header in text output (default) |
| [-help](@ref f_help) | misc | X | help [search string\|all], -help all, shows all options |
| [-ijundefine](@ref f_ijundefine) | misc | X Y Z | sets grid point values to undefined X=(in-box\|out-box) Y=ix0:ix1 Z=iy0:iy1 ix=(1..nx) iy=(1..ny) |
| [-import_bin](@ref f_import_bin) | misc | X | read binary file (X) for data |
| [-import_grib](@ref f_import_grib) | misc | X | read grib2 file (X) for data |
| [-import_grib_fs](@ref f_import_grib_fs) | misc | X Y | read grib2 file (Y) sequentially for record that matches X (fixed string) |
| [-import_ieee](@ref f_import_ieee) | misc | X | read ieee file (X) for data |
| [-import_lonlat](@ref f_import_lonlat) | misc | X | read lon-lat data from binary file |
| [-import_netcdf](@ref f_import_netcdf) | misc | X Y Z | alpha X=file Y=var Z=hyper-cube specification |
| [-import_text](@ref f_import_text) | misc | X | read text file (X) for data |
| [-limit](@ref f_limit) | misc | X | stops after X fields decoded |
| [-little_endian](@ref f_little_endian) | misc |  | sets ieee output to little endian (default is big endian) |
| [-mem_del](@ref f_mem_del) | misc | X | delete mem file X |
| [-mem_final](@ref f_mem_final) | misc | X Y | write mem file X to file Y at cleanup step |
| [-mem_init](@ref f_mem_init) | misc | X Y | read mem file X from file Y (on initialization) |
| [-new_grid_format](@ref f_new_grid_format) | misc | X | new_grid output format X=bin,ieee,grib |
| [-new_grid_interpolation](@ref f_new_grid_interpolation) | misc | X | new_grid interpolation X=bilinear,bicubic,neighbor,budget |
| [-new_grid_ipopt](@ref f_new_grid_ipopt) | misc | X | new_grid ipopt values X=i1:i2..:iN N <= 20 |
| [-new_grid_vectors](@ref f_new_grid_vectors) | misc | X | change fields to vector interpolate: X=none,default,UGRD:VGRD,(U:V list) |
| [-new_grid_winds](@ref f_new_grid_winds) | misc | X | new_grid wind orientation: X = grid, earth (no default) |
| [-no_header](@ref f_no_header) | misc |  | no f77 header or nx-ny header in text output |
| [-proj4](@ref f_proj4) | misc | X | X=0,1 use proj4 library for geolocation (testing) |
| [-quit](@ref f_quit) | misc |  | stop after first (sub)message (save time) |
| [-read_sec](@ref f_read_sec) | misc | X Y | read grib message section (0-8) X from binary file (Y) |
| [-rewind_final](@ref f_rewind_final) | misc | X | rewinds file X on cleanup step if already opened, CW2 |
| [-rewind_proc](@ref f_rewind_proc) | misc | X | rewinds file X on processing step if already opened, CW2 |
| [-rpn](@ref f_rpn) | misc | X | reverse polish notation calculator |
| [-rpn_rcl](@ref f_rpn_rcl) | misc | X | data = register X .. same as -rpn rcl_X .. no geolocation calc needed |
| [-rpn_sto](@ref f_rpn_sto) | misc | X | register X = data.. same as -rpn sto_X .. no geolocation calc needed |
| [-scaling_0001](@ref f_scaling_0001) | misc |  | changes scaling testing (sample) |
| [-set](@ref f_set) | misc | X Y | set X = Y, X=local_table,etc (help: -set help help) |
| [-set_ave](@ref f_set_ave) | misc | X | set ave/acc .. only use on pdt=4.0/4.8 (old code) |
| [-set_bin_prec](@ref f_set_bin_prec) | misc | X | X use X bits and ECMWF-style grib encoding |
| [-set_bitmap](@ref f_set_bitmap) | misc | X | use bitmap when creating complex packed files X=1/0 |
| [-set_byte](@ref f_set_byte) | misc | X Y Z | set bytes in Section X, Octet Y, bytes Z (a\|a:b:c) |
| [-set_center](@ref f_set_center) | misc | X | changes center X = C or C:S C and S are center/subcenter numbers |
| [-set_date](@ref f_set_date) | misc | X | changes date code, X=(+\|-)N(hr\|dy\|mo\|yr), YYYYMMDDHHmmSS, u(UNIX TIME) |
| [-set_ensm_derived_fcst](@ref f_set_ensm_derived_fcst) | misc | X Y | convert PDT 0,1,2 -> 2, 8,11,12 -> 12, X=code table 4.7 Y=num ens members |
| [-set_ens_num](@ref f_set_ens_num) | misc | X Y Z | ensemble member info, X=code table 4.6 Y=pert num Z=num ens members -1=No Change |
| [-set_flag_table_3.3](@ref f_set_flag_table_3.3) | misc | X | flag table 3.3 = X |
| [-set_flag_table_3.4](@ref f_set_flag_table_3.4) | misc | X | flag table 3.4 = X |
| [-set_ftime](@ref f_set_ftime) | misc | X | either set_ftime1 or set_ftime2 dep on version_ftime |
| [-set_ftime1](@ref f_set_ftime1) | misc | X | set ftime |
| [-set_ftime2](@ref f_set_ftime2) | misc | X | set ftime2 .. will be replace -set_ftime/ave in the future -- TESTING --- |
| [-set_gds](@ref f_set_gds) | misc | X | makes new gds (section 3), X=size in bytes |
| [-set_grib_max_bits](@ref f_set_grib_max_bits) | misc | X | sets scaling so number of bits does not exceed N in (new) grib output |
| [-set_grib_type](@ref f_set_grib_type) | misc | X | set grib type = jpeg, simple, ieee, complex(1\|2\|3), aec, same |
| [-set_hex](@ref f_set_hex) | misc | X Y Z | set bytes in Section X, Octet Y, bytes Z (a\|a:b:c\|abc) in hexadecimal |
| [-set_ieee](@ref f_set_ieee) | misc | X Y Z | set ieee float in Section X, Octet Y, floats Z (a\|a:b:c) |
| [-set_ijval](@ref f_set_ijval) | misc | X Y Z | sets grid point value X=ix Y=iy Z=val |
| [-set_int](@ref f_set_int) | misc | X Y Z | set 4-byte ints in Section X, Octet Y, signed integers Z (a\|a:b:c) |
| [-set_int2](@ref f_set_int2) | misc | X Y Z | set 2-byte ints in Section X, Octet Y, signed integers Z (a\|a:b:c) |
| [-set_ival](@ref f_set_ival) | misc | X Y | sets grid point value X=i1:i2:.. Y=va1:val2:.. grid[i1] = val1,etc i>0 |
| [-set_lev](@ref f_set_lev) | misc | X | changes level code .. not complete |
| [-set_metadata](@ref f_set_metadata) | misc | X | read meta-data for grib writing from file X |
| [-set_metadata_str](@ref f_set_metadata_str) | misc | X | X = metadata string |
| [-set_pdt](@ref f_set_pdt) | misc | X | makes new pdt, X=(+)PDT_number or X=(+)PDT_number:size of PDT in octets, +=copy metadata |
| [-set_percentile](@ref f_set_percentile) | misc | X | convert PDT 0..6 -> 6, 8..15 -> 10, X=percentile (0..100) |
| [-set_prob](@ref f_set_prob) | misc | 5 args | X/Y forecasts Z=Code Table 4.9 A=lower limit B=upper limit |
| [-set_radius](@ref f_set_radius) | misc | X | set radius of Earth X= 0,2,4,5,6,8,9 (Code Table 3.2), X=1:radius , X=7:major:minor |
| [-set_scaling](@ref f_set_scaling) | misc | X Y | set decimal scaling=X/same binary scaling=Y/same new grib messages |
| [-set_sec_size](@ref f_set_sec_size) | misc | X Y | resizes section , X=section number, Y=size in octets, DANGEROUS |
| [-set_ts_dates](@ref f_set_ts_dates) | misc | X Y Z | changes date code for time series X=YYYYMMDDHH(mmss) Y=dtime Z=#msgs/date |
| [-set_var](@ref f_set_var) | misc | X | changes variable name |
| [-start_timer](@ref f_start_timer) | misc |  | starts OpenMP timer |
| [-status](@ref f_status) | misc | X | X X=file |
| [-submsg](@ref f_submsg) | misc | X | process submessage X (0=process all messages) |
| [-sys](@ref f_sys) | misc | X | run system/shell command, X=shell command |
| [-text_col](@ref f_text_col) | misc | X | number of columns on text output |
| [-text_fmt](@ref f_text_fmt) | misc | X | format for text output (C) |
| [-udf](@ref f_udf) | misc | X Y | run UDF, X=program+optional_args, Y=return file |
| [-udf_arg](@ref f_udf_arg) | misc | X Y | add grib-data to UDF argument file, X=file Y=name |
| [-undefine](@ref f_undefine) | misc | X Y Z | sets grid point values to undefined X=(in-box\|out-box) Y=lon0:lon1 Z=lat0:lat1 |
| [-undefine_val](@ref f_undefine_val) | misc | X | grid point set to undefined if X=val or X=low:high |
| [-v](@ref f_v) | misc |  | verbose (v=1) |
| [-v0](@ref f_v0) | misc |  | not verbose (v=0) |
| [-v1](@ref f_v1) | misc |  | verbose (v=1) |
| [-v2](@ref f_v2) | misc |  | really verbose (v=2) |
| [-v97](@ref f_v97) | misc |  | verbose mode for debugging only (v=97) |
| [-v98](@ref f_v98) | misc |  | verbose mode for debugging only (v=98) |
| [-v99](@ref f_v99) | misc |  | verbose mode for debugging only (v=99) |
| [-version](@ref f_version) | misc |  | print version |
| [-AAIG](@ref f_AAIG) | out |  | writes Ascii ArcInfo Grid file, lat-lon grid only (alpha) |
| [-AAIGlong](@ref f_AAIGlong) | out |  | writes Ascii ArcInfo Grid file, lat-lon grid only long-name *.asc (alpha) |
| [-ave](@ref f_ave) | out | X Y | average X=time step Y=output v2 |
| [-ave0](@ref f_ave0) | out | X Y | average X=time step, Y=output grib file needs file is special order |
| [-ave_var](@ref f_ave_var) | out | X Y | average/std dev/min/max X=time step, Y=output |
| [-bin](@ref f_bin) | out | X | write binary data to X |
| [-cress_lola](@ref f_cress_lola) | out | X..Z,A | lon-lat grid values X=lon0:nlon:dlon Y=lat0:nlat:dlat Z=file A=radius1:radius2:..:radiusN |
| [-csv](@ref f_csv) | out | X | make comma separated file, X=file (WxText enabled) |
| [-csv_long](@ref f_csv_long) | out | X | make comma separated file, X=file (WxText enabled) |
| [-cubeface2global](@ref f_cubeface2global) | out | X Y | write faces X as global cubed grid to Y: X=list of faces to exclude |
| [-ens_processing](@ref f_ens_processing) | out | X Y | ave/min/max/spread X=output Y=future use |
| [-ens_qc](@ref f_ens_qc) | out | X..Z,A | simple qc ensemble members X=stats.grb Y=extreme.grb Z=extreme.txt A=1 (qc_version) |
| [-fcst_ave](@ref f_fcst_ave) | out | X Y | average X=time step Y=output v2 |
| [-fcst_ave0](@ref f_fcst_ave0) | out | X Y | average X=time step, Y=output grib file needs file is special order |
| [-fi](@ref f_fi) | out |  | depreceated, used in old IF structure |
| [-grib](@ref f_grib) | out | X | writes GRIB record (one submessage) to X |
| [-GRIB](@ref f_GRIB) | out | X | writes entire GRIB record (all submessages) |
| [-grib_ieee](@ref f_grib_ieee) | out | X | writes data[] to X.grb, X.head, X.tail, and X.h |
| [-grib_out](@ref f_grib_out) | out | X | writes decoded/modified data in grib-2 format to file X |
| [-grib_out_irr](@ref f_grib_out_irr) | out | X Y | writes irregular grid grib (GDT=130 not adopted) X=(all\|defined) Y=(output file) |
| [-grib_out_irr2](@ref f_grib_out_irr2) | out | 5 args | writes irregular grid grib GDT 101 X=npnts Y=grid_no Z=grid_ref A=UUID B=(output file) |
| [-gribtable_used](@ref f_gribtable_used) | out | X | write out sample gribtable as derived from grib file, X=file |
| [-gridout](@ref f_gridout) | out | X | text file with grid: i j lat lon (1st record) |
| [-ieee](@ref f_ieee) | out | X | write (default:big-endian) IEEE data to X |
| [-ijbox](@ref f_ijbox) | out | X..Z,A | grid values in bounding box X=i1:i2[:di] Y=j1:j2[:dj] Z=file A=[bin\|text\|spread] |
| [-ijsmall_grib](@ref f_ijsmall_grib) | out | X Y Z | make small domain grib file X=ix0:ix1 Y=iy0:iy1 Z=file |
| [-irr_grid](@ref f_irr_grid) | out | X Y Z | make irregular grid (GDT=130 not adopted), nearest neighbor, X=lon-lat list Y=radius (km) Z=output grib file |
| [-lola](@ref f_lola) | out | X..Z,A | lon-lat grid values X=lon0:nlon:dlon Y=lat0:nlat:dlat Z=file A=[bin\|text\|spread\|grib] |
| [-merge_fcst](@ref f_merge_fcst) | out | X Y | merge forecast ave/acc/min/max X=number to intervals to merge (0=every) Y=output grib file |
| [-mysql](@ref f_mysql) | out | 5 args | H=[host] U=[user] P=[password] D=[db] T=[table] |
| [-mysql_dump](@ref f_mysql_dump) | out | 7 args | H=[host] U=[user] P=[password] D=[db] T=[table] W=[western_lons:0\|1] PV=[remove unlikely:0\|1] |
| [-mysql_speed](@ref f_mysql_speed) | out | 7 args | H=[host] U=[user] P=[password] D=[db] T=[table] W=[western_lons:0\|1] PV=[remove unlikely:0\|1] |
| [-ncep_norm](@ref f_ncep_norm) | out | X | normalize NCEP-type ave/acc X=output grib file |
| [-ncep_uv](@ref f_ncep_uv) | out | X | combine U and V fields into one message like NCEP operations |
| [-netcdf](@ref f_netcdf) | out | X | write netcdf data to X |
| [-new_grid](@ref f_new_grid) | out | X..Z,A | bilinear interpolate: X=projection Y=x0:nx:dx Z=y0:ny:dy A=grib_file alpha |
| [-new_grid_order](@ref f_new_grid_order) | out | X Y | put in required order for -new_grid, X=out Y=out2 no matching vector |
| [-reduced_gaussian_grid](@ref f_reduced_gaussian_grid) | out | X Y Z | reduced Gaussian grid, X=outputfile Y=-1 Z=(neighbor\|linear)[-extrapolate] |
| [-small_grib](@ref f_small_grib) | out | X Y Z | make small domain grib file X=lonW:lonE Y=latS:latN Z=file |
| [-spread](@ref f_spread) | out | X | write text - spread sheet format into X (WxText enabled) |
| [-submsg_uv](@ref f_submsg_uv) | out | X | combine vector fields into one message |
| [-text](@ref f_text) | out | X | write text data into X |
| [-time_processing](@ref f_time_processing) | out | X..Z,A | average X=CodeTable 4.10 Y=CodeTable 4.11 Z=time step A=output |
| [-tosubmsg](@ref f_tosubmsg) | out | X | convert GRIB message to submessage and write to file X |
| [-unmerge_fcst](@ref f_unmerge_fcst) | out | X Y Z | unmerge_fcst X=output Y=fcst_time_0 Z: 0->result 1->+init 2->+all |
| [-wind_dir](@ref f_wind_dir) | out | X | calculate wind direction, X = output gribfile (direction in degrees, 0=wind from north, 90=wind from east) |
| [-wind_speed](@ref f_wind_speed) | out | X | calculate wind speed, X = output gribfile (U then V in datafile) |
| [-wind_uv](@ref f_wind_uv) | out | X | calculate UGRD/VGRD from speed/dir, X = output gribfile |
| [-write_sec](@ref f_write_sec) | out | X Y | write grib msessage section X (0-8) to binary file Y |
| [-alarm](@ref f_alarm) | init | X | terminate after X seconds |
| [-append](@ref f_append) | init |  | append mode, write to existing output files |
| [-crlf](@ref f_crlf) | init |  | make the end of the inventory a crlf (windows) instead of newline (unix) |
| [-d](@ref f_d) | init | X | dump message X = n, n.m, n:offset, n.m:offset, only 1 -d allowed |
| [-egrep](@ref f_egrep) | init | X | egrep X \| wgrib2 (X is POSIX regular expression) |
| [-egrep_v](@ref f_egrep_v) | init | X | egrep -v X \| wgrib2 (X is POSIX regular expression) |
| [-eof_bin](@ref f_eof_bin) | init | X Y | send (binary) integer to file upon EOF: X=file Y=integer |
| [-eof_string](@ref f_eof_string) | init | X Y | send string to file upon EOF: X=file Y=string |
| [-err_bin](@ref f_err_bin) | init | X Y | send (binary) integer to file upon err exit: X=file Y=integer |
| [-err_string](@ref f_err_string) | init | X Y | send string to file upon err exit: X=file Y=string |
| [-fgrep](@ref f_fgrep) | init | X | fgrep X \| wgrib2 |
| [-fgrep_v](@ref f_fgrep_v) | init | X | fgrep -v X \| wgrib2 |
| [-fix_ncep_2](@ref f_fix_ncep_2) | init |  | ncep bug fix 2, probability observation < -ve number |
| [-fix_ncep_3](@ref f_fix_ncep_3) | init |  | sets flag to fix ncep bug 3 (constant fields) |
| [-fix_ncep_4](@ref f_fix_ncep_4) | init |  | fixes NCEP grib2 files where DX and DY are undefined |
| [-fix_undef](@ref f_fix_undef) | init |  | set unused values to undef |
| [-flush](@ref f_flush) | init |  | flush output buffers after every write (interactive) |
| [-for](@ref f_for) | init | X | process record numbers in range, X=(start:end:step), only one -for allowed |
| [-for_n](@ref f_for_n) | init | X | process inv numbers in range, X=(start:end:step), only one -for allowed |
| [-g2clib](@ref f_g2clib) | init | X | X=0/1/2 0=WMO std 1=emulate g2clib 2=use g2clib |
| [-i](@ref f_i) | init |  | read Inventory from stdin |
| [-i_file](@ref f_i_file) | init | X | read Inventory from file |
| [-inv](@ref f_inv) | init | X | write inventory to X |
| [-match](@ref f_match) | init | X | process data that matches X (POSIX regular expression) |
| [-match_fs](@ref f_match_fs) | init | X | process data that matches X (fixed string) |
| [-match_inv_add](@ref f_match_inv_add) | init | X Y Z | add new options to match_inventory |
| [-names](@ref f_names) | init | X | grib name convention, X=DWD, dwd, ECMWF, ecmwf, NCEP, ncep |
| [-nc3](@ref f_nc3) | init |  | use netcdf3 (classic) |
| [-nc4](@ref f_nc4) | init |  | use netcdf4 (compressed, controlled endianness etc) |
| [-nc_grads](@ref f_nc_grads) | init |  | require netcdf file to be grads v1.9b4 compatible (fixed time step only) |
| [-nc_nlev](@ref f_nc_nlev) | init | X | netcdf, X = max LEV dimension for {TIME,LEV,LAT,LON} data |
| [-nc_pack](@ref f_nc_pack) | init | X | pack/check limits of all NEW input variables, X=min:max[:byte\|short\|float] |
| [-ncpu](@ref f_ncpu) | init | X | number of threads, default is environment variable OMP_NUM_THREADS/number of cpus |
| [-nc_table](@ref f_nc_table) | init | X | X is conversion_to_netcdf_table file name |
| [-nc_time](@ref f_nc_time) | init | X | netcdf, [[-]yyyymmddhhnnss]:[dt{s[ec]\|m[in]\|h[our]\|d[ay]}], [-] is for time alignment only |
| [-ndate](@ref f_ndate) | init | X Y | X=date Y=dt print date + dt |
| [-ndates](@ref f_ndates) | init | X Y Z | X=date0 Y=(date1\|dt1) Z=dt2 for (date=date0; date<(date1\|date0+dt1); date+=dt2) print date |
| [-ndates_fmt](@ref f_ndates_fmt) | init | X | X = C format for ndates option ex. 'date=%s' |
| [-no_append](@ref f_no_append) | init |  | not append mode, write to new output files (default) |
| [-no_flush](@ref f_no_flush) | init |  | flush output buffers when full (default) |
| [-no_nc_grads](@ref f_no_nc_grads) | init |  | netcdf file may be not grads v1.9b4 compatible, variable time step |
| [-no_nc_pack](@ref f_no_nc_pack) | init |  | no packing in netcdf for NEW variables |
| [-no_nc_table](@ref f_no_nc_table) | init |  | disable previously defined conversion_to_netcdf_table |
| [-no_nc_time](@ref f_no_nc_time) | init |  | netcdf, disable previously defined initial or relative date and time step |
| [-not](@ref f_not) | init | X | process data that does not match X (POSIX regular expression) |
| [-not_fs](@ref f_not_fs) | init | X | process data that does not match X (fixed string) |
| [-one_line](@ref f_one_line) | init |  | puts all on one line (makes into inventory format) |
| [-order](@ref f_order) | init | X | decoded data in X (raw\|we:sn\|we:ns) order, we:sn is default |
| [-persistent](@ref f_persistent) | init | X | makes file X persistent if already opened (default on open), CW2 |
| [-rewind_init](@ref f_rewind_init) | init | X | rewinds file X on initialization if already opened, CW2 |
| [-set_ext_name](@ref f_set_ext_name) | init | X | X=type ext_name (1*misc+2*level+4*ftime) |
| [-set_ext_name_chars](@ref f_set_ext_name_chars) | init | X Y | extended name characters X=field Y=space |
| [-set_regex](@ref f_set_regex) | init | X | set regex mode X = 0:extended regex (default) 1:pattern 2:extended regex & quote metacharacters |
| [-set_version_ftime](@ref f_set_version_ftime) | init | X | set version of ftime X=1, 2 |
| [-tigge](@ref f_tigge) | init |  | use modified-TIGGE grib table |
| [-transient](@ref f_transient) | init | X | make file X transient, CW2 |