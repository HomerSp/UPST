#ifndef QCDM_H
#define QCDM_H

namespace Serial {
    namespace QCDM {
        enum DiagCommands : unsigned char
        {
            DIAG_VERNO_F                        = 0x00, // [0] Version Number Request/Response
            DIAG_ESN_F                          = 0x01, // [1] Mobile Station ESN Request/Response
            DIAG_PEEKB_F                        = 0x02, // [2] Peek byte Request/Response
            DIAG_PEEKW_F                        = 0x03, // [3] Peek word Request/Response
            DIAG_PEEKD_F                        = 0x04, // [4] Peek dword Request/Response
            DIAG_POKEB_F                        = 0x05, // [5] Poke byte Request/Response
            DIAG_POKEW_F                        = 0x06, // [6] Poke word Request/Response
            DIAG_POKED_F                        = 0x07, // [7] Poke dword Request/Response
            DIAG_OUTP_F                         = 0x08, // [8] Byte output Request/Response
            DIAG_OUTPW_F                        = 0x09, // [9] Word output Request/Response
            DIAG_INP_F                          = 0x0A, // [10] Byte input Request/Response
            DIAG_INPW_F                         = 0x0B, // [11] Word input Request/Response
            DIAG_STATUS_F                       = 0x0C, // [12] DMSS status Request/Response

            /* 13-14 Reserved */

            DIAG_LOGMASK_F                      = 0x0F, // [15] Set logging mask Request/Response
            DIAG_LOG_F                          = 0x10, // [16] Log packet Request/Response
            DIAG_NV_PEEK_F                      = 0x11, // [17] Peek at NV memory Request/Response
            DIAG_NV_POKE_F                      = 0x12, // [18] Poke at NV memory Request/Response
            DIAG_BAD_CMD_F                      = 0x13, // [19] Invalid Command Response
            DIAG_BAD_PARM_F                     = 0x14, // [20] Invalid parmaeter Response
            DIAG_BAD_LEN_F                      = 0x15, // [21] Invalid packet length Response

            /* 22-23 Reserved */
            DIAG_BAD_MODE_F                     = 0x18, // [24] Packet not allowed in this mode  ( online vs offline )

            DIAG_TAGRAPH_F                      = 0x19, // [25] info for TA power and voice graphs
            DIAG_MARKOV_F                       = 0x1A, // [26] Markov statistics
            DIAG_MARKOV_RESET_F                 = 0x1B, // [27] Reset of Markov statistics
            DIAG_DIAG_VER_F                     = 0x1C, // [28] Return diag version for comparison to detect incompatabilities
            DIAG_TS_F                           = 0x1D, // [29] Return a timestamp
            DIAG_TA_PARM_F                      = 0x1E, // [30] Set TA parameters
            DIAG_MSG_F                          = 0x1F, // [31] Request for msg report
            DIAG_HS_KEY_F                       = 0x20, // [32] Handset Emulation -- keypress
            DIAG_HS_LOCK_F                      = 0x21, // [33] Handset Emulation -- lock or unlock
            DIAG_HS_SCREEN_F                    = 0x22, // [34] Handset Emulation -- display request

            /* 35 Reserved */

            DIAG_PARM_SET_F                     = 0x24, // [36] Parameter Download

            /* 37 Reserved */

            DIAG_NV_READ_F                      = 0x26, // [38] Read NV item
            DIAG_NV_WRITE_F                     = 0x27, // [39] Write NV item

            /* 40 Reserved */

            DIAG_CONTROL_F                      = 0x29, // [41] Mode change request
            DIAG_ERR_READ_F                     = 0x2A, // [42] Error record retreival
            DIAG_ERR_CLEAR_F                    = 0x2B, // [43] Error record clear
            DIAG_SER_RESET_F                    = 0x2C, // [44] Symbol error rate counter reset
            DIAG_SER_REPORT_F                   = 0x2D, // [45] Symbol error rate counter report
            DIAG_TEST_F                         = 0x2E, // [46] Run a specified test
            DIAG_GET_DIPSW_F                    = 0x2F, // [47] Retreive the current dip switch setting
            DIAG_SET_DIPSW_F                    = 0x30, // [48] Write new dip switch setting
            DIAG_VOC_PCM_LB_F                   = 0x31, // [49] Start/Stop Vocoder PCM loopback
            DIAG_VOC_PKT_LB_F                   = 0x32, // [50] Start/Stop Vocoder PKT loopback

            /* 51-52 Reserved */

            DIAG_ORIG_F                         = 0x35, // [53] Originate a call
            DIAG_END_F                          = 0x36, // [54] End a call

            /* 55-57 Reserved */

            DIAG_DLOAD_F                        = 0x3A, // [58] Switch to downloader
            DIAG_TMOB_F                         = 0x3B, // [59] Test Mode Commands and FTM commands
            DIAG_FTM_CMD_F                      = 0x3B, // [59] Test Mode Commands and FTM commands

            /* 60-62 Reserved */

            DIAG_TEST_STATE_F                   = 0x3D, // [61]
            DIAG_STATE_F                        = 0x3F, // [63] Return the current state of the phone
            DIAG_PILOT_SETS_F                   = 0x40, // [64] Return all current sets of pilots
            DIAG_SPC_F                          = 0x41, // [65] Send the Service Prog. Code to allow SP
            DIAG_BAD_SPC_MODE_F                 = 0x42, // [66] Invalid nv_read/write because SP is locked
            DIAG_PARM_GET2_F                    = 0x43, // [67] get parms obsoletes PARM_GET
            DIAG_SERIAL_CHG_F                   = 0x44, // [68] Serial mode change Request/Response

            /* 69 Reserved */

            DIAG_PASSWORD_F                     = 0x46, // [70] Send password to unlock secure operations the phone to be in a security state that it wasn't - like unlocked.
            DIAG_BAD_SEC_MODE_F                 = 0x47, // [71] An operation was attempted which required
            DIAG_PR_LIST_WR_F                   = 0x48, // [72] Write Preferred Roaming list to the phone.
            DIAG_PR_LIST_RD_F                   = 0x49, // [73]

            /* 74 Reserved */

            DIAG_SUBSYS_CMD_F                   = 0x4B, // [75] Subssytem dispatcher (extended diag cmd)

            /* 76-80 Reserved */

            DIAG_FEATURE_QUERY_F                = 0x51, // [81] Asks the phone what it supports

            /* 82 Reserved */

            DIAG_SMS_READ_F                     = 0x53, // [83] Read SMS message out of NV
            DIAG_SMS_WRITE_F                    = 0x54, // [84] Write SMS message into NV
            DIAG_SUP_FER_F                      = 0x55, // [85] info for Frame Error Rate           on multiple channels
            DIAG_SUP_WALSH_CODES_F              = 0x56, // [86] Supplemental channel walsh codes
            DIAG_SET_MAX_SUP_CH_F               = 0x57, // [87] Sets the maximum # supplemental  channels
            DIAG_PARM_GET_IS95B_F               = 0x58, // [88] get parms including SUPP and MUX2:  obsoletes PARM_GET and PARM_GET_2
            DIAG_FS_OP_F                        = 0x59, // [89] Performs an Embedded File System (EFS) operation.
            DIAG_AKEY_VERIFY_F                  = 0x5A, // [90] AKEY Verification.
            DIAG_BMP_HS_SCREEN_F                = 0x5B, // [91] Handset emulation - Bitmap screen
            DIAG_CONFIG_COMM_F                  = 0x5C, // [92] Configure communications
            DIAG_EXT_LOGMASK_F                  = 0x5D, // [93] Extended logmask for > 32 bits.

            /* 94-95 reserved */

            DIAG_EVENT_REPORT_F                 = 0x60, // [96] Static Event reporting.
            DIAG_STREAMING_CONFIG_F             = 0x61, // [97] Load balancing and more!
            DIAG_PARM_RETRIEVE_F                = 0x62, // [98] Parameter retrieval
            DIAG_STATUS_SNAPSHOT_F              = 0x63, // [99] A state/status snapshot of the DMSS.
            DIAG_RPC_F                          = 0x64, // [100] Used for RPC
            DIAG_GET_PROPERTY_F                 = 0x65, // [101] Get_property requests
            DIAG_PUT_PROPERTY_F                 = 0x66, // [102] Put_property requests
            DIAG_GET_GUID_F                     = 0x67, // [103] Get_guid requests
            DIAG_USER_CMD_F                     = 0x68, // [104] Invocation of user callbacks
            DIAG_GET_PERM_PROPERTY_F            = 0x69, // [105] Get permanent properties
            DIAG_PUT_PERM_PROPERTY_F            = 0x6A, // [106] Put permanent properties
            DIAG_PERM_USER_CMD_F                = 0x6B, // [107] Permanent user callbacks
            DIAG_GPS_SESS_CTRL_F                = 0x6C, // [108] GPS Session Control
            DIAG_GPS_GRID_F                     = 0x6D, // [109] GPS search grid
            DIAG_GPS_STATISTICS_F               = 0x6E, // [110] GPS Statistics
            DIAG_ROUTE_F                        = 0x6F, // [111] Packet routing for multiple instances of diag
            DIAG_IS2000_STATUS_F                = 0x70, // [112] IS2000 status
            DIAG_RLP_STAT_RESET_F               = 0x71, // [113] RLP statistics reset
            DIAG_TDSO_STAT_RESET_F              = 0x72, // [114] (S)TDSO statistics reset
            DIAG_LOG_CONFIG_F                   = 0x73, // [115] Logging configuration packet
            DIAG_TRACE_EVENT_REPORT_F           = 0x74, // [116] Static Trace Event reporting
            DIAG_SBI_READ_F                     = 0x75, // [117] SBI Read
            DIAG_SBI_WRITE_F                    = 0x76, // [118] SBI Write
            DIAG_SSD_VERIFY_F                   = 0x77, // [119] SSD Verify
            DIAG_LOG_ON_DEMAND_F                = 0x78, // [120] Log on Request
            DIAG_EXT_MSG_F                      = 0x79, // [121] Request for extended msg report
            DIAG_ONCRPC_F                       = 0x7A, // [122] ONCRPC diag packet
            DIAG_PROTOCOL_LOOPBACK_F            = 0x7B, // [123] Diagnostics protocol loopback.
            DIAG_EXT_BUILD_ID_F                 = 0x7C, // [124] Extended build ID text
            DIAG_EXT_MSG_CONFIG_F               = 0x7D, // [125] Request for extended msg report
            DIAG_EXT_MSG_TERSE_F                = 0x7E, // [126] Extended messages in terse format
            DIAG_EXT_MSG_TERSE_XLATE_F          = 0x7F, // [127] Translate terse format message identifier
            DIAG_SUBSYS_CMD_VER_2_F             = 0x80, // [128] Subssytem dispatcher Version 2 (delayed response capable)
            DIAG_EVENT_MASK_GET_F               = 0x81, // [129] Get the event mask
            DIAG_EVENT_MASK_SET_F               = 0x82, // [130] Set the event mask

            /* 131-139 reserved */

            DIAG_CHANGE_PORT_SETTINGS           = 0x8C, // [140] Command Code for Changing Port Settings
            DIAG_CNTRY_INFO_F                   = 0x8D, // [141] Country network information for assisted dialing
            DIAG_SUPS_REQ_F                     = 0x8E, // [142] Send a Supplementary Service Request
            DIAG_MMS_ORIG_SMS_REQUEST_F         = 0x8F, // [143] Originate SMS request for MMS
            DIAG_MEAS_MODE_F                    = 0x90, // [144]
            DIAG_MEAS_REQ_F                     = 0x91, // [145] Request measurements for HDR channels
            DIAG_QSR_EXT_MSG_TERSE_F            = 0x92, // [146] Send Optimized F3 messages
            DIAG_LGF_SCREEN_SHOT_F              = 0x96, // [150] LGE_CHANGES_S [minjong.gong@lge.com] 2010-06-11, LG_FW_DIAG_SCREEN_CAPTURE
            DIAG_MTC_F                          = 0xF0, // [240]
            DIAG_WMCSYNC_MAPPING_F              = 0xF1, // [241]
            DIAG_WEBDLOAD_COMMON_F              = 0xEF, // [239]
            DIAG_WIFI_MAC_ADDR                  = 0xD6, // [214]
            DIAG_USET_DATA_BACKUP               = 0xF9, // [249]
            DIAG_TEST_MODE_F                    = 0xFA, // [250] Number of packets defined.
            DIAG_SMS_TEST_F                     = 0xDC, // [220]
            DIAG_UDM_SMS_MODE                   = 0xFC, // [252]
            DIAG_LCD_Q_TEST_F                   = 0xFD, // [253]
            DIAG_ERI_CMD_F                      = 0xFE, // [254]
            DIAG_MAX_F                          = 0xFF, // [255]
        };

        enum Response
        {
           DIAG_ERR_SUCCESS             = 0x0,
           DIAG_ERR_FAIL                = 0x1,
           DIAG_ERR_BAD_CMD             = 0x2,
           DIAG_ERR_BAD_PARAM           = 0x3,
           DIAG_ERR_BAD_LENGTH          = 0x4,
           DIAG_ERR_BAD_SECURITY_MODE   = 0x5,
           DIAG_ERR_BAD_REPLY_CMD       = 0xB,
           DIAG_ERR_BAD_SPC_MODE        = 0x15,
           DIAG_ERR_BAD_MODE            = 0x20,
        };

        enum Mode
        {
           MODE_RADIO_OFFLINE   = 0x1,
           MODE_RADIO_RESET     = 0x2,
           MODE_RADIO_ONLINE    = 0x4,
           MODE_RADIO_LOWPOWER  = 0x5,
        };

        struct ExtendedBuildID {
            uint16_t unk1;
            uint32_t msmv2;
            uint32_t mobModel;
            char mobModelSw[256];
        };
    }
}

#endif // QCDM_H
