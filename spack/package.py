# Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

import os
import re

from spack.package import *

variant_map = {
    "netcdf": "USE_NETCDF",
    "spectral": "USE_SPECTRAL",
    "mysql": "USE_MYSQL",
    "udf": "USE_UDF",
    "regex": "USE_REGEX",
    "tigge": "USE_TIGGE",
    "proj4": "USE_PROJ4",
    "aec": "USE_AEC",
    "g2c": "USE_G2CLIB",
    "png": "USE_PNG",
    "jasper": "USE_JASPER",
    "openmp": "USE_OPENMP",
    "wmo_validation": "USE_WMO_VALIDATION",
    "ipolates": "USE_IPOLATES",
    "disable_timezone": "DISABLE_TIMEZONE",
    "disable_alarm": "DISABLE_ALARM",
    "fortran_api": "MAKE_FTN_API",
    "disable_stat": "DISABLE_STAT",
    "openjpeg": "USE_OPENJPEG",
    "enable_docs": "ENABLE_DOCS",
}


class Wgrib2(MakefilePackage, CMakePackage):
    """Utility for interacting with GRIB2 files"""

    homepage = "https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2"
    url = "https://www.ftp.cpc.ncep.noaa.gov/wd51we/wgrib2/wgrib2.tgz.v2.0.8"
    git = "https://github.com/NOAA-EMC/wgrib2"

    maintainers("t-brown", "AlexanderRichert-NOAA", "Hang-Lei-NOAA", "edwardhartnett")

    build_system(conditional("cmake", when="@3.2:"), conditional("makefile", when="@:3.1"))

    version("develop", branch="develop")
    version("3.3.0", sha256="010827fba9c31f05807e02375240950927e9e51379e1444388153284f08f58e2")
    version("3.2.0", sha256="ac3ace77a32c2809cbc4538608ad64aabda2c9c1e44e7851da79764a6eb3c369")
    version(
        "3.1.1",
        sha256="9236f6afddad76d868c2cfdf5c4227f5bdda5e85ae40c18bafb37218e49bc04a",
        extension="tar.gz",
    )
    version(
        "3.1.0",
        sha256="5757ef9016b19ae87491918e0853dce2d3616b14f8c42efe3b2f41219c16b78f",
        extension="tar.gz",
    )
    version(
        "2.0.8",
        sha256="5e6a0d6807591aa2a190d35401606f7e903d5485719655aea1c4866cc2828160",
        extension="tar.gz",
    )
    version(
        "2.0.7",
        sha256="d7f1a4f9872922c62b3c7818c022465532cca1f5666b75d3ac5735f0b2747793",
        extension="tar.gz",
    )

    def url_for_version(self, version):
        if version >= Version("3.2.0"):
            url_fmt = "https://github.com/NOAA-EMC/wgrib2/archive/refs/tags/v{0}.tar.gz"
        else:
            url_fmt = "https://www.ftp.cpc.ncep.noaa.gov/wd51we/wgrib2/wgrib2.tgz.v{0}"
        return url_fmt.format(version)

    variant(
        "netcdf", default=False, description="Link in netcdf library to read, write netcdf files"
    )
    variant(
        "ipolates",
        default=False,
        description="Use to interpolate to new grids",
        when="@3.3:",
    )
    variant(
        "spectral", default=False, description="Spectral interpolation in -new_grid", when="@:3.1"
    )
    #   Not currently working for @3.2:
    variant(
        "fortran_api",
        default=True,
        description="Make wgrib2api which allows fortran code to read/write grib2",
        when="@3.3:",
    )
    #   Not currently working for @3.2:
    #    variant("lib", default=True, description="Build library", when="@3.2:")
    variant(
        "mysql",
        default=False,
        description="Link in interface to MySQL to write to mysql database",
        when="@:3.1",
    )
    variant(
        "udf",
        default=False,
        description="Add commands for user-defined functions and shell commands",
    )
    variant("regex", default=True, description="Use regular expression library (POSIX-2)")
    variant("tigge", default=True, description="Ability for TIGGE-like variable names")
    variant(
        "proj4",
        default=False,
        description="The proj4 library is used to verify gctpc.",
        when="@:3.1",
    )
    variant(
        "aec", default=True, description="Use the libaec library for packing with GRIB2 template"
    )
    variant(
        "g2c",
        default=False,
        description="Include NCEP g2clib (mainly for testing purposes)",
        when="@:3.1",
    )
    variant(
        "disable_timezone", default=False, description="Some machines do not support timezones"
    )
    variant(
        "disable_alarm",
        default=False,
        description="Some machines do not support the alarm to terminate wgrib2",
    )
    variant("png", default=True, description="PNG encoding")
    variant("jasper", default=True, description="JPEG compression using Jasper")
    variant("openmp", default=True, description="OpenMP parallelization")
    variant("wmo_validation", default=False, description="WMO validation")
    #    variant("shared", default=False, description="Enable shared library", when="+lib")
    variant("disable_stat", default=False, description="Disable POSIX feature", when="@:3.1")
    variant("openjpeg", default=False, description="Enable OpenJPEG", when="@:3.1")
    variant("enable_docs", default=False, description="Build doxygen documentation", when="@3.4.0:")

    conflicts("+netcdf3", when="+netcdf4")
    conflicts("+openmp", when="%apple-clang")

    depends_on("wget", type=("build"), when="@:3.1 +netcdf4")
    depends_on("ip@5.1:", when="@develop +ipolates")
    depends_on("libaec@1.0.6:", when="@3.2: +aec")
    depends_on("netcdf-c", when="@3.2: +netcdf4")
    depends_on("jasper@:2", when="@3.2: +jasper")
    depends_on("zlib-api", when="+png")
    depends_on("libpng", when="+png")
    depends_on("openjpeg", when="+openjpeg")

    # Use Spack compiler wrapper flags
    def inject_flags(self, name, flags):
        if name == "cflags":
            if self.spec.compiler.name == "apple-clang":
                flags.append("-Wno-error=implicit-function-declaration")

            # When mixing Clang/gfortran need to link to -lgfortran
            # Find this by searching for gfortran/../lib
            if self.spec.compiler.name in ["apple-clang", "clang"]:
                if "gfortran" in self.compiler.fc:
                    output = Executable(self.compiler.fc)("-###", output=str, error=str)
                    libdir = re.search("--libdir=(.+?) ", output).group(1)
                    flags.append("-L{}".format(libdir))

        return (flags, None, None)


class CMakeBuilder(spack.build_systems.cmake.CMakeBuilder):
    # Disable parallel build
    parallel = False

    def cmake_args(self):
        args = [self.define_from_variant(variant_map[k], k) for k in variant_map]
        #        args.append(self.define_from_variant("BUILD_LIB", "lib"))
        #        args.append(self.define_from_variant("BUILD_SHARED_LIB", "shared"))

        return args


class MakefileBuilder(spack.build_systems.makefile.MakefileBuilder):
    # Disable parallel build
    parallel = False

    flag_handler = inject_flags

    def url_for_version(self, version):
        url = "https://www.ftp.cpc.ncep.noaa.gov/wd51we/wgrib2/wgrib2.tgz.v{}"
        return url.format(version)

    def edit(self, pkg, spec, prefix):
        makefile = FileFilter("makefile")

        # ifort no longer accepts -openmp
        makefile.filter(r"-openmp", "-qopenmp")
        makefile.filter(r"-Wall", " ")
        makefile.filter(r"-Werror=format-security", " ")

        # clang doesn"t understand --fast-math
        if spec.satisfies("%clang") or spec.satisfies("%apple-clang"):
            makefile.filter(r"--fast-math", "-ffast-math")

        for variant_name, makefile_option in variant_map.items():
            value = int(spec.variants[variant_name].value)
            makefile.filter(r"^%s=.*" % makefile_option, "{}={}".format(makefile_option, value))

    def setup_build_environment(self, env):
        if self.spec.compiler.name in ["oneapi", "intel"]:
            comp_sys = "intel_linux"
        elif self.spec.compiler.name in ["gcc", "clang", "apple-clang"]:
            comp_sys = "gnu_linux"

        env.set("COMP_SYS", comp_sys)

    def build(self, pkg, spec, prefix):
        # Get source files for netCDF4 builds
        if self.spec.satisfies("+netcdf4"):
            with working_dir(self.build_directory):
                os.system(
                    "wget https://downloads.unidata.ucar.edu/netcdf-c/4.8.1/netcdf-c-4.8.1.tar.gz"
                )
                os.system(
                    "wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.1/src/hdf5-1.12.1.tar.gz"
                )

        make()

        # Move wgrib2 executable to a tempoary directory
        mkdir("install")
        mkdir(join_path("install", "bin"))
        move(join_path("wgrib2", "wgrib2"), join_path("install", "bin"))

        # Build wgrib2 library by disabling all options
        # and enabling only MAKE_FTN_API=1
        if "+fortran_api" in spec:
            make("clean")
            make("deep-clean")
            makefile = FileFilter("makefile")

            # Disable all options
            for variant_name, makefile_option in variant_map.items():
                value = 0
                makefile.filter(
                    r"^%s=.*" % makefile_option, "{}={}".format(makefile_option, value)
                )

            # Need USE_REGEX in addition to MAKE_FTN_API to build lib
            makefile.filter(r"^MAKE_FTN_API=.*", "MAKE_FTN_API=1")
            makefile.filter(r"^USE_REGEX=.*", "USE_REGEX=1")
            make("lib")
            mkdir(join_path("install", "lib"))
            mkdir(join_path("install", "include"))

            move(join_path("lib", "libwgrib2.a"), join_path("install", "lib"))
            move(join_path("lib", "wgrib2api.mod"), join_path("install", "include"))
            move(join_path("lib", "wgrib2lowapi.mod"), join_path("install", "include"))

    def install(self, pkg, spec, prefix):
        install_tree("install/", prefix)
