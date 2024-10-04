/* used by C and fortran callable_wgrib2 API  10/2024 Public Domain  Wesley Ebisuzaki */

int wgrib2(int argc, const char **argv);
int wgrib2a(char *arg1, ...);

size_t wgrib2_get_mem_buffer_size(int n);
int wgrib2_get_mem_buffer(unsigned char *my_buffer, size_t size, int n);
int wgrib2_set_mem_buffer(const unsigned char *my_buffer, size_t size, int n);

size_t wgrib2_get_reg_size(int reg);
int wgrib2_get_reg_data(float *data, size_t size, int reg);
int wgrib2_set_reg(float *data, size_t size, int reg);

int wgrib2_free_file(const char *filename);		// will delete files that are used/not used
