/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details:
 *
 * Copyright (C) 2012 Google, Inc.
 */

#include "mm-modem-helpers-qmi.h"
#include "mm-log.h"

/*****************************************************************************/

MMModemCapability
mm_modem_capability_from_qmi_radio_interface (QmiDmsRadioInterface network)
{
    switch (network) {
    case QMI_DMS_RADIO_INTERFACE_CDMA20001X:
        return MM_MODEM_CAPABILITY_CDMA_EVDO;
    case QMI_DMS_RADIO_INTERFACE_EVDO:
        return MM_MODEM_CAPABILITY_CDMA_EVDO;
    case QMI_DMS_RADIO_INTERFACE_GSM:
        return MM_MODEM_CAPABILITY_GSM_UMTS;
    case QMI_DMS_RADIO_INTERFACE_UMTS:
        return MM_MODEM_CAPABILITY_GSM_UMTS;
    case QMI_DMS_RADIO_INTERFACE_LTE:
        return MM_MODEM_CAPABILITY_LTE;
    default:
        mm_warn ("Unhandled QMI radio interface (%u)",
                 (guint)network);
        return MM_MODEM_CAPABILITY_NONE;
    }
}

/*****************************************************************************/

/* pin1 TRUE for PIN1, FALSE for PIN2 */
MMModemLock
mm_modem_lock_from_qmi_uim_pin_status (QmiDmsUimPinStatus status,
                                       gboolean pin1)
{
    switch (status) {
    case QMI_DMS_UIM_PIN_STATUS_NOT_INITIALIZED:
        return MM_MODEM_LOCK_UNKNOWN;
    case QMI_DMS_UIM_PIN_STATUS_ENABLED_NOT_VERIFIED:
        return pin1 ? MM_MODEM_LOCK_SIM_PIN : MM_MODEM_LOCK_SIM_PIN2;
    case QMI_DMS_UIM_PIN_STATUS_ENABLED_VERIFIED:
        return MM_MODEM_LOCK_NONE;
    case QMI_DMS_UIM_PIN_STATUS_DISABLED:
        return MM_MODEM_LOCK_NONE;
    case QMI_DMS_UIM_PIN_STATUS_BLOCKED:
        return pin1 ? MM_MODEM_LOCK_SIM_PUK : MM_MODEM_LOCK_SIM_PUK2;
    case QMI_DMS_UIM_PIN_STATUS_PERMANENTLY_BLOCKED:
        return MM_MODEM_LOCK_UNKNOWN;
    case QMI_DMS_UIM_PIN_STATUS_UNBLOCKED:
        /* This state is possibly given when after an Unblock() operation has been performed.
         * We'll assume the PIN is verified after this. */
        return MM_MODEM_LOCK_NONE;
    case QMI_DMS_UIM_PIN_STATUS_CHANGED:
        /* This state is possibly given when after an ChangePin() operation has been performed.
         * We'll assume the PIN is verified after this. */
        return MM_MODEM_LOCK_NONE;
    default:
        return MM_MODEM_LOCK_UNKNOWN;
    }
}

/*****************************************************************************/

QmiDmsUimFacility
mm_3gpp_facility_to_qmi_uim_facility (MMModem3gppFacility mm)
{
    switch (mm) {
    case MM_MODEM_3GPP_FACILITY_PH_SIM:
        /* Not really sure about this one; it may be PH_FSIM? */
        return QMI_DMS_UIM_FACILITY_PF;

    case MM_MODEM_3GPP_FACILITY_NET_PERS:
        return QMI_DMS_UIM_FACILITY_PN;

    case MM_MODEM_3GPP_FACILITY_NET_SUB_PERS:
        return QMI_DMS_UIM_FACILITY_PU;

    case MM_MODEM_3GPP_FACILITY_PROVIDER_PERS:
        return QMI_DMS_UIM_FACILITY_PP;

    case MM_MODEM_3GPP_FACILITY_CORP_PERS:
        return QMI_DMS_UIM_FACILITY_PC;

    default:
        /* Never try to ask for a facility we cannot translate */
        g_assert_not_reached ();
    }
}

/*****************************************************************************/

typedef struct {
    QmiDmsBandCapability qmi_band;
    MMModemBand mm_band;
} BandsMap;

static const BandsMap bands_map [] = {
    /* CDMA bands */
    {
        (QMI_DMS_BAND_CAPABILITY_BC_0_A_SYSTEM | QMI_DMS_BAND_CAPABILITY_BC_0_B_SYSTEM),
        MM_MODEM_BAND_CDMA_BC0_CELLULAR_800
    },
    { QMI_DMS_BAND_CAPABILITY_BC_1_ALL_BLOCKS, MM_MODEM_BAND_CDMA_BC1_PCS_1900       },
    { QMI_DMS_BAND_CAPABILITY_BC_2,            MM_MODEM_BAND_CDMA_BC2_TACS           },
    { QMI_DMS_BAND_CAPABILITY_BC_3_A_SYSTEM,   MM_MODEM_BAND_CDMA_BC3_JTACS          },
    { QMI_DMS_BAND_CAPABILITY_BC_4_ALL_BLOCKS, MM_MODEM_BAND_CDMA_BC4_KOREAN_PCS     },
    { QMI_DMS_BAND_CAPABILITY_BC_5_ALL_BLOCKS, MM_MODEM_BAND_CDMA_BC5_NMT450         },
    { QMI_DMS_BAND_CAPABILITY_BC_6,            MM_MODEM_BAND_CDMA_BC6_IMT2000        },
    { QMI_DMS_BAND_CAPABILITY_BC_7,            MM_MODEM_BAND_CDMA_BC7_CELLULAR_700   },
    { QMI_DMS_BAND_CAPABILITY_BC_8,            MM_MODEM_BAND_CDMA_BC8_1800           },
    { QMI_DMS_BAND_CAPABILITY_BC_9,            MM_MODEM_BAND_CDMA_BC9_900            },
    { QMI_DMS_BAND_CAPABILITY_BC_10,           MM_MODEM_BAND_CDMA_BC10_SECONDARY_800 },
    { QMI_DMS_BAND_CAPABILITY_BC_11,           MM_MODEM_BAND_CDMA_BC11_PAMR_400      },
    { QMI_DMS_BAND_CAPABILITY_BC_12,           MM_MODEM_BAND_CDMA_BC12_PAMR_800      },
    { QMI_DMS_BAND_CAPABILITY_BC_14,           MM_MODEM_BAND_CDMA_BC14_PCS2_1900     },
    { QMI_DMS_BAND_CAPABILITY_BC_15,           MM_MODEM_BAND_CDMA_BC15_AWS           },
    { QMI_DMS_BAND_CAPABILITY_BC_16,           MM_MODEM_BAND_CDMA_BC16_US_2500       },
    { QMI_DMS_BAND_CAPABILITY_BC_17,           MM_MODEM_BAND_CDMA_BC17_US_FLO_2500   },
    { QMI_DMS_BAND_CAPABILITY_BC_18,           MM_MODEM_BAND_CDMA_BC18_US_PS_700     },
    { QMI_DMS_BAND_CAPABILITY_BC_19,           MM_MODEM_BAND_CDMA_BC19_US_LOWER_700  },

    /* GSM bands */
    { QMI_DMS_BAND_CAPABILITY_GSM_DCS_1800,     MM_MODEM_BAND_DCS  },
    { QMI_DMS_BAND_CAPABILITY_GSM_900_EXTENDED, MM_MODEM_BAND_EGSM },
    { QMI_DMS_BAND_CAPABILITY_GSM_PCS_1900,     MM_MODEM_BAND_PCS  },
    { QMI_DMS_BAND_CAPABILITY_GSM_850,          MM_MODEM_BAND_G850 },

    /* UMTS/WCDMA bands */
    { QMI_DMS_BAND_CAPABILITY_WCDMA_2100,       MM_MODEM_BAND_U2100 },
    { QMI_DMS_BAND_CAPABILITY_WCDMA_DCS_1800,   MM_MODEM_BAND_U1800 },
    { QMI_DMS_BAND_CAPABILITY_WCDMA_1700_US,    MM_MODEM_BAND_U17IV },
    { QMI_DMS_BAND_CAPABILITY_WCDMA_800,        MM_MODEM_BAND_U800  },
    {
        (QMI_DMS_BAND_CAPABILITY_WCDMA_850_US | QMI_DMS_BAND_CAPABILITY_WCDMA_850_JAPAN),
        MM_MODEM_BAND_U850
    },
    { QMI_DMS_BAND_CAPABILITY_WCDMA_900,        MM_MODEM_BAND_U900  },
    { QMI_DMS_BAND_CAPABILITY_WCDMA_1700_JAPAN, MM_MODEM_BAND_U17IX },
    { QMI_DMS_BAND_CAPABILITY_WCDMA_2600,       MM_MODEM_BAND_U2600 }

    /* NOTE. The following bands were unmatched:
     *
     * - QMI_DMS_BAND_CAPABILITY_GSM_900_PRIMARY
     * - QMI_DMS_BAND_CAPABILITY_GSM_450
     * - QMI_DMS_BAND_CAPABILITY_GSM_480
     * - QMI_DMS_BAND_CAPABILITY_GSM_750
     * - QMI_DMS_BAND_CAPABILITY_GSM_900_RAILWAILS
     * - QMI_DMS_BAND_CAPABILITY_WCDMA_1500
     * - MM_MODEM_BAND_CDMA_BC13_IMT2000_2500
     * - MM_MODEM_BAND_U1900
     */
};

static void
add_qmi_bands (GArray *mm_bands,
               QmiDmsBandCapability qmi_bands)
{
    static QmiDmsBandCapability qmi_bands_expected = 0;
    QmiDmsBandCapability not_expected;
    guint i;

    g_assert (mm_bands != NULL);

    /* Build mask of expected bands only once */
    if (G_UNLIKELY (qmi_bands_expected == 0)) {
        for (i = 0; i < G_N_ELEMENTS (bands_map); i++) {
            qmi_bands_expected |= bands_map[i].qmi_band;
        }
    }

    /* Log about the bands that cannot be represented in ModemManager */
    not_expected = ((qmi_bands_expected ^ qmi_bands) & qmi_bands);
    if (not_expected) {
        gchar *aux;

        aux = qmi_dms_band_capability_build_string_from_mask (not_expected);
        mm_dbg ("Cannot add the following bands: '%s'", aux);
        g_free (aux);
    }

    /* And add the expected ones */
    for (i = 0; i < G_N_ELEMENTS (bands_map); i++) {
        if (qmi_bands & bands_map[i].qmi_band)
            g_array_append_val (mm_bands, bands_map[i].mm_band);
    }
}

typedef struct {
    QmiDmsLteBandCapability qmi_band;
    MMModemBand mm_band;
} LteBandsMap;

static const LteBandsMap lte_bands_map [] = {
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_1,  MM_MODEM_BAND_EUTRAN_I       },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_2,  MM_MODEM_BAND_EUTRAN_II      },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_3,  MM_MODEM_BAND_EUTRAN_III     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_4,  MM_MODEM_BAND_EUTRAN_IV      },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_5,  MM_MODEM_BAND_EUTRAN_V       },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_6,  MM_MODEM_BAND_EUTRAN_VI      },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_7,  MM_MODEM_BAND_EUTRAN_VII     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_8,  MM_MODEM_BAND_EUTRAN_VIII    },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_9,  MM_MODEM_BAND_EUTRAN_IX      },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_10, MM_MODEM_BAND_EUTRAN_X       },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_11, MM_MODEM_BAND_EUTRAN_XI      },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_12, MM_MODEM_BAND_EUTRAN_XII     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_13, MM_MODEM_BAND_EUTRAN_XIII    },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_14, MM_MODEM_BAND_EUTRAN_XIV     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_17, MM_MODEM_BAND_EUTRAN_XVII    },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_18, MM_MODEM_BAND_EUTRAN_XVIII   },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_19, MM_MODEM_BAND_EUTRAN_XIX     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_20, MM_MODEM_BAND_EUTRAN_XX      },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_21, MM_MODEM_BAND_EUTRAN_XXI     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_24, MM_MODEM_BAND_EUTRAN_XXIV    },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_25, MM_MODEM_BAND_EUTRAN_XXV     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_33, MM_MODEM_BAND_EUTRAN_XXXIII  },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_34, MM_MODEM_BAND_EUTRAN_XXXIV   },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_35, MM_MODEM_BAND_EUTRAN_XXXV    },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_36, MM_MODEM_BAND_EUTRAN_XXXVI   },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_37, MM_MODEM_BAND_EUTRAN_XXXVII  },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_38, MM_MODEM_BAND_EUTRAN_XXXVIII },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_39, MM_MODEM_BAND_EUTRAN_XXXIX   },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_40, MM_MODEM_BAND_EUTRAN_XL      },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_41, MM_MODEM_BAND_EUTRAN_XLI     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_42, MM_MODEM_BAND_EUTRAN_XLI     },
    { QMI_DMS_LTE_BAND_CAPABILITY_EUTRAN_43, MM_MODEM_BAND_EUTRAN_XLIII   }

    /* NOTE. The following bands were unmatched:
     *
     * - MM_MODEM_BAND_EUTRAN_XXII
     * - MM_MODEM_BAND_EUTRAN_XXIII
     * - MM_MODEM_BAND_EUTRAN_XXVI
     */
};

static void
add_qmi_lte_bands (GArray *mm_bands,
                   QmiDmsLteBandCapability qmi_bands)
{
    /* All QMI LTE bands have a counterpart in ModemManager, no need to check
     * for unexpected ones */
    guint i;

    g_assert (mm_bands != NULL);

    for (i = 0; i < G_N_ELEMENTS (lte_bands_map); i++) {
        if (qmi_bands & lte_bands_map[i].qmi_band)
            g_array_append_val (mm_bands, lte_bands_map[i].mm_band);
    }
}

GArray *
mm_modem_bands_from_qmi_band_capabilities (QmiDmsBandCapability qmi_bands,
                                           QmiDmsLteBandCapability qmi_lte_bands)
{
    GArray *mm_bands;

    mm_bands = g_array_new (FALSE, FALSE, sizeof (MMModemBand));
    add_qmi_bands (mm_bands, qmi_bands);
    add_qmi_lte_bands (mm_bands, qmi_lte_bands);

    return mm_bands;
}

/*****************************************************************************/

typedef struct {
    QmiNasActiveBand qmi_band;
    MMModemBand mm_band;
} ActiveBandsMap;

static const ActiveBandsMap active_bands_map [] = {
    /* CDMA bands */
    { QMI_NAS_ACTIVE_BAND_BC_0,  MM_MODEM_BAND_CDMA_BC0_CELLULAR_800   },
    { QMI_NAS_ACTIVE_BAND_BC_1,  MM_MODEM_BAND_CDMA_BC1_PCS_1900       },
    { QMI_NAS_ACTIVE_BAND_BC_2,  MM_MODEM_BAND_CDMA_BC2_TACS           },
    { QMI_NAS_ACTIVE_BAND_BC_3,  MM_MODEM_BAND_CDMA_BC3_JTACS          },
    { QMI_NAS_ACTIVE_BAND_BC_4,  MM_MODEM_BAND_CDMA_BC4_KOREAN_PCS     },
    { QMI_NAS_ACTIVE_BAND_BC_5,  MM_MODEM_BAND_CDMA_BC5_NMT450         },
    { QMI_NAS_ACTIVE_BAND_BC_6,  MM_MODEM_BAND_CDMA_BC6_IMT2000        },
    { QMI_NAS_ACTIVE_BAND_BC_7,  MM_MODEM_BAND_CDMA_BC7_CELLULAR_700   },
    { QMI_NAS_ACTIVE_BAND_BC_8,  MM_MODEM_BAND_CDMA_BC8_1800           },
    { QMI_NAS_ACTIVE_BAND_BC_9,  MM_MODEM_BAND_CDMA_BC9_900            },
    { QMI_NAS_ACTIVE_BAND_BC_10, MM_MODEM_BAND_CDMA_BC10_SECONDARY_800 },
    { QMI_NAS_ACTIVE_BAND_BC_11, MM_MODEM_BAND_CDMA_BC11_PAMR_400      },
    { QMI_NAS_ACTIVE_BAND_BC_12, MM_MODEM_BAND_CDMA_BC12_PAMR_800      },
    { QMI_NAS_ACTIVE_BAND_BC_13, MM_MODEM_BAND_CDMA_BC13_IMT2000_2500  },
    { QMI_NAS_ACTIVE_BAND_BC_14, MM_MODEM_BAND_CDMA_BC14_PCS2_1900     },
    { QMI_NAS_ACTIVE_BAND_BC_15, MM_MODEM_BAND_CDMA_BC15_AWS           },
    { QMI_NAS_ACTIVE_BAND_BC_16, MM_MODEM_BAND_CDMA_BC16_US_2500       },
    { QMI_NAS_ACTIVE_BAND_BC_17, MM_MODEM_BAND_CDMA_BC17_US_FLO_2500   },
    { QMI_NAS_ACTIVE_BAND_BC_18, MM_MODEM_BAND_CDMA_BC18_US_PS_700     },
    { QMI_NAS_ACTIVE_BAND_BC_19, MM_MODEM_BAND_CDMA_BC19_US_LOWER_700  },

    /* GSM bands */
    { QMI_NAS_ACTIVE_BAND_GSM_850, MM_MODEM_BAND_G850          },
    { QMI_NAS_ACTIVE_BAND_GSM_900_EXTENDED, MM_MODEM_BAND_EGSM },
    { QMI_NAS_ACTIVE_BAND_GSM_DCS_1800, MM_MODEM_BAND_DCS      },
    { QMI_NAS_ACTIVE_BAND_GSM_PCS_1900, MM_MODEM_BAND_PCS      },

    /* WCDMA bands */
    { QMI_NAS_ACTIVE_BAND_WCDMA_2100, MM_MODEM_BAND_U2100       },
    { QMI_NAS_ACTIVE_BAND_WCDMA_PCS_1900, MM_MODEM_BAND_PCS     },
    { QMI_NAS_ACTIVE_BAND_WCDMA_DCS_1800, MM_MODEM_BAND_DCS     },
    { QMI_NAS_ACTIVE_BAND_WCDMA_1700_US, MM_MODEM_BAND_U17IV    },
    { QMI_NAS_ACTIVE_BAND_WCDMA_850, MM_MODEM_BAND_U850         },
    { QMI_NAS_ACTIVE_BAND_WCDMA_800, MM_MODEM_BAND_U800         },
    { QMI_NAS_ACTIVE_BAND_WCDMA_2600, MM_MODEM_BAND_U2600       },
    { QMI_NAS_ACTIVE_BAND_WCDMA_900, MM_MODEM_BAND_U900         },
    { QMI_NAS_ACTIVE_BAND_WCDMA_1700_JAPAN, MM_MODEM_BAND_U17IX },
    { QMI_NAS_ACTIVE_BAND_WCDMA_850_JAPAN, MM_MODEM_BAND_U850   },

    /* LTE bands */
    { QMI_NAS_ACTIVE_BAND_EUTRAN_1,  MM_MODEM_BAND_EUTRAN_I       },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_2,  MM_MODEM_BAND_EUTRAN_II      },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_3,  MM_MODEM_BAND_EUTRAN_III     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_4,  MM_MODEM_BAND_EUTRAN_IV      },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_5,  MM_MODEM_BAND_EUTRAN_V       },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_6,  MM_MODEM_BAND_EUTRAN_VI      },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_7,  MM_MODEM_BAND_EUTRAN_VII     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_8,  MM_MODEM_BAND_EUTRAN_VIII    },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_9,  MM_MODEM_BAND_EUTRAN_IX      },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_10, MM_MODEM_BAND_EUTRAN_X       },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_11, MM_MODEM_BAND_EUTRAN_XI      },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_12, MM_MODEM_BAND_EUTRAN_XII     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_13, MM_MODEM_BAND_EUTRAN_XIII    },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_14, MM_MODEM_BAND_EUTRAN_XIV     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_17, MM_MODEM_BAND_EUTRAN_XVII    },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_18, MM_MODEM_BAND_EUTRAN_XVIII   },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_19, MM_MODEM_BAND_EUTRAN_XIX     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_20, MM_MODEM_BAND_EUTRAN_XX      },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_21, MM_MODEM_BAND_EUTRAN_XXI     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_24, MM_MODEM_BAND_EUTRAN_XXIV    },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_25, MM_MODEM_BAND_EUTRAN_XXV     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_33, MM_MODEM_BAND_EUTRAN_XXXIII  },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_34, MM_MODEM_BAND_EUTRAN_XXXIV   },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_35, MM_MODEM_BAND_EUTRAN_XXXV    },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_36, MM_MODEM_BAND_EUTRAN_XXXVI   },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_37, MM_MODEM_BAND_EUTRAN_XXXVII  },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_38, MM_MODEM_BAND_EUTRAN_XXXVIII },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_39, MM_MODEM_BAND_EUTRAN_XXXIX   },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_40, MM_MODEM_BAND_EUTRAN_XL      },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_41, MM_MODEM_BAND_EUTRAN_XLI     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_42, MM_MODEM_BAND_EUTRAN_XLI     },
    { QMI_NAS_ACTIVE_BAND_EUTRAN_43, MM_MODEM_BAND_EUTRAN_XLIII   }

    /* NOTE. The following bands were unmatched:
     *
     * - QMI_NAS_ACTIVE_BAND_GSM_450
     * - QMI_NAS_ACTIVE_BAND_GSM_480
     * - QMI_NAS_ACTIVE_BAND_GSM_750
     * - QMI_NAS_ACTIVE_BAND_GSM_900_PRIMARY
     * - QMI_NAS_ACTIVE_BAND_GSM_900_RAILWAYS
     * - QMI_NAS_ACTIVE_BAND_WCDMA_1500_JAPAN
     * - QMI_NAS_ACTIVE_BAND_TDSCDMA_A
     * - QMI_NAS_ACTIVE_BAND_TDSCDMA_B
     * - QMI_NAS_ACTIVE_BAND_TDSCDMA_C
     * - QMI_NAS_ACTIVE_BAND_TDSCDMA_D
     * - QMI_NAS_ACTIVE_BAND_TDSCDMA_E
     * - QMI_NAS_ACTIVE_BAND_TDSCDMA_F
     */
};

static void
add_active_bands (GArray *mm_bands,
                  QmiNasActiveBand qmi_bands)
{
    guint i;

    g_assert (mm_bands != NULL);

    for (i = 0; i < G_N_ELEMENTS (active_bands_map); i++) {
        if (qmi_bands == active_bands_map[i].qmi_band) {
            guint j;

            /* Avoid adding duplicate band entries */
            for (j = 0; j < mm_bands->len; j++) {
                if (g_array_index (mm_bands, MMModemBand, j) == active_bands_map[i].mm_band)
                    return;
            }

            g_array_append_val (mm_bands, active_bands_map[i].mm_band);
            return;
        }
    }
}

GArray *
mm_modem_bands_from_qmi_rf_band_information_array (GArray *info_array)
{
    GArray *mm_bands;

    mm_bands = g_array_new (FALSE, FALSE, sizeof (MMModemBand));

    if (info_array) {
        guint i;

        for (i = 0; i < info_array->len; i++) {
            QmiMessageNasGetRfBandInformationOutputListElement *item;

            item = &g_array_index (info_array, QmiMessageNasGetRfBandInformationOutputListElement, i);
            add_active_bands (mm_bands, item->active_band_class);
        }
    }

    return mm_bands;
}

/*****************************************************************************/

MMModemAccessTechnology
mm_modem_access_technology_from_qmi_radio_interface (QmiNasRadioInterface interface)
{
    switch (interface) {
    case QMI_NAS_RADIO_INTERFACE_CDMA_1X:
        return MM_MODEM_ACCESS_TECHNOLOGY_1XRTT;
    case QMI_NAS_RADIO_INTERFACE_CDMA_1XEVDO:
        return MM_MODEM_ACCESS_TECHNOLOGY_EVDO0;
    case QMI_NAS_RADIO_INTERFACE_GSM:
        return MM_MODEM_ACCESS_TECHNOLOGY_GSM;
    case QMI_NAS_RADIO_INTERFACE_UMTS:
        return MM_MODEM_ACCESS_TECHNOLOGY_UMTS;
    case QMI_NAS_RADIO_INTERFACE_LTE:
        return MM_MODEM_ACCESS_TECHNOLOGY_LTE;
    case QMI_NAS_RADIO_INTERFACE_TD_SCDMA:
    case QMI_NAS_RADIO_INTERFACE_AMPS:
    case QMI_NAS_RADIO_INTERFACE_NONE:
    default:
        return MM_MODEM_ACCESS_TECHNOLOGY_UNKNOWN;
    }
}

/*****************************************************************************/

MMModemAccessTechnology
mm_modem_access_technologies_from_qmi_radio_interface_array (GArray *radio_interfaces)
{
    MMModemAccessTechnology access_technology = 0;
    guint i;

    for (i = 0; i < radio_interfaces->len; i++) {
        QmiNasRadioInterface iface;

        iface = g_array_index (radio_interfaces, QmiNasRadioInterface, i);
        access_technology |= mm_modem_access_technology_from_qmi_radio_interface (iface);
    }

    return access_technology;
}

/*****************************************************************************/

MMModemAccessTechnology
mm_modem_access_technology_from_qmi_data_capability (QmiNasDataCapability cap)
{
    switch (cap) {
    case QMI_NAS_DATA_CAPABILITY_GPRS:
        return MM_MODEM_ACCESS_TECHNOLOGY_GPRS;
    case QMI_NAS_DATA_CAPABILITY_EDGE:
        return MM_MODEM_ACCESS_TECHNOLOGY_EDGE;
    case QMI_NAS_DATA_CAPABILITY_HSDPA:
        return MM_MODEM_ACCESS_TECHNOLOGY_HSDPA;
    case QMI_NAS_DATA_CAPABILITY_HSUPA:
        return MM_MODEM_ACCESS_TECHNOLOGY_HSUPA;
    case QMI_NAS_DATA_CAPABILITY_WCDMA:
        return MM_MODEM_ACCESS_TECHNOLOGY_UMTS;
    case QMI_NAS_DATA_CAPABILITY_CDMA:
        return MM_MODEM_ACCESS_TECHNOLOGY_1XRTT;
    case QMI_NAS_DATA_CAPABILITY_EVDO_REV_0:
        return MM_MODEM_ACCESS_TECHNOLOGY_EVDO0;
    case QMI_NAS_DATA_CAPABILITY_EVDO_REV_A:
        return MM_MODEM_ACCESS_TECHNOLOGY_EVDOA;
    case QMI_NAS_DATA_CAPABILITY_GSM:
        return MM_MODEM_ACCESS_TECHNOLOGY_GSM;
    case QMI_NAS_DATA_CAPABILITY_EVDO_REV_B:
        return MM_MODEM_ACCESS_TECHNOLOGY_EVDOB;
    case QMI_NAS_DATA_CAPABILITY_LTE:
        return MM_MODEM_ACCESS_TECHNOLOGY_LTE;
    case QMI_NAS_DATA_CAPABILITY_HSDPA_PLUS:
    case QMI_NAS_DATA_CAPABILITY_DC_HSDPA_PLUS:
        return MM_MODEM_ACCESS_TECHNOLOGY_HSPA_PLUS;
    default:
        return MM_MODEM_ACCESS_TECHNOLOGY_UNKNOWN;
    }
}

/*****************************************************************************/

MMModemAccessTechnology
mm_modem_access_technologies_from_qmi_data_capability_array (GArray *data_capabilities)
{
    MMModemAccessTechnology access_technology = 0;
    guint i;

    for (i = 0; i < data_capabilities->len; i++) {
        QmiNasDataCapability cap;

        cap = g_array_index (data_capabilities, QmiNasDataCapability, i);
        access_technology |= mm_modem_access_technology_from_qmi_data_capability (cap);
    }

    return access_technology;
}

/*****************************************************************************/

MMModemMode
mm_modem_mode_from_qmi_radio_technology_preference (QmiNasRadioTechnologyPreference qmi)
{
    MMModemMode mode = MM_MODEM_MODE_NONE;

    if (qmi & QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_3GPP2) {
        if (qmi & QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_AMPS_OR_GSM)
            mode |= MM_MODEM_MODE_CS; /* AMPS */
        if (qmi & QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_CDMA_OR_WCDMA)
            mode |= MM_MODEM_MODE_2G; /* CDMA */
        if (qmi & QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_HDR)
            mode |= MM_MODEM_MODE_3G; /* EV-DO */
    }

    if (qmi & QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_3GPP) {
        if (qmi & QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_AMPS_OR_GSM)
            mode |= (MM_MODEM_MODE_CS | MM_MODEM_MODE_2G); /* GSM */
        if (qmi & QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_CDMA_OR_WCDMA)
            mode |= MM_MODEM_MODE_3G; /* WCDMA */
    }

    if (qmi & QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_LTE)
        mode |= MM_MODEM_MODE_4G;

    return mode;
}

QmiNasRadioTechnologyPreference
mm_modem_mode_to_qmi_radio_technology_preference (MMModemMode mode,
                                                  gboolean is_cdma)
{
    QmiNasRadioTechnologyPreference pref = 0;

    if (is_cdma) {
        pref |= QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_3GPP2;
        if (mode & MM_MODEM_MODE_2G)
            pref |= QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_CDMA_OR_WCDMA; /* CDMA */
        if (mode & MM_MODEM_MODE_3G)
            pref |= QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_HDR; /* EV-DO */
    } else {
        pref |= QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_3GPP;
        if (mode & MM_MODEM_MODE_2G)
            pref |= QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_AMPS_OR_GSM; /* GSM */
        if (mode & MM_MODEM_MODE_3G)
            pref |= QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_CDMA_OR_WCDMA; /* WCDMA */
    }

    if (mode & MM_MODEM_MODE_4G)
        pref |= QMI_NAS_RADIO_TECHNOLOGY_PREFERENCE_LTE;

    return pref;
}

/*****************************************************************************/

#if defined WITH_NEWEST_QMI_COMMANDS

MMModemMode
mm_modem_mode_from_qmi_rat_mode_preference (QmiNasRatModePreference qmi)
{
    MMModemMode mode = MM_MODEM_MODE_NONE;

    if (qmi & QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1X)
        mode |= MM_MODEM_MODE_2G;

    if (qmi & QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1XEVDO)
        mode |= MM_MODEM_MODE_3G;

    if (qmi & QMI_NAS_RAT_MODE_PREFERENCE_GSM)
        mode |= MM_MODEM_MODE_2G;

    if (qmi & QMI_NAS_RAT_MODE_PREFERENCE_UMTS)
        mode |= MM_MODEM_MODE_3G;

    if (qmi & QMI_NAS_RAT_MODE_PREFERENCE_LTE)
        mode |= MM_MODEM_MODE_4G;

    /* Assume CS if 2G supported */
    if (mode & MM_MODEM_MODE_2G)
        mode |= MM_MODEM_MODE_CS;

    return mode;
}

QmiNasRatModePreference
mm_modem_mode_to_qmi_rat_mode_preference (MMModemMode mode,
                                          gboolean is_cdma,
                                          gboolean is_3gpp)
{
    QmiNasRatModePreference pref = 0;

    if (is_cdma) {
        if (mode & MM_MODEM_MODE_2G)
            pref |= QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1X;

        if (mode & MM_MODEM_MODE_3G)
            pref |= QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1XEVDO;
    }

    if (is_3gpp) {
        if (mode & MM_MODEM_MODE_2G)
            pref |= QMI_NAS_RAT_MODE_PREFERENCE_GSM;

        if (mode & MM_MODEM_MODE_3G)
            pref |= QMI_NAS_RAT_MODE_PREFERENCE_UMTS;

        if (mode & MM_MODEM_MODE_4G)
            pref |= QMI_NAS_RAT_MODE_PREFERENCE_LTE;
    }

    return mode;
}

/*****************************************************************************/

MMModemMode
mm_modem_mode_from_qmi_gsm_wcdma_acquisition_order_preference (QmiNasGsmWcdmaAcquisitionOrderPreference qmi)
{
    switch (qmi) {
    case QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_AUTOMATIC:
        return MM_MODEM_MODE_NONE;
    case QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_GSM:
        return MM_MODEM_MODE_2G;
    case QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_WCDMA:
        return MM_MODEM_MODE_3G;
    default:
        mm_dbg ("Unknown acquisition order preference: '%s'",
                qmi_nas_gsm_wcdma_acquisition_order_preference_get_string (qmi));
        return MM_MODEM_MODE_NONE;
    }
}

QmiNasGsmWcdmaAcquisitionOrderPreference
mm_modem_mode_to_qmi_gsm_wcdma_acquisition_order_preference (MMModemMode mode)
{
    gchar *str;

    /* mode is not a mask in this case, only a value */

    switch (mode) {
    case MM_MODEM_MODE_3G:
        return QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_WCDMA;
    case MM_MODEM_MODE_2G:
        return QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_GSM;
    case MM_MODEM_MODE_NONE:
        return QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_AUTOMATIC;
    default:
        break;
    }

    str = mm_modem_mode_build_string_from_mask (mode);
    mm_dbg ("Unhandled modem mode: '%s'", str);
    g_free (str);

    return MM_MODEM_MODE_NONE;
}

#endif /* WITH_NEWEST_QMI_COMMANDS */

/*****************************************************************************/

MMModem3gppRegistrationState
mm_modem_3gpp_registration_state_from_qmi_registration_state (QmiNasAttachState attach_state,
                                                              QmiNasRegistrationState registration_state,
                                                              gboolean roaming)
{
    if (attach_state == QMI_NAS_ATTACH_STATE_UNKNOWN)
        return MM_MODEM_3GPP_REGISTRATION_STATE_UNKNOWN;

    if (attach_state == QMI_NAS_ATTACH_STATE_DETACHED)
        return QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED;

    /* attached */

    switch (registration_state) {
    case QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED:
        return MM_MODEM_3GPP_REGISTRATION_STATE_IDLE;
    case QMI_NAS_REGISTRATION_STATE_REGISTERED:
        return (roaming ? MM_MODEM_3GPP_REGISTRATION_STATE_ROAMING : MM_MODEM_3GPP_REGISTRATION_STATE_HOME);
    case QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED_SEARCHING:
        return MM_MODEM_3GPP_REGISTRATION_STATE_SEARCHING;
    case QMI_NAS_REGISTRATION_STATE_REGISTRATION_DENIED:
        return MM_MODEM_3GPP_REGISTRATION_STATE_DENIED;
    case QMI_NAS_REGISTRATION_STATE_UNKNOWN:
    default:
        return MM_MODEM_3GPP_REGISTRATION_STATE_UNKNOWN;
    }
}

/*****************************************************************************/

MMModemCdmaRegistrationState
mm_modem_cdma_registration_state_from_qmi_registration_state (QmiNasRegistrationState registration_state)
{
    switch (registration_state) {
    case QMI_NAS_REGISTRATION_STATE_REGISTERED:
        return MM_MODEM_CDMA_REGISTRATION_STATE_REGISTERED;
    case QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED:
    case QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED_SEARCHING:
    case QMI_NAS_REGISTRATION_STATE_REGISTRATION_DENIED:
    case QMI_NAS_REGISTRATION_STATE_UNKNOWN:
    default:
        return MM_MODEM_CDMA_REGISTRATION_STATE_UNKNOWN;
    }
}