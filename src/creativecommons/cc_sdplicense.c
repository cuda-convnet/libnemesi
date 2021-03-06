/* *
 * This file is part of libnemesi
 *
 * Copyright (C) 2007 by LScube team <team@streaming.polito.it>
 * See AUTHORS for more details
 *
 * libnemesi is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libnemesi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libnemesi; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */

#include "config.h"
#include <strings.h>

#include "nemesi/cc.h"
#include "utils/utils.h"

#include "nemesi/comm.h"

static const char *const nms_cc_licenses[][2] = CC_LICENSE;

/*! \brief Set the correct field in License definition.
 *
 * The sdp_l string is in the form, coming from an sdp description,
 * <param>=<value>. According to <param>, we set the right field.  Warning the
 * function does't copy the string, it sets just the right pointer to sdp_l, so
 * the sdp_l parameter cannot be a temporary string.
 */
int cc_set_sdplicense(cc_license * cc, char *sdp_l)
{
    unsigned int i;

    // shawill: sizeof(nms_cc_licenses)/sizeof(*nms_cc_licenses) == number of couples name-description present
    for (i = 0; i < sizeof(nms_cc_licenses) / sizeof(*nms_cc_licenses); i++) {
        if (!strncasecmp
            (sdp_l, nms_cc_licenses[i][CC_ATTR_NAME],
             strlen(nms_cc_licenses[i][CC_ATTR_NAME]))) {
            // XXX: we do not duplicate the string!!! Do we have to do that?
            /* set the correct field using cc_license struct like an array of strings
             * skipping the sdp param and setting the pointer after the colon */
            ((char **) cc)[i] =
                sdp_l + strlen(nms_cc_licenses[i][CC_ATTR_NAME]) + 1;
            return 0;
        }
    }

    return 1;
}

int issdplicense(char *sdp_a)
{
    unsigned int i;

    // shawill: sizeof(nms_cc_licenses)/sizeof(*nms_cc_licenses) == number of couples name-description present
    for (i = 0; i < sizeof(nms_cc_licenses) / sizeof(*nms_cc_licenses); i++) {
        if (!strncasecmp
            (sdp_a, nms_cc_licenses[i][CC_ATTR_NAME],
             strlen(nms_cc_licenses[i][CC_ATTR_NAME]))) {
            nms_printf(NMSML_DBG1,
                   "found valid cc field in SDP description (%s - %s)\n",
                   nms_cc_licenses[i][CC_ATTR_NAME],
                   nms_cc_licenses[i][CC_ATTR_DESCR]);
            return 1;
        }
    }

    return 0;
}
