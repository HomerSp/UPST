#ifndef QCDM_H
#define QCDM_H

namespace Serial {
    namespace QCDM {
        enum DiagCommands : unsigned char
        {
           NOT_A_COMMAND = 0xFF,
           DIAG_VERNO_F = 0x0,             // Version Number Request/Response
           DIAG_ESN_F = 0x1,               // Mobile Station ESN Request/Response
           DIAG_PEEKB_F = 0x2,             // Peek byte Request/Response
           DIAG_PEEKW_F = 0x3,             // Peek word Request/Response
           DIAG_PEEKD_F = 0x4,             // Peek dword Request/Response
           DIAG_POKEB_F = 0x5,             // Poke byte Request/Response
           DIAG_POKEW_F = 0x6,             // Poke word Request/Response
           DIAG_POKED_F = 0x7,             // Poke dword Request/Response
           DIAG_OUTP_F = 0x8,              // Byte output Request/Response
           DIAG_OUTPW_F = 0x9,             // Word output Request/Response
           DIAG_INP_F = 0xA,               // Byte input Request/Response
           DIAG_INPW_F = 0xB,              // Word input Request/Response
           DIAG_STATUS_F = 0xC,            // DMSS status Request/Response
           DIAG_LOGMASK_F = 0xF,           // Set logging mask Request/Response
           DIAG_LOG_F = 0x10,              // Log packet Request/Response
           DIAG_NV_PEEK_F = 0x11,          // Peek at NV memory Request/Response
           DIAG_NV_POKE_F = 0x12,          // Poke at NV memory Request/Response
           DIAG_BAD_CMD_F = 0x13,          // Invalid Command Response
           DIAG_BAD_PARM_F = 0x14,         // Invalid parmaeter Response
           DIAG_BAD_LEN_F = 0x15,          // Invalid packet length Response
           DIAG_BAD_MODE_F = 0x18,         // Packet not allowed in this mode
           DIAG_TAGRAPH_F = 0x19,          // info for TA power and voice graphs
           DIAG_MARKOV_F = 0x1A,           // Markov statistics
           DIAG_MARKOV_RESET_F = 0x1B,     // Reset of Markov statistics
           DIAG_DIAG_VER_F = 0x1C,         // Return diag version for comparison to
           DIAG_TS_F = 0x1D,               // Return a timestamp
           DIAG_TA_PARM_F = 0x1E,          // Set TA parameters
           DIAG_MSG_F = 0x1F,              // Request for msg report
           DIAG_HS_KEY_F = 0x20,           // Handset Emulation -- keypress
           DIAG_HS_LOCK_F = 0x21,          // Handset Emulation -- lock or unlock
           DIAG_HS_SCREEN_F = 0x22,        // Handset Emulation -- display request
           DIAG_PARM_SET_F = 0x24,         // Parameter Download
           DIAG_NV_READ_F = 0x26,          // Read NV item
           DIAG_NV_WRITE_F = 0x27,         // Write NV item
           DIAG_CONTROL_F = 0x29,          // Mode change request
           DIAG_ERR_READ_F = 0x2A,         // Error record retreival
           DIAG_ERR_CLEAR_F = 0x2B,        // Error record clear
           DIAG_SER_RESET_F = 0x2C,        // Symbol error rate counter reset
           DIAG_SER_REPORT_F = 0x2D,       // Symbol error rate counter report
           DIAG_TEST_F = 0x2E,             // Run a specified test
           DIAG_GET_DIPSW_F = 0x2F,        // Retreive the current dip switch setting
           DIAG_SET_DIPSW_F = 0x30,        // Write new dip switch setting
           DIAG_VOC_PCM_LB_F = 0x31,       // Start/Stop Vocoder PCM loopback
           DIAG_VOC_PKT_LB_F = 0x32,       // Start/Stop Vocoder PKT loopback
           DIAG_ORIG_F = 0x35,             // Originate a call
           DIAG_END_F = 0x36,              // End a call
           DIAG_DLOAD_F = 0x3A,            // Switch to downloader
           DIAG_TMOB_F = 0x3B,             // Test Mode Commands and FTM commands
           DIAG_STATE_F = 0x3F,            // Return the current state of the phone
           DIAG_PILOT_SETS_F = 0x40,       // Return all current sets of pilots
           DIAG_SPC_F = 0x41,              // Send the Service Prog. Code to allow SP
           DIAG_BAD_SPC_MODE_F = 0x42,     // Invalid nv_read/write because SP is locked
           DIAG_PARM_GET2_F = 0x43,        // get parms obsoletes PARM_GET
           DIAG_SERIAL_CHG_F = 0x44,       // Serial mode change Request/Response
           DIAG_PASSWORD_F = 0x46,         // Send password to unlock secure operations
           DIAG_BAD_SEC_MODE_F = 0x47,     // An operation was attempted which required
           DIAG_PR_LIST_WR_F = 0x48,       // Write Preferred Roaming list to the phone.
           DIAG_PR_LIST_RD_F = 0x49,       // Read Preferred Roaming list from the phone.
           DIAG_SUBSYS_CMD_F = 0x4B,       // Dispatches a command to a subsystem
           DIAG_FEATURE_QUERY_F = 0x51,    // Asks the phone what it supports
           DIAG_SMS_READ_F = 0x53,         // Read SMS message out of NV
           DIAG_SMS_WRITE_F = 0x54,        // Write SMS message into NV
           DIAG_SUP_FER_F = 0x55,          // info for Frame Error Rate
           DIAG_SUP_WALSH_CODES_F = 0x56,  // Supplemental channel walsh codes
           DIAG_SET_MAX_SUP_CH_F = 0x57,   // Sets the maximum # supplemental channels
           DIAG_PARM_GET_IS95B_F = 0x58,   // get parms including SUPP and MUX2:
           DIAG_RAM_RW_F = 0x59,           // calibration RAM control using DM
           DIAG_CPU_RW_F = 0x5A,           // calibration CPU control using DM
           DIAG_FS_OP_F = 0x59,            // Performs an Embedded File
           DIAG_AKEY_VERIFY_F = 0x5A,      // AKEY Verification.
           DIAG_BMP_HS_SCREEN_F = 0x5B,    // Handset emulation - Bitmap screen
           DIAG_CONFIG_COMM_F = 0x5C,      // Configure communications
           DIAG_EXT_LOGMASK_F = 0x5D,      // Extended logmask for > 32 bits.
           DIAG_EVENT_REPORT_F = 0x60,     // Static Event reporting.
           DIAG_STREAMING_CONFIG_F = 0x61, // Load balancing and more!
           DIAG_PARM_RETRIEVE_F = 0x62,    // Parameter retrieval
           DIAG_STATUS_SNAPSHOT_F = 0x63,  // A state/status snapshot of the DMSS
           DIAG_RPC_F = 0x64,              // Used for RPC
           DIAG_GET_PROPERTY_F = 0x65,     // Get_property requests
           DIAG_PUT_PROPERTY_F = 0x66,     // Put_property requests
           DIAG_GET_GUID_F = 0x67,         // Get_guid requests
           DIAG_USER_CMD_F = 0x68,         // Invocation of user callbacks
           DIAG_GET_PERM_PROPERTY_F = 0x69,// Get permanent properties
           DIAG_PUT_PERM_PROPERTY_F = 0x6A,// Put permanent properties
           DIAG_PERM_USER_CMD_F = 0x6B,    // Permanent user callbacks
           DIAG_GPS_SESS_CTRL_F = 0x6C,    // GPS Session Control
           DIAG_GPS_GRID_F = 0x6D,         // GPS search grid
           DIAG_GPS_STATISTICS_F = 0x6E,   // GPS Statistics
           DIAG_TUNNEL_F = 0x6F,           // DIAG Tunneling command code
           DIAG_MAX_F,                     // Number of packets defined.

           DIAG_SW_VERSION_F = 0x38,
           DIAG_SET_FTM_TEST_MODE = 0x72,
        };

        enum Response
        {
           DIAG_ERR_SUCCESS = 0x0,
           DIAG_ERR_FAIL = 0x1,
           DIAG_ERR_BAD_CMD = 0x2,
           DIAG_ERR_BAD_PARAM = 0x3,
           DIAG_ERR_BAD_LENGTH = 0x4,
           DIAG_ERR_BAD_SECURITY_MODE = 0x5,
           DIAG_ERR_BAD_REPLY_CMD = 0xB,
           DIAG_ERR_BAD_SPC_MODE = 0x15,
           DIAG_ERR_BAD_MODE = 0x20
        };

        enum Mode
        {
           MODE_RADIO_OFFLINE = 0x1,
           MODE_RADIO_RESET = 0x2,
           MODE_RADIO_ONLINE = 0x4,
           MODE_RADIO_LOWPOWER = 0x5
        };
    }
}

#endif // QCDM_H
