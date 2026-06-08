/**
 * This is a test for the wgrib2 project. This test is for all 
 * of the same_sec*() routines in test_sec.c.
 * 
 * Alyson Stahl, 3/2026
 */

#include <stdio.h>
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"

int
main()
{
    printf("Testing same_sec0()...\n");
    {
        unsigned char sec0_a[16] = {
            'G', 'R', 'I', 'B', 
            0, 0,                   /* reserved */
            0, 2,                   /* discipline = 0, edition = 2 */
            0, 0, 0, 0, 0, 0, 0, 0  /* total length (not checked) */
        };
        unsigned char sec0_b[16] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        /**
         * same_sec0() does not compare beyond byte 8 (total length). 
         * Test will break if same_sec0() ever compares all 16 bytes.
         */
        for (int i = 0; i < 8; i++) sec0_b[i] = sec0_a[i];
        for (int i = 8; i < 16; i++) sec0_b[i] = 255;
        
        sec_a[0] = sec0_a;
        sec_b[0] = sec0_b;

        if (!same_sec0(sec_a, sec_b)) {
            printf("same_sec0: sections should be the same\n");
            return 1;
        }

        for (int i = 0; i < 8; i++) {
            sec0_b[i] = 255;
            if (same_sec0(sec_a, sec_b)) {
                printf("same_sec0: sections should be different at byte %d\n", i);
                return 1;
            }
            sec0_b[i] = sec0_a[i];
        }
    }
    printf("Testing same_sec0_not_var()...\n");
    {
        unsigned char sec0_a[16] = {
            'G', 'R', 'I', 'B', 
            0, 0,                   /* reserved */
            0, 2,                   /* discipline = 0, edition = 2 */
            0, 0, 0, 0, 0, 0, 0, 0  /* total length (not checked) */
        };
        unsigned char sec0_b[16] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        /**
         * same_sec0() does not compare beyond byte 8 (total length). 
         * Test will break if same_sec0() ever compares all 16 bytes.
         */
        for (int i = 0; i < 8; i++) sec0_b[i] = sec0_a[i];
        for (int i = 8; i < 16; i++) sec0_b[i] = 255;
        
        sec_a[0] = sec0_a;
        sec_b[0] = sec0_b;

        if (!same_sec0_not_var(1, sec_a, sec_b)) {
            printf("same_sec0_not_var: sections should be the same\n");
            return 1;
        }

        /* different edition, should be skipped */
        sec0_b[7] = 1;
        if (!same_sec0_not_var(1, sec_a, sec_b)) {
            printf("same_sec0_not_var: different edition should return 1\n");
            return 1;
        }
        sec0_b[7] = sec0_a[7];

        for (int i = 0; i < 8; i++) {
            if (i == 7) continue; /* ignores GRIB variable */
            sec0_b[i] = 255;
            if (same_sec0_not_var(1, sec_a, sec_b)) {
                printf("same_sec0_not_var: sections should be different at byte %d\n", i);
                return 1;
            }
            sec0_b[i] = sec0_a[i];
        }
    }
    printf("Testing same_sec1()...\n");
    {
        unsigned char sec1_a[21] = {
            0, 0, 0, 21,    /* Section length */
            1,              /* Section number */
            0, 7,           /* Center */
            0, 14,          /* Subcenter */
            2, 0,           /* Master table and local table */
            0,              /* Sig. of ref. time*/
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0, /* Month, day, hour, minute, second */
            0, 0            /* Prod status & type of processed data */
        };
        unsigned char sec1_b[21] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 21; i++) sec1_b[i] = sec1_a[i];

        sec_a[1] = sec1_a;
        sec_b[1] = sec1_b;

        if (!same_sec1(sec_a, sec_b)) {
            printf("same_sec1: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec1_b[3] = 20; 
        if (same_sec1(sec_a, sec_b)) {
            printf("same_sec1: different section length should return 0\n");
            return 1;
        }
        sec1_b[3] = sec1_a[3];

        for (int i = 4; i < 21; i++) {
            sec1_b[i] = 255;
            if (same_sec1(sec_a, sec_b)) {
                printf("same_sec1: sections should be different at byte %d\n", i);
                return 1;
            }
            sec1_b[i] = sec1_a[i];
        }
    }
    printf("testing same_sec1_not_time()...\n");
    {
        unsigned char sec1_a[21] = {
            0, 0, 0, 21,    /* Section length */
            1,              /* Section number */
            0, 7,           /* Center */
            0, 14,          /* Subcenter */
            2, 0,           /* Master table and local table */
            0,              /* Sig. of ref. time*/
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0, /* Month, day, hour, minute, second */
            0, 0            /* Prod status & type of processed data */
        };
        unsigned char sec1_b[21] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 21; i++) sec1_b[i] = sec1_a[i];

        sec_a[1] = sec1_a;
        sec_b[1] = sec1_b;

        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec1_b[3] = 20; 
        if (same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different section length should return 0\n");
            return 1;
        }
        sec1_b[3] = sec1_a[3];

        for (int i = 4; i < 21; i++) {
            if (i >= 12 && i < 19) continue; /* ignores time stamp */
            sec1_b[i] = 255;
            if (same_sec1_not_time(1, sec_a, sec_b)) {
                printf("same_sec1_not_time: sections should be different at byte %d\n", i);
                return 1;
            }
            sec1_b[i] = sec1_a[i];
        }

        /* different year */
        sec1_b[12] = (2024 >> 8) & 0xff;
        sec1_b[13] = 2024 & 0xff;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different year should return 1\n");
            return 1;
        }
        sec1_b[12] = sec1_a[12];
        sec1_b[13] = sec1_a[13];

        /* different month */
        sec1_b[14] = 4;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different month should return 1\n");
            return 1;
        }
        sec1_b[14] = sec1_a[14];

        /* different day */
        sec1_b[15] = 20;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different day should return 1\n");
            return 1;
        }
        sec1_b[15] = sec1_a[15];

        /* different hour */
        sec1_b[16] = 18;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different hour should return 1\n");
            return 1;
        }
        sec1_b[16] = sec1_a[16];

        /* different minute */
        sec1_b[17] = 30;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different minute should return 1\n");
            return 1;
        }
        sec1_b[17] = sec1_a[17];

        /* different second */
        sec1_b[18] = 45;
        if (!same_sec1_not_time(1, sec_a, sec_b)) {
            printf("same_sec1_not_time: different second should return 1\n");
            return 1;
        }
        sec1_b[18] = sec1_a[18];

    }
    printf("Testing same_sec1_not_var()...\n");
    {
        unsigned char sec1_a[21] = {
            0, 0, 0, 21,    /* Section length */
            1,              /* Section number */
            0, 7,           /* Center */
            0, 14,          /* Subcenter */
            2, 0,           /* Master table and local table */
            0,              /* Sig. of ref. time*/
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0, /* Month, day, hour, minute, second */
            0, 0            /* Prod status & type of processed data */
        };
        unsigned char sec1_b[21] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 21; i++) sec1_b[i] = sec1_a[i];

        sec_a[1] = sec1_a;
        sec_b[1] = sec1_b;

        if (!same_sec1_not_var(1, sec_a, sec_b)) {
            printf("same_sec1_not_var: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec1_b[3] = 20;
        if (same_sec1_not_var(1, sec_a, sec_b)) {
            printf("same_sec1_not_var: different section length should return 0\n");
            return 1;
        }
        sec1_b[3] = sec1_a[3];

        for (int i = 4; i < 21; i++) {
            if (i == 9 || i == 10) continue; /* ignores master table and local table */
            sec1_b[i] = 255;
            if (same_sec1_not_var(1, sec_a, sec_b)) {
                printf("same_sec1_not_var: sections should be different at byte %d\n", i);
                return 1;
            }
            sec1_b[i] = sec1_a[i];
        }

        /* different master table */
        sec1_b[9] = 0;
        if (!same_sec1_not_var(1, sec_a, sec_b)) {
            printf("same_sec1_not_var: different master table should return 1\n");
            return 1;
        }
        sec1_b[9] = sec1_a[9];

        /* different local table */
        sec1_b[10] = 255;
        if (!same_sec1_not_var(1, sec_a, sec_b)) {
            printf("same_sec1_not_var: different local table should return 1\n");
            return 1;
        }
        sec1_b[10] = sec1_a[10];
    }
    printf("Testing same_sec2()...\n");
    {
        unsigned char sec2_a[6] = {
            0, 0, 0, 6, /* Section length */
            2,          /* Section number */
            0           /* Local Use */
        };
        unsigned char sec2_b[6] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 6; i++) sec2_b[i] = sec2_a[i];

        sec_a[2] = sec2_a;
        sec_b[2] = sec2_b;

        if (!same_sec2(sec_a, sec_b)) {
            printf("same_sec2: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec2_b[3] = 5;
        if (same_sec2(sec_a, sec_b)) {
            printf("same_sec2: different section length should return 0\n");
            return 1;
        }
        sec2_b[3] = sec2_a[3];

        for (int i = 4; i < 6; i++) {
            sec2_b[i] = 255;
            if (same_sec2(sec_a, sec_b)) {
                printf("same_sec2: sections should be different at byte %d\n", i);
                return 1;
            }
            sec2_b[i] = sec2_a[i];
        }
    }
    printf("Testing same_sec3()...\n");
    {
        unsigned char sec3_a[15] = {
            0, 0, 0, 15,    /* Section length */
            3,              /* Section number */
            0,              /* Source of GDT */
            0, 0, 0, 0,     /* Number of data points */
            0,              /* Num octets for optional list */
            0,              /* Interpretation of optional list */
            0, 0,           /* Grid definition template number */
            0               /* Template */
        };
        unsigned char sec3_b[15] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 15; i++) sec3_b[i] = sec3_a[i];

        sec_a[3] = sec3_a;
        sec_b[3] = sec3_b;

        if (!same_sec3(sec_a, sec_b)) {
            printf("same_sec3: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec3_b[3] = 14;
        if (same_sec3(sec_a, sec_b)) {
            printf("same_sec3: different section length should return 0\n");
            return 1;
        }
        sec3_b[3] = sec3_a[3];

        for (int i = 6; i < 15; i++) {
            sec3_b[i] = 255;
            if (same_sec3(sec_a, sec_b)) {
                printf("same_sec3: sections should be different at byte %d\n", i);
                return 1;
            }
            sec3_b[i] = sec3_a[i];
        }
    }
    printf("Testing same_sec4()...\n");
    {
        unsigned char sec4_a[20] = {0};
        unsigned char sec4_b[20] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        sec4_a[3] = 20; /* section length */
        sec4_b[3] = 20;
        sec4_a[4] = 4;  /* section number */
        sec4_b[4] = 4;

        for (int i = 0; i < 20; i++) sec4_b[i] = sec4_a[i];

        sec_a[4] = sec4_a;
        sec_b[4] = sec4_b;

        if (!same_sec4(sec_a, sec_b)) {
            printf("same_sec4: sections should be the same\n");
            return 1;
        }

        /* different section length */
        sec4_b[3] = 19;
        if (same_sec4(sec_a, sec_b)) {
            printf("same_sec4: different section length should return 0\n");
            return 1;
        }
        sec4_b[3] = sec4_a[3];

        for (int i = 10; i < 20; i++) {
            sec4_b[i] = 255;
            if (same_sec4(sec_a, sec_b)) {
                printf("same_sec4: sections should be different at byte %d\n", i);
                return 1;
            }
            sec4_b[i] = sec4_a[i];
        }
    }
    printf("Testing same_sec4_not_time()...\n");
    {
        unsigned char sec1[21] = {0};
        unsigned char sec4_a[46] = {
            0, 0, 0, 46,    /* Section length */
            4,              /* Section number */
            0, 0,           /* Num coord values after template */
            0, 8,           /* Product definition template number */
            /* Product Definition Template 8 */
            0, 0,           /* Parameter category and number */
            0, 0, 0,        /* Generating process info */
            0, 0,           /* Hours after ref time cutoff */
            0,              /* Minutes after ref time cutoff */
            0,              /* Indicator of unit of time range */
            0, 0, 0, 0,     /* Forecast time in units specified by PDT */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of first fixed surface */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of second fixed surface */
            /* Time of end of overall time interval */
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0,                   /* Month, day, hour, minute, second */
            /* Rest of Template */
            0, 0, 0, 0, 0
        };
        unsigned char sec4_b[46] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 46; i++) sec4_b[i] = sec4_a[i];

        sec_a[1] = sec1; /* Section 1 is accessed for Center */
        sec_b[1] = sec1;
        sec_a[4] = sec4_a;
        sec_b[4] = sec4_b;

        /* First testing on PDT with stat time (PDT 8) */

        if (!same_sec4_not_time(1, sec_a, sec_b)) {
            printf("same_sec4_not_time: sections should be the same\n");
            return 1;
        } 

        /* different section length */
        sec4_b[3] = 45;
        if (same_sec4_not_time(1, sec_a, sec_b)) {
            printf("same_sec4_not_time: different section length should return 0\n");
            return 1;
        }
        sec4_b[3] = sec4_a[3];
  
        for (int i = 4; i < 46; i++) {
            sec4_b[i] = 255;
            if (i >= 17 && i < 22) {
                if (!same_sec4_not_time(1, sec_a, sec_b)) {
                    printf("same_sec4_not_time: different forecast time should return 1\n");
                    return 1;
                }
            }
            else if (i >= 34 && i < 41) {
                if (!same_sec4_not_time(1, sec_a, sec_b)) {
                    printf("same_sec4_not_time: different end of overall time interval should return 1\n");
                    return 1;
                }
            }
            else {
                if (same_sec4_not_time(1, sec_a, sec_b)) {
                    printf("same_sec4_not_time: sections should be different at byte %d\n", i);
                    return 1;
                }
            }
            sec4_b[i] = sec4_a[i];
        }
        
        /* Now testing on PDT without stat time (PDT 7) */
        sec4_a[3] = 34; /* section length */
        sec4_b[3] = 34;

        sec4_a[8] = 7; /* PDT Number */
        sec4_b[8] = 7;

        if (!same_sec4_not_time(1, sec_a, sec_b)) {
            printf("same_sec4_not_time: sections should be the same\n");
            return 1;
        } 

        for (int i = 4; i < 34; i++) {
            sec4_b[i] = 255;
            if (i >= 17 && i < 22) {
                if (!same_sec4_not_time(1, sec_a, sec_b)) {
                    printf("same_sec4_not_time: different forecast time should return 1\n");
                    return 1;
                }
            }
            else {
                if (same_sec4_not_time(1, sec_a, sec_b)) {
                    printf("same_sec4_not_time: sections should be different at byte %d\n", i);
                    return 1;
                }
            }
            sec4_b[i] = sec4_a[i];
        }

        /* Now testing on PDT without code table 4.4 (PDT 35)*/
        sec4_a[3] = 15; /* section length */
        sec4_b[3] = 15;

        sec4_a[8] = 35; /* PDT Number */
        sec4_b[8] = 35;

        if (!same_sec4_not_time(1, sec_a, sec_b)) {
            printf("same_sec4_not_time: sections should be the same\n");
            return 1;
        }

        for (int i = 4; i < 15; i++) {
            sec4_b[i] = 255;
            if (same_sec4_not_time(1, sec_a, sec_b)) {
                printf("same_sec4_not_time: sections should be different at byte %d\n", i);
                return 1;
            }
            sec4_b[i] = sec4_a[i];
        }
    }
    printf("Testing same_sec4_diff_ave_period()...\n");
    {
        unsigned char sec1[21] = {0};
        unsigned char sec4_a[58] = {
            0, 0, 0, 58,    /* Section length */
            4,              /* Section number */
            0, 0,           /* Num coord values after template */
            0, 8,           /* Product definition template number */
            /* Product Definition Template 8 */
            0, 0,           /* Parameter category and number */
            0, 0, 0,        /* Generating process info */
            0, 0,           /* Hours after ref time cutoff */
            0,              /* Minutes after ref time cutoff */
            0,              /* Indicator of unit of time range */
            0, 0, 0, 0,     /* Forecast time in units specified by PDT */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of first fixed surface */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of second fixed surface */
            /* Time of end of overall time interval */
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0,    /* Month, day, hour, minute, second */
            /* Rest of Template */
            0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0
        };
        unsigned char sec4_b[58] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 58; i++) sec4_b[i] = sec4_a[i];

        sec_a[1] = sec1; /* Section 1 is accessed for Center */
        sec_b[1] = sec1;
        sec_a[4] = sec4_a;
        sec_b[4] = sec4_b;

        if (!same_sec4_diff_ave_period(sec_a, sec_b)) {
            printf("same_sec4_diff_ave_period: sections should be the same\n");
            return 1;
        } 

        /* different section length */
        sec4_b[3] = 57;
        if (same_sec4_diff_ave_period(sec_a, sec_b)) {
            printf("same_sec4_diff_ave_period: different section length should return 0\n");
            return 1;
        }
        sec4_b[3] = sec4_a[3];

        for (int i = 4; i < 58; i++) {
            sec4_b[i] = 255;
            if (i >= 34 && i < 41) {
                if (!same_sec4_diff_ave_period(sec_a, sec_b)) {
                    printf("same_sec4_diff_ave_period: different end of overall time interval should return 1\n");
                    return 1;
                }
            }
            else if (i >= 49 && i < 53) {
                if (!same_sec4_diff_ave_period(sec_a, sec_b)) {
                    printf("same_sec4_diff_ave_period: different time range return 1\n");
                    return 1;
                }
            }
            else {
                if (same_sec4_diff_ave_period(sec_a, sec_b)) {
                    printf("same_sec4_diff_ave_period: sections should be different at byte %d\n", i);
                    return 1;
                }
            }
            sec4_b[i] = sec4_a[i];
        }
        
        /* Now testing on PDT without stat time (PDT 7) */
        sec4_a[3] = 34; /* section length */
        sec4_b[3] = 34;

        sec4_a[8] = 7; /* PDT Number */
        sec4_b[8] = 7;

        if (same_sec4_diff_ave_period(sec_a, sec_b)) {
            printf("same_sec4_diff_ave_period: PDT without stat time should return 0\n");
            return 1;
        }
    }
    printf("Testing same_sec4_for_merge()...\n");
    {
        unsigned char sec1[21] = {0};
        unsigned char sec4_a[58] = {
            0, 0, 0, 58,    /* Section length */
            4,              /* Section number */
            0, 0,           /* Num coord values after template */
            0, 8,           /* Product definition template number */
            /* Product Definition Template 8 */
            0, 0,           /* Parameter category and number */
            0, 0, 0,        /* Generating process info */
            0, 0,           /* Hours after ref time cutoff */
            0,              /* Minutes after ref time cutoff */
            0,              /* Indicator of unit of time range */
            0, 0, 0, 0,     /* Forecast time in units specified by PDT */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of first fixed surface */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of second fixed surface */
            /* Time of end of overall time interval */
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0,    /* Month, day, hour, minute, second */
            /* Rest of Template */
            0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0
        };
        unsigned char sec4_b[58] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 58; i++) sec4_b[i] = sec4_a[i];

        sec_a[1] = sec1; /* Section 1 is accessed for Center */
        sec_b[1] = sec1;
        sec_a[4] = sec4_a;
        sec_b[4] = sec4_b;

        if (!same_sec4_for_merge(1, sec_a, sec_b)) {
            printf("same_sec4_for_merge: sections should be the same\n");
            return 1;
        } 

        /* different section length */
        sec4_b[3] = 57;
        if (same_sec4_for_merge(1, sec_a, sec_b)) {
            printf("same_sec4_for_merge: different section length should return 0\n");
            return 1;
        }
        sec4_b[3] = sec4_a[3];
  
        for (int i = 4; i < 58; i++) {
            sec4_b[i] = 255;
            if (i >= 18 && i < 22) {
                if (!same_sec4_for_merge(1, sec_a, sec_b)) {
                    printf("same_sec4_for_merge: different forecast time should return 1. Failed at index %d\n", i);
                    return 1;
                }
            }
            else if (i >= 34 && i < 41) {
                if (!same_sec4_for_merge(1, sec_a, sec_b)) {
                    printf("same_sec4_for_merge: different end of overall time interval should return 1. Failed at index %d\n", i);
                    return 1;
                }
            }
            else {
                if (same_sec4_for_merge(1, sec_a, sec_b)) {
                    printf("same_sec4_for_merge: sections should be different at byte %d\n", i);
                    return 1;
                }
            }
            sec4_b[i] = sec4_a[i];
        }

        /* Now testing on PDT without stat time (PDT 7) */
        sec4_a[3] = 34; /* section length */
        sec4_b[3] = 34;

        sec4_a[8] = 7; /* PDT Number */
        sec4_b[8] = 7;

        if (same_sec4_for_merge(1, sec_a, sec_b)) {
            printf("same_sec4_for_merge: PDT without stat time should return 0\n");
            return 1;
        }
    }
    printf("Testing same_sec4_but_ensemble()...\n");
    {
        unsigned char sec1[21] = {0};
        unsigned char sec4_a[39] = {
            0, 0, 0, 39,    /* Section length */
            4,              /* Section number */
            0, 0,           /* Num coord values after template */
            0, 41,          /* Product definition template number */
            /* Product Definition Template 41 */
            0, 0,           /* Parameter category and number */
            0, 0,           /* Atmospheric Chemical Constituent Type */
            0, 0, 0,        /* Generating process info */
            0, 0,           /* Hours after ref time cutoff */
            0,              /* Minutes after ref time cutoff */
            0,              /* Indicator of unit of time range (Code Table 4.4) */
            0, 0, 0, 0,     /* Forecast time in units specified by PDT */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of first fixed surface */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of second fixed surface */
            0,              /* Type of ensemble forecast */
            0,              /* Perturbation number */
            0,              /* Number of forecasts in ensemble */
        };
        unsigned char sec4_b[39] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 39; i++) sec4_b[i] = sec4_a[i];

        sec_a[1] = sec1; /* Section 1 is accessed for Center */
        sec_b[1] = sec1;
        sec_a[4] = sec4_a;
        sec_b[4] = sec4_b;

        /* use mode = 98 for coverage */
        if (!same_sec4_but_ensemble(98, sec_a, sec_b)) {
            printf("same_sec4_but_ensemble: sections should be the same\n");
            return 1;
        } 

        /* different section length */
        sec4_b[3] = 38;
        if (same_sec4_but_ensemble(1, sec_a, sec_b)) {
            printf("same_sec4_but_ensemble: different section length should return 0\n");
            return 1;
        }
        sec4_b[3] = sec4_a[3];

        for (int i = 4; i < 39; i++) {
            sec4_b[i] = 255;
            if (i == 19 || (i >= 20 && i < 24) || i == 36 || i == 37) {
                if (!same_sec4_but_ensemble(1, sec_a, sec_b)) {
                    if (i == 19) {
                        printf("same_sec4_but_ensemble: different indicator of unit time range should return 1\n");
                    }
                    else if (i >= 20 && i < 24) {
                        printf("same_sec4_but_ensemble: different forecast time should return 1. Failed at index %d\n", i);
                    }
                    else if (i == 36) {
                        printf("same_sec4_but_ensemble: different type of ensemble forecast should return 1\n");
                    }
                    else {
                        printf("same_sec4_but_ensemble: different perturbation number should return 1\n");
                    }
                    return 1;
                }
            }
            else {
                if (same_sec4_but_ensemble(1, sec_a, sec_b)) {
                    printf("same_sec4_but_ensemble: sections should be different at byte %d\n", i);
                    return 1;
                }
            }
            sec4_b[i] = sec4_a[i];
        }
    }
    printf("Testing same_sec4_not_var()...\n");
    {
        unsigned char sec1[21] = {0};
        unsigned char sec4_a[34] = {
            0, 0, 0, 34,    /* Section length */
            4,              /* Section number */
            0, 0,           /* Num coord values after template */
            0, 7,           /* Product definition template number */
            /* Product Definition Template 7 */
            0, 0,           /* Parameter category and number */
            0, 0, 0,        /* Generating process info */
            0, 0,           /* Hours after ref time cutoff */
            0,              /* Minutes after ref time cutoff */
            0,              /* Indicator of unit of time range */
            0, 0, 0, 0,     /* Forecast time in units specified by PDT */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of first fixed surface */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of second fixed surface */
        };
        unsigned char sec4_b[34] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 34; i++) sec4_b[i] = sec4_a[i];

        sec_a[1] = sec1; /* Section 1 is accessed for Center */
        sec_b[1] = sec1;
        sec_a[4] = sec4_a;
        sec_b[4] = sec4_b;

        if (!same_sec4_not_var(1, sec_a, sec_b)) {
            printf("same_sec4_not_var: sections should be the same\n");
            return 1;
        } 

        /* different section length */
        sec4_b[3] = 33;
        if (same_sec4_not_var(1, sec_a, sec_b)) {
            printf("same_sec4_not_var: different section length should return 0\n");
            return 1;
        }
        sec4_b[3] = sec4_a[3];
  
        for (int i = 4; i < 34; i++) {
            sec4_b[i] = 255;
            if (i == 9) {
                if (!same_sec4_not_var(1, sec_a, sec_b)) {
                    printf("same_sec4_not_var: different parameter category should return 1\n");
                    return 1;
                }
            }
            else if (i == 10) {
                if (!same_sec4_not_var(1, sec_a, sec_b)) {
                    printf("same_sec4_not_var: different parameter number should return 1\n");
                    return 1;
                }
            }
            else {
                if (same_sec4_not_var(1, sec_a, sec_b)) {
                    printf("same_sec4_not_var: sections should be different at byte %d\n", i);
                    return 1;
                }
            }
            sec4_b[i] = sec4_a[i];
        }
    }
    printf("Testing same_sec4_unmerge_fcst()...\n");
    {
        unsigned char sec1[21] = {0};
        unsigned char sec4_a[58] = {
            0, 0, 0, 58,    /* Section length */
            4,              /* Section number */
            0, 0,           /* Num coord values after template */
            0, 8,           /* Product definition template number */
            /* Product Definition Template 8 */
            0, 0,           /* Parameter category and number */
            0, 0, 0,        /* Generating process info */
            0, 0,           /* Hours after ref time cutoff */
            0,              /* Minutes after ref time cutoff */
            0,              /* Indicator of unit of time range */
            0, 0, 0, 0,     /* Forecast time in units specified by PDT */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of first fixed surface */
            0, 0, 0, 0, 0, 0, /* Type, scale factor, scaled value of second fixed surface */
            /* Time of end of overall time interval */
            (2025 >> 8) & 0xff, (2025 & 0xff), /* Year */
            3, 15, 12, 0, 0,    /* Month, day, hour, minute, second */
            1,                  /* Number of time ranges */
            /* Rest of Template */
            0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0
        };
        unsigned char sec4_b[58] = {0};
        unsigned char *sec_a[10] = {0};
        unsigned char *sec_b[10] = {0};

        for (int i = 0; i < 58; i++) sec4_b[i] = sec4_a[i];

        sec_a[1] = sec1; /* Section 1 is accessed for Center */
        sec_b[1] = sec1;
        sec_a[4] = sec4_a;
        sec_b[4] = sec4_b;

        if (!same_sec4_unmerge_fcst(1, sec_a, sec_b)) {
            printf("same_sec4_unmerge_fcst: sections should be the same\n");
            return 1;
        } 

        for (int i = 4; i < 58; i++) {
            sec4_b[i] = 255;
            if (i >= 34 && i < 40) {
                if (!same_sec4_unmerge_fcst(1, sec_a, sec_b)) {
                    printf("same_sec4_unmerge_fcst: different verification time should return 1\n");
                    return 1;
                }
            }
            else if (i >= 49 && i < 53) {
                if (!same_sec4_unmerge_fcst(1, sec_a, sec_b)) {
                    printf("same_sec4_unmerge_fcst: different length of time range should return 1\n");
                    return 1;
                }
            }
            else {
                if (same_sec4_unmerge_fcst(1, sec_a, sec_b)) {
                    printf("same_sec4_unmerge_fcst: sections should be different at byte %d\n", i);
                    return 1;
                }
            }
            sec4_b[i] = sec4_a[i];
        }

        /* number of time ranges must be 1 */
        sec4_a[41] = 0;
        if (same_sec4_unmerge_fcst(1, sec_a, sec_b)) {
            printf("same_sec4_unmerge_fcst: number of time ranges n != 1 should return 0\n");
            return 1;
        }
        sec4_a[41] = 1;

        /* must have stat proc time */
        sec4_a[8] = 7; /* PDT Number */
        sec4_b[8] = 7;
        if (same_sec4_unmerge_fcst(1, sec_a, sec_b)) {
            printf("same_sec4_unmerge_fcst: PDT without stat time should return 0\n");
            return 1;   
        }
    }
    printf("SUCCESS!\n");
    return 0;
}