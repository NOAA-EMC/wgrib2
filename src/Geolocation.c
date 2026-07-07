/** @file
 * @brief Implements the -geolocation option for wgrib2.
 * @author Public Domain: Wesley Ebisuzaki @date 4/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "wgrib2.h"
#include "fnlist.h"

/*
 * HEADER:100:geolocation:inv:0:package used to get lat/lon of grid points (proj4,gctpc,internal,external, not_used)
 */

/** Current geolocation type. */
extern enum geolocation_type geolocation;

/**
 * Print the geolocation routines used.
 * 
 * Geolocation routines are used to calculate the longitudes and latitudes of the grid points. Wgrib2 can use Proj4, 
 * gctpc and its own internal routines to do the geolocation. Each set of routines has advanages and disadvantages. 
 * The routines are listed in the default order. 
 * 
 * ## Proj
 * The Proj library was called PROJ4 from 1994-2017. In 2018, PROJ v5 was released and the software was renamed PROJ. 
 * Wgrib2 is still using a v4 library and all references in the code are to PROJ4.
 * 
 * PROJ is the gold standard of geolocation libraries. It is commonly used and well supported. The drawbacks are it 
 * doesn't support OpenMP and model grids. As of wgrib2 v3.1.2, PROJ4 is installed by default in order to support 
 * aspherical equal area Lambert grids.
 *
 * ## GCTPC
 * Gctpc is an older library was which has lost support of its original authors (USGS). The advantage is that it does 
 * support OpenMP unlike Proj4. It supports fewer grids than Proj4 but that isn't a major problem because the grib2 
 * standard only has support for a few grids. Gctpc supports an ellipsoidal earth and is always compiled with wgrib2. 
 * 
 * ## Internal
 * The internal routines are used to support model grids (Gaussian grid, rotated lat-lon grid), spherical earth grids, 
 * and observational grids such as the space-view grid (used by EUMETSAT) and radar grids (angle, distance from the origin). 
 * 
 * ## External, wgrib2 v3.1.2+
 * Wgrib2 can set or overwrite the locations of the grid points using -rpn (sto_lat, sto_lon), reading from a grib file 
 * using -grid_def, and reading from a double precision binary file using -import_lonlat. These methods will set the 
 * geolocation flag to external. 
 * 
 * For wgrib2 prior to v3.1.2, the geolocation flag showed which geolocation library generated the locations of the grid 
 * points. External methods were not considered. 
 * 
 * ## Usage
 * -geolocation
 *
 * Prints out either proj4, gctpc, internal, external, or not_used.
 * 
 * External means the lat/lon may be read or computed by an external calculation.
 * 
 * If wgrib2 does not need the geolocation information, the geolocation routines will not be called.
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success
 * 
 * ## Example
 * ???
 * 
 * @author Wesley Ebisuzaki @date 4/2020
 */
int f_geolocation(ARG0) {

    if (mode >= 0) {
        strcat(inv_out,"geolocation=");
        inv_out += strlen(inv_out);
        if (geolocation == proj4) strcat(inv_out,"proj4");
        else if (geolocation == gctpc) strcat(inv_out,"gctpc");
        else if (geolocation == internal) strcat(inv_out,"internal");
        else if (geolocation == external) strcat(inv_out,"external");
        else if (geolocation == not_used) strcat(inv_out,"not_used");
    }
    return 0;
}
