// -*- Mode:C++ -*-

#pragma once

#define COMMAND(ogf,ocf,name) OPCODE_##name = OPCODE(ogf,ocf),
#define EVENT(code,name) EVENT_##name = code,
#define LE_EVENT(code,name) LE_EVENT_##name = code,
#define OPCODE(ogf,ocf) (((ogf) << 10) | (ocf))

namespace akt {
  namespace bluetooth {
    namespace HCI {
      enum status {
        SUCCESS                                                          = 0x00,
        UNKNOWN_HCI_COMMAND                                              = 0x01,
        UNKNOWN_CONNECTION_IDENTIFIER                                    = 0x02,
        HARDWARE_FAILURE                                                 = 0x03,
        PAGE_TIMEOUT                                                     = 0x04,
        AUTHENTICATION_FAILURE                                           = 0x05,
        PIN_OR_KEY_MISSING                                               = 0x06,
        MEMORY_CAPACITY_EXCEEDED                                         = 0x07,
        CONNECTION_TIMEOUT                                               = 0x08,
        CONNECTION_LIMIT_EXCEEDED                                        = 0x09,
        SYNCHRONOUS_CONNECTION_LIMIT_TO_A_DEVICE_EXCEEDED                = 0x0a,
        ACL_CONNECTION_ALREADY_EXISTS                                    = 0x0b,
        COMMAND_DISALLOWED                                               = 0x0c,
        CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES                     = 0x0d,
        CONNECTION_REJECTED_DUE_TO_SECURITY_REASONS                      = 0x0e,
        CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR                  = 0x0f,
        CONNECTION_ACCEPT_TIMEOUT_EXCEEDED                               = 0x10,
        UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE                           = 0x11,
        INVALID_HCI_COMMAND_PARAMETERS                                   = 0x12,
        REMOTE_USER_TERMINATED_CONNECTION                                = 0x13,
        REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES         = 0x14,
        REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF             = 0x15,
        CONNECTION_TERMINATED_BY_LOCAL_HOST                              = 0x16,
        REPEATED_ATTEMPTS                                                = 0x17,
        PAIRING_NOT_ALLOWED                                              = 0x18,
        UNKNOWN_LMP_PDU                                                  = 0x19,
        UNSUPPORTED_REMOTE_LMP_FEATURE                                   = 0x1a,
        SCO_OFFSET_REJECTED                                              = 0x1b,
        SCO_INTERVAL_REJECTED                                            = 0x1c,
        SCO_AIR_MODE_REJECTED                                            = 0x1d,
        INVALID_LMP_PARAMETERS                                           = 0x1e,
        UNSPECIFIED_ERROR                                                = 0x1f,
        UNSUPPORTED_LMP_PARAMETER_VALUE                                  = 0x20,
        ROLE_CHANGE_NOT_ALLOWED                                          = 0x21,
        LMP_LL_RESPONSE_TIMEOUT                                          = 0x22,
        LMP_ERROR_TRANSACTION_COLLISION                                  = 0x23,
        LMP_PDU_NOT_ALLOWED                                              = 0x24,
        ENCRYPTION_MODE_NOT_ACCEPTABLE                                   = 0x25,
        LINK_KEY_CANNOT_BE_CHANGED                                       = 0x26,
        REQUESTED_QOS_NOT_SUPPORTED                                      = 0x27,
        INSTANT_PASSED                                                   = 0x28,
        PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED                              = 0x29,
        DIFFERENT_TRANSACTION_COLLISION                                  = 0x2a,
        QOS_UNACCEPTABLE_PARAMETER                                       = 0x2c,
        QOS_REJECTED                                                     = 0x2d,
        CHANNEL_ASSESSMENT_NOT_SUPPORTED                                 = 0x2e,
        INSUFFICIENT_SECURITY                                            = 0x2f,
        PARAMETER_OUT_OF_MANDATORY_RANGE                                 = 0x30,
        ROLE_SWITCH_PENDING                                              = 0x32,
        RESERVED_SLOT_VIOLATION                                          = 0x34,
        ROLE_SWITCH_FAILED                                               = 0x35,
        EXTENDED_INQUIRY_RESPONSE_TOO_LARGE                              = 0x36,
        SIMPLE_PAIRING_NOT_SUPPORTED_BY_HOST                             = 0x37,
        HOST_BUSY_PAIRING                                                = 0x38,
        CONNECTION_REJECTED_DUE_TO_NO_SUITABLE_CHANNEL_FOUND             = 0x39,
        CONTROLLER_BUSY                                                  = 0x3a,
        UNACCEPTABLE_CONNECTION_INTERVAL                                 = 0x3b,
        DIRECTED_ADVERTISING_TIMEOUT                                     = 0x3c,
        CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE                         = 0x3e,
        MAC_CONNECTION_FAILED                                            = 0x3f
      };

      enum opcode_group {
        LC /* Link Control */                                            = 0x01,
        LP /* Link Policy */                                             = 0x02,
        BB /* Controller & Baseband */                                   = 0x03, 
        IF /* Informational */                                           = 0x04,
        ST /* Status */                                                  = 0x05,
        TS /* Testing */                                                 = 0x06,
        LE /* Low Energy */                                              = 0x08,
        VS /* Vendor Specific */                                         = 0x3f
      };

      enum version {
        SPECIFICATION_1_1B,
        SPECIFICATION_1_1,
        SPECIFICATION_1_2,
        SPECIFICATION_2_0,
        SPECIFICATION_2_1_EDR,
        SPECIFICATION_3_0_HS,
        SPECIFICATION_4_0
      };

      enum packet_indicator {
        COMMAND_PACKET                                                   = 0x01,
        ACL_PACKET                                                       = 0x02,
        SYNCHRONOUS_DATA_PACKET                                          = 0x03,
        EVENT_PACKET                                                     = 0x04,
        GO_TO_SLEEP_IND                                                  = 0x30,
        GO_TO_SLEEP_ACK                                                  = 0x31,
        WAKE_UP_IND                                                      = 0x32,
        WAKE_UP_ACK                                                      = 0x33
      };

      enum opcode {
        COMMAND(LC, 0x0001, INQUIRY)
        COMMAND(LC, 0x0002, INQUIRY_CANCEL)
        COMMAND(LC, 0x0003, PERIODIC_INQUIRY_MODE)
        COMMAND(LC, 0x0004, EXIT_PERIODIC_INQUIRY_MODE)
        COMMAND(LC, 0x0005, CREATE_CONNECTION)
        COMMAND(LC, 0x0006, DISCONNECT)
        COMMAND(LC, 0x0008, CREATE_CONNECTION_CANCEL)
        COMMAND(LC, 0x0009, ACCEPT_CONNECTION_REQUEST)
        COMMAND(LC, 0x000a, REJECT_CONNECTION_REQUEST)
        COMMAND(LC, 0x000b, LINK_KEY_REQUEST_REPLY)
        COMMAND(LC, 0x000c, LINK_KEY_REQUEST_NEGATIVE_REPLY)
        COMMAND(LC, 0x000d, PIN_CODE_REQUEST_REPLY)
        COMMAND(LC, 0x000e, PIN_CODE_REQUEST_NEGATIVE_REPLY)
        COMMAND(LC, 0x000f, CHANGE_CONNECTION_PACKET_TYPE)
        COMMAND(LC, 0x0011, AUTHENTICATION_REQUESTED)
        COMMAND(LC, 0x0013, SET_CONNECTION_ENCRYPTION)
        COMMAND(LC, 0x0015, CHANGE_CONNECTION_LINK_KEY)
        COMMAND(LC, 0x0017, MASTER_LINK_KEY)
        COMMAND(LC, 0x0019, REMOTE_NAME_REQUEST)
        COMMAND(LC, 0x001a, REMOTE_NAME_REQUEST_CANCEL)
        COMMAND(LC, 0x001b, READ_REMOTE_SUPPORTED_FEATURES)
        COMMAND(LC, 0x001c, READ_REMOTE_EXTENDED_FEATURES)
        COMMAND(LC, 0x001d, READ_REMOTE_VERSION_INFORMATION)
        COMMAND(LC, 0x001f, READ_CLOCK_OFFSET)
        COMMAND(LC, 0x0020, READ_LMP_HANDLE)
        COMMAND(LC, 0x0028, SETUP_SYNCHRONOUS_CONNECTION)
        COMMAND(LC, 0x0029, ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST)
        COMMAND(LC, 0x002a, REJECT_SYNCHRONOUS_CONNECTION_REQUEST)
        COMMAND(LC, 0x002b, IO_CAPABILITY_REQUEST_REPLY)
        COMMAND(LC, 0x002c, USER_CONFIRMATION_REQUEST_REPLY)
        COMMAND(LC, 0x002d, USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY)
        COMMAND(LC, 0x002e, USER_PASSKEY_REQUEST_REPLY)
        COMMAND(LC, 0x002f, USER_PASSKEY_REQUEST_NEGATIVE_REPLY)
        COMMAND(LC, 0x0030, REMOTE_OOB_DATA_REQUEST_REPLY)
        COMMAND(LC, 0x0033, REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY)
        COMMAND(LC, 0x0034, IO_CAPABILITY_REQUEST_NEGATIVE_REPLY)
        COMMAND(LC, 0x0035, CREATE_PHYSICAL_LINK)
        COMMAND(LC, 0x0036, ACCEPT_PHYSICAL_LINK)
        COMMAND(LC, 0x0037, DISCONNECT_PHYSICAL_LINK)
        COMMAND(LC, 0x0038, CREATE_LOGICAL_LINK)
        COMMAND(LC, 0x0039, ACCEPT_LOGICAL_LINK)
        COMMAND(LC, 0x003a, DISCONNECT_LOGICAL_LINK)
        COMMAND(LC, 0x003b, LOGICAL_LINK_CANCEL)
        COMMAND(LC, 0x003c, FLOW_SPEC_MODIFY)
                       
        COMMAND(LP, 0x0001, HOLD_MODE)
        COMMAND(LP, 0x0003, SNIFF_MODE)
        COMMAND(LP, 0x0004, EXIT_SNIFF_MODE)
        COMMAND(LP, 0x0005, PARK_STATE)
        COMMAND(LP, 0x0006, EXIT_PARK_STATE)
        COMMAND(LP, 0x0007, QOS_SETUP)
        COMMAND(LP, 0x0009, ROLE_DISCOVERY)
        COMMAND(LP, 0x000b, SWITCH_ROLE)
        COMMAND(LP, 0x000c, READ_LINK_POLICY_SETTINGS)
        COMMAND(LP, 0x000d, WRITE_LINK_POLICY)
        COMMAND(LP, 0x000e, READ_DEFAULT_LINK_POLICY_SETTINGS)
        COMMAND(LP, 0x000f, WRITE_DEFAULT_LINK_POLICY_SETTINGS)
        COMMAND(LP, 0x0010, FLOW_SPECIFICATION)
        COMMAND(LP, 0x0011, SNIFF_SUBRATING)
                       
        COMMAND(BB, 0x0001, SET_EVENT_MASK)
        COMMAND(BB, 0x0003, RESET)
        COMMAND(BB, 0x0005, SET_EVENT_FILTER)
        COMMAND(BB, 0x0008, FLUSH)
        COMMAND(BB, 0x0009, READ_PIN_TYPE)
        COMMAND(BB, 0x000a, WRITE_PIN_TYPE)
        COMMAND(BB, 0x000b, CREATE_NEW_UNIT_KEY)
        COMMAND(BB, 0x000d, READ_STORED_LINK_KEY)
        COMMAND(BB, 0x0011, WRITE_STORED_LINK_KEY)
        COMMAND(BB, 0x0012, DELETE_STORED_LINK_KEY)
        COMMAND(BB, 0x0013, WRITE_LOCAL_NAME_COMMAND)
        COMMAND(BB, 0x0014, READ_LOCAL_NAME_COMMAND)
        COMMAND(BB, 0x0015, READ_CONNECTION_ACCEPT_TIMEOUT)
        COMMAND(BB, 0x0016, WRITE_CONNECTION_ACCEPT_TIMEOUT)
        COMMAND(BB, 0x0017, READ_PAGE_TIMEOUT)
        COMMAND(BB, 0x0018, WRITE_PAGE_TIMEOUT)
        COMMAND(BB, 0x0019, READ_SCAN_ENABLE)
        COMMAND(BB, 0x001a, WRITE_SCAN_ENABLE)
        COMMAND(BB, 0x001b, READ_PAGE_SCAN_ACTIVITY)
        COMMAND(BB, 0x001c, WRITE_PAGE_SCAN_ACTIVITY)
        COMMAND(BB, 0x001d, READ_INQUIRY_SCAN_ACTIVITY)
        COMMAND(BB, 0x001e, WRITE_INQUIRY_SCAN_ACTIVITY)
        COMMAND(BB, 0x001f, READ_AUTHENTICATION_ENABLE)
        COMMAND(BB, 0x0020, WRITE_AUTHENTICATION_ENABLE)
        COMMAND(BB, 0x0023, READ_CLASS_OF_DEVICE)
        COMMAND(BB, 0x0024, WRITE_CLASS_OF_DEVICE)
        COMMAND(BB, 0x0025, READ_VOICE_SETTING)
        COMMAND(BB, 0x0026, WRITE_VOICE_SETTING)
        COMMAND(BB, 0x0027, READ_AUTOMATIC_FLUSH_TIMEOUT)
        COMMAND(BB, 0x0028, WRITE_AUTOMATIC_FLUSH_TIMEOUT)
        COMMAND(BB, 0x0029, READ_NUM_BROADCAST_RETRANSMISSIONS)
        COMMAND(BB, 0x002a, WRITE_NUM_BROADCAST_RETRANSMISSIONS)
        COMMAND(BB, 0x002b, READ_HOLD_MODE_ACTIVITY)
        COMMAND(BB, 0x002c, WRITE_HOLD_MODE_ACTIVITY)
        COMMAND(BB, 0x002d, READ_TRANSMIT_POWER_LEVEL)
        COMMAND(BB, 0x002e, READ_SYNCHRONOUS_FLOW_CONTROL_ENABLE)
        COMMAND(BB, 0x002f, WRITE_SYNCHRONOUS_FLOW_CONTROL_ENABLE)
        COMMAND(BB, 0x0031, SET_CONTROLLER_TO_HOST_FLOW_CONTROL)
        COMMAND(BB, 0x0033, HOST_BUFFER_SIZE)
        COMMAND(BB, 0x0035, HOST_NUMBER_OF_COMPLETED_PACKETS)
        COMMAND(BB, 0x0036, READ_LINK_SUPERVISION_TIMEOUT)
        COMMAND(BB, 0x0037, WRITE_LINK_SUPERVISION_TIMEOUT)
        COMMAND(BB, 0x0038, READ_NUMBER_OF_SUPPORTED_IAC)
        COMMAND(BB, 0x0039, READ_CURRENT_IAC_LAP)
        COMMAND(BB, 0x003a, WRITE_CURRENT_IAC_LAP)
        COMMAND(BB, 0x003f, SET_AFH_HOST_CHANNEL_CLASSIFICATION)
        COMMAND(BB, 0x0042, READ_INQUIRY_SCAN_TYPE)
        COMMAND(BB, 0x0043, WRITE_INQUIRY_SCAN_TYPE)
        COMMAND(BB, 0x0044, READ_INQUIRY_MODE)
        COMMAND(BB, 0x0045, WRITE_INQUIRY_MODE)
        COMMAND(BB, 0x0046, READ_PAGE_SCAN_TYPE)
        COMMAND(BB, 0x0047, WRITE_PAGE_SCAN_TYPE)
        COMMAND(BB, 0x0048, READ_AFH_CHANNEL_ASSESSMENT_MODE)
        COMMAND(BB, 0x0049, WRITE_AFH_CHANNEL_ASSESSMENT_MODE)
        COMMAND(BB, 0x0051, READ_EXTENDED_INQURY_RESPONSE)
        COMMAND(BB, 0x0052, WRITE_EXTENDED_INQUIRY_RESPONSE)
        COMMAND(BB, 0x0053, REFRESH_ENCRYPTION_KEY)
        COMMAND(BB, 0x0055, READ_SIMPLE_PAIRING_MODE)
        COMMAND(BB, 0x0056, WRITE_SIMPLE_PAIING_MODE)
        COMMAND(BB, 0x0057, READ_LOCAL_OOB_DATA)
        COMMAND(BB, 0x0058, READ_INQUIRY_RESPONSE_TRANSMIT_POWER_LEVEL)
        COMMAND(BB, 0x0059, WRITE_INQUIRY_RESPONSE_TRANSMIT_POWER_LEVEL)
        COMMAND(BB, 0x0060, SEND_KEYPRESS_NOTIFICATION)
        COMMAND(BB, 0x005a, READ_DEFAULT_ERRONEOUS_DATA_REPORTING)
        COMMAND(BB, 0x005b, WRITE_DEFAULT_ERRONEOUS_DATA_REPORTING)
        COMMAND(BB, 0x005f, ENHANCED_FLUSH)
        COMMAND(BB, 0x0061, READ_LOGICAL_LINK_ACCEPT_TIMEOUT)
        COMMAND(BB, 0x0062, WRITE_LOGICAL_LINK_ACCEPT_TIMEOUT)
        COMMAND(BB, 0x0063, SET_EVENT_MASK_PAGE_2)
        COMMAND(BB, 0x0064, READ_LOCATION_DATA)
        COMMAND(BB, 0x0065, WRITE_LOCATION_DATA)
        COMMAND(BB, 0x0066, READ_FLOW_CONTROL_MODE)
        COMMAND(BB, 0x0067, WRITE_FLOW_CONTROL_MODE)
        COMMAND(BB, 0x0068, READ_ENHANCED_TRANSMIT_POWER_LEVEL)
        COMMAND(BB, 0x0069, READ_BEST_EFFORT_FLUSH_TIMEOUT)
        COMMAND(BB, 0x006a, WRITE_BEST_EFFORT_FLUSH_TIMEOUT)
        COMMAND(BB, 0x006b, SHORT_RANGE_MODE)
        COMMAND(BB, 0x006c, READ_LE_HOST_SUPPORT)
        COMMAND(BB, 0x006d, WRITE_LE_HOST_SUPPORT)
                      
        COMMAND(IF, 0x0001, READ_LOCAL_VERSION_INFORMATION)
        COMMAND(IF, 0x0002, READ_LOCAL_SUPPORTED_COMMANDS)
        COMMAND(IF, 0x0003, READ_LOCAL_SUPPORTED_FEATURES)
        COMMAND(IF, 0x0004, READ_LOCAL_EXTENDED_FEATURES)
        COMMAND(IF, 0x0005, READ_BUFFER_SIZE_COMMAND)
        COMMAND(IF, 0x0009, READ_BD_ADDR)
        COMMAND(IF, 0x000a, READ_DATA_BLOCK_SIZE)

        COMMAND(ST, 0x0001, READ_FAILED_CONTACT_COUNTER)
        COMMAND(ST, 0x0002, RESET_FAILED_CONTACT_COUNTER)
        COMMAND(ST, 0x0003, READ_LINK_QUALITY)
        COMMAND(ST, 0x0005, READ_RSSI)
        COMMAND(ST, 0x0006, READ_AFH_CHANNEL_MAP)
        COMMAND(ST, 0x0007, READ_CLOCK)
        COMMAND(ST, 0x0008, READ_ENCRYPTION_KEY_SIZE)
        COMMAND(ST, 0x0009, READ_LOCAL_AMP_INFO)
        COMMAND(ST, 0x000a, READ_LOCAL_AMP_ASSOC)
        COMMAND(ST, 0x000b, WRITE_REMOTE_AMP_ASSOC)

        COMMAND(TS, 0x0001, READ_LOOPBACK_MODE)
        COMMAND(TS, 0x0002, WRITE_LOOPBACK_MODE)
        COMMAND(TS, 0x0003, ENABLE_DEVICE_UNDER_TEST_MODE)
        COMMAND(TS, 0x0004, WRITE_SIMPLE_PAIRING_DEBUG_MODE)
        COMMAND(TS, 0x0007, ENABLE_AMP_RECEIVER_REPORTS)
        COMMAND(TS, 0x0008, AMP_TEST_END)
        COMMAND(TS, 0x0009, AMP_TEST)

        COMMAND(VS, 0x0336, PAN13XX_CHANGE_BAUD_RATE)
        COMMAND(VS, 0x010c, SLEEP_MODE_CONFIGURATIONS)
        COMMAND(VS, 0x0d2b, HCILL_PARAMETERS)

        COMMAND(LE, 0x0001, LE_SET_EVENT_MASK)
        COMMAND(LE, 0x0002, LE_READ_BUFFER_SIZE)
        COMMAND(LE, 0x0003, LE_READ_LOCAL_SUPPORTED_FEATURES)
        COMMAND(LE, 0x0005, LE_SET_RANDOM_ADDRESS)
        COMMAND(LE, 0x0006, LE_SET_ADVERTISING_PARAMETERS)
        COMMAND(LE, 0x0007, LE_READ_ADVERTISING_CHANNEL_TX_POWER)
        COMMAND(LE, 0x0008, LE_SET_ADVERTISING_DATA)
        COMMAND(LE, 0x0009, LE_SET_SCAN_RESPONSE_DATA)
        COMMAND(LE, 0x000a, LE_SET_ADVERTISE_ENABLE)
        COMMAND(LE, 0x000b, LE_SET_SCAN_PARAMETERS)
        COMMAND(LE, 0x000c, LE_SET_SCAN_ENABLE)
        COMMAND(LE, 0x000d, LE_CREATE_CONNECTION)
        COMMAND(LE, 0x000e, LE_CREATE_CONNECTION_CANCEL)
        COMMAND(LE, 0x000f, LE_READ_WHITE_LIST_SIZE)
        COMMAND(LE, 0x0010, LE_CLEAR_WHITE_LIST)
        COMMAND(LE, 0x0011, LE_ADD_DEVICE_TO_WHITE_LIST)
        COMMAND(LE, 0x0012, LE_REMOVE_DEVICE_FROM_WHITE_LIST)
        COMMAND(LE, 0x0013, LE_CONNECTION_UPDATE)
        COMMAND(LE, 0x0014, LE_SET_HOST_CHANNEL_CLASSIFICATION)
        COMMAND(LE, 0x0015, LE_READ_CHANNEL_MAP)
        COMMAND(LE, 0x0016, LE_READ_REMOTE_USED_FEATURES)
        COMMAND(LE, 0x0017, LE_ENCRYPT)
        COMMAND(LE, 0x0018, LE_RAND)
        COMMAND(LE, 0x0019, LE_START_ENCRYPTION)
        COMMAND(LE, 0x001a, LE_LONG_TERM_KEY_REQUEST_REPLY)
        COMMAND(LE, 0x001b, LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY)
        COMMAND(LE, 0x001c, LE_READ_SUPPORTED_STATES)
        COMMAND(LE, 0x001d, LE_RECEIVER_TEST)
        COMMAND(LE, 0x001e, LE_TRANSMITTER_TEST)
        COMMAND(LE, 0x001f, LE_TEST_END)
      };

      enum event {
        EVENT(0x01, INQUIRY_COMPLETE)
        EVENT(0x02, INQIRY_RESULT)
        EVENT(0x03, CONNECTION_COMPLETE)
        EVENT(0x04, CONNECTION_REQUEST)
        EVENT(0x05, DISCONNECTION_COMPLETE)
        EVENT(0x06, AUTHENTICATION_COMPLETE)
        EVENT(0x07, REMOTE_NAME_REQUEST_COMPLETE)
        EVENT(0x08, ENCRYPTION_CHANGE)
        EVENT(0x09, CHANGE_CONNECTION_LINK_KEY_COMPLETE)
        EVENT(0x0a, MASTER_LINK_KEY_COMPLETE)
        EVENT(0x0b, READ_REMOTE_SUPPORTED_FEATURES_COMPLETE)
        EVENT(0x0c, READ_REMOTE_VERSION_INFORMATION_COMPLETE)
        EVENT(0x0d, QOS_SETUP_COMPLETE)
        EVENT(0x0e, COMMAND_COMPLETE)
        EVENT(0x0f, COMMAND_STATUS)
        EVENT(0x10, HARDWARE_ERROR)
        EVENT(0x11, FLUSH_OCCURRED)
        EVENT(0x12, ROLE_CHANGE)
        EVENT(0x13, NUMBER_OF_COMPLETED_PACKETS)
        EVENT(0x14, MODE_CHANGE)
        EVENT(0x15, RETURN_LINK_KEYS)
        EVENT(0x16, PIN_CODE_REQUEST)
        EVENT(0x17, LINK_KEY_REQUEST)
        EVENT(0x18, LINK_KEY_NOTIFICATION)
        EVENT(0x19, LOOPBACK_COMMAND)
        EVENT(0x1a, DATA_BUFFER_OVERFLOW)
        EVENT(0x1b, MAX_SLOTS_CHANGE)
        EVENT(0x1c, READ_CLOCK_OFFSET_COMPLETE)
        EVENT(0x1d, CONNECTION_PACKET_TYPE_CHANGED)
        EVENT(0x1e, QOS_VIOLATION)
        EVENT(0x20, PAGE_SCAN_REPETITION_MODE_CHANGE)
        EVENT(0x21, FLOW_SPECIFICATION_COMPLETE)
        EVENT(0x22, INQUIRY_RESULT_WITH_RSSI)
        EVENT(0x23, READ_REMOTE_EXTENDED_FEATURES_COMPLETE)
        EVENT(0x2c, SYNCHRONOUS_CONNECTION_COMPLETE_EVENT)
        EVENT(0x2d, SYNCHRONOUS_CONNECTION_CHANGED)
        EVENT(0x2e, SNIFF_SUBRATING)
        EVENT(0x2f, EXTENDED_INQUIRY_RESULT)
        EVENT(0x30, ENCRYPTION_KEY_REFRESH_COMPLETE)
        EVENT(0x31, IO_CAPABILITY_REQUEST)
        EVENT(0x32, IO_CAPABILITY_RESPONSE)
        EVENT(0x33, USER_CONFIRMATION_REQUEST)
        EVENT(0x34, USER_PASSKEY_REQUEST)
        EVENT(0x35, REMOTE_OOB_DATA_REQUEST)
        EVENT(0x36, SIMPLE_PAIRING_COMPLETE)
        EVENT(0x38, LINK_SUPERVISION_TIMEOUT_CHANGED)
        EVENT(0x39, ENHANCED_FLUSH_COMPLETE)
        EVENT(0x3b, USER_PASSKEY_NOTIFICATION)
        EVENT(0x3c, KEYPRESS_NOTIFICATION)
        EVENT(0x3d, REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION)
        EVENT(0x3e, LE_META_EVENT)
        EVENT(0x40, PHYSICAL_LINK_COMPLETE)
        EVENT(0x41, CHANNEL_SELECTED)
        EVENT(0x42, DISCONNECTION_PHYSICAL_LINK_COMPLETE)
        EVENT(0x43, PHYSICAL_LINK_LOSS_EARLY_WARNING)
        EVENT(0x44, PHYSICAL_LINK_RECOVERY_EVENT)
        EVENT(0x45, LOGICAL_LINK_COMPLETE)
        EVENT(0x46, DISCONNECTION_LOGICAL_LINK_COMPLETE)
        EVENT(0x47, FLOW_SPEC_MODIFY_COMPLETE)
        EVENT(0x48, NUMBER_OF_DATA_BLOCKS_COMPLETED)
        EVENT(0x4c, SHORT_RANGE_MODE_CHANGE_COMPLETE)
        EVENT(0x4d, AMP_STATUS_CHANGE)
        EVENT(0x49, AMP_START_TEST)
        EVENT(0x4a, AMP_TEST_END)
        EVENT(0x4b, AMP_RECEIVER_REPORT)
      };

      enum le_event {
        LE_EVENT(0x01, CONNECTION_COMPLETE)
        LE_EVENT(0x02, ADVERTISING_REPORT)
        LE_EVENT(0x03, CONNECTION_UPDATE_COMPLETE)
        LE_EVENT(0x04, READ_REMOTE_USED_FEATURES_COMPLETE)
        LE_EVENT(0x05, LONG_TERM_KEY_REQUEST)
      };
    };

    namespace L2CAP {
      enum fixed_channel_id {
        SIGNALLING_CID                                                   = 0x0001,
        CONNECTIONLESS_CID                                               = 0x0002,
        AMP_MANAGER_CID                                                  = 0x0003,
        ATTRIBUTE_CID                                                    = 0x0004,
        LE_SIGNALLING_CID                                                = 0x0005,
        SECURITY_MANAGER_CID                                             = 0x0006,
        AMP_TEST_MANAGER_CID                                             = 0x003f
      };
    };

    namespace ATT {
      enum opcode {
        OPCODE_ERROR                                                     = 0x01,
        OPCODE_EXCHANGE_MTU_REQUEST                                      = 0x02,
        OPCODE_EXCHANGE_MTU_RESPONSE                                     = 0x03,
        OPCODE_FIND_INFORMATION_REQUEST                                  = 0x04,
        OPCODE_FIND_INFORMATION_RESPONSE                                 = 0x05,
        OPCODE_FIND_BY_TYPE_VALUE_REQUEST                                = 0x06,
        OPCODE_FIND_BY_TYPE_VALUE_RESPONSE                               = 0x07,
        OPCODE_READ_BY_TYPE_REQUEST                                      = 0x08,
        OPCODE_READ_BY_TYPE_RESPONSE                                     = 0x09,
        OPCODE_READ_REQUEST                                              = 0x0a,
        OPCODE_READ_RESPONSE                                             = 0x0b,
        OPCODE_READ_BLOB_REQUEST                                         = 0x0c,
        OPCODE_READ_BLOB_RESPONSE                                        = 0x0d,
        OPCODE_READ_MULTIPLE_REQUEST                                     = 0x0e,
        OPCODE_READ_MULTIPLE_RESPONSE                                    = 0x0f,
        OPCODE_READ_BY_GROUP_TYPE_REQUEST                                = 0x10,
        OPCODE_READ_BY_GROUP_TYPE_RESPONSE                               = 0x11,
        OPCODE_WRITE_REQUEST                                             = 0x12,
        OPCODE_WRITE_RESPONSE                                            = 0x13,
        OPCODE_WRITE_COMMAND                                             = 0x52,
        OPCODE_SIGNED_WRITE_COMMAND                                      = 0xd2,
        OPCODE_PREPARE_WRITE_REQUEST                                     = 0x16,
        OPCODE_PREPARE_WRITE_RESPONSE                                    = 0x17,
        OPCODE_EXECUTE_WRITE_REQUEST                                     = 0x18,
        OPCODE_EXECUTE_WRITE_RESPONSE                                    = 0x19,
        OPCODE_HANDLE_VALUE_NOTIFICATION                                 = 0x1b,
        OPCODE_HANDLE_VALUE_INDICATION                                   = 0x1d,
        OPCODE_HANDLE_VALUE_CONFIRMATION                                 = 0x1e
      };

      enum error {
        INVALID_HANDLE                                                   = 0x01,
        READ_NOT_PERMITTED                                               = 0x02,
        WRITE_NOT_PERMITTED                                              = 0x03,
        INVALID_PDU                                                      = 0x04,
        INSUFFICIENT_AUTHENTICATION                                      = 0x05,
        REQUEST_NOT_SUPPORTED                                            = 0x06,
        INVALID_OFFSET                                                   = 0x07,
        INSUFFICIENT_AUTHORIZATION                                       = 0x08,
        PREPARE_QUEUE_FULL                                               = 0x09,
        ATTRIBUTE_NOT_FOUND                                              = 0x0a,
        ATTRIBUTE_NOT_LONG                                               = 0x0b,
        INSUFFICIENT_ENCRYPTION_KEY_SIZE                                 = 0x0c,
        INVALID_ATTRIBUTE_VALUE_LENGTH                                   = 0x0d,
        UNLIKELY_ERROR                                                   = 0x0e,
        INSUFFICIENT_ENCRYPTION                                          = 0x0f,
        UNSUPPORTED_GROUP_TYPE                                           = 0x10,
        INSUFFICIENT_RESOURCES                                           = 0x11
      };
    };

    namespace GAP {
      // https://www.bluetooth.org/Technical/AssignedNumbers/generic_access_profile.htm
      enum advertising_data_type {
        FLAGS                                                            = 0x01,
        INCOMPLETE_16BIT_UUIDS                                           = 0x02,
        COMPLETE_16BIT_UUIDS                                             = 0x03,
        INCOMPLETE_32BIT_UUIDS                                           = 0x04,
        COMPLETE_32BIT_UUIDS                                             = 0x05,
        INCOMPLETE_128BIT_UUIDS                                          = 0x06,
        COMPLETE_128BIT_UUIDS                                            = 0x07,
        SHORTENED_LOCAL_NAME                                             = 0x08,
        COMPLETE_LOCAL_NAME                                              = 0x09,
        TX_POWER_LEVEL                                                   = 0x0a,
        CLASS_OF_DEVICE                                                  = 0x0d,
        SIMPLE_PAIRING_HASH_C                                            = 0x0e,
        SIMPLE_PAIRING_RANDOMIZER_R                                      = 0x0f,
        DEVICE_ID                                                        = 0x10,
        SECURITY_MANAGER_TK_VALUE                                        = 0x10,
        SECURITY_MANAGER_OUT_OF_BAND_FLAGS                               = 0x11,
        SLAVE_CONNECTION_INTERVAL_RANGE                                  = 0x12,
        SERVICE_SOLICITATION_16BIT_UUIDS                                 = 0x14,
        SERVICE_SOLICITATION_128BIT_UUIDS                                = 0x15,
        PUBLIC_TARGET_ADDRESS                                            = 0x17,
        RANDOM_TARGET_ADDRESS                                            = 0x18,
        APPEARANCE                                                       = 0x19,
        MANUFACTURER_SPECIFIC_DATA                                       = 0xff
      };

      enum advertising_flag_masks {
        LE_LIMITED_DISCOVERABLE_MODE                                     = 0x01,
        LE_GENERAL_DISCOVERABLE_MODE                                     = 0x02,
        BR_EDR_NOT_SUPPORTED                                             = 0x04,
        SIMULTANEOUS_CONTROLLER                                          = 0x08,
        SIMULTANEOUS_HOST                                                = 0x10
      };
    };

    namespace GATT {
      // https://www.bluetooth.org/Technical/AssignedNumbers/Generic-Attribute-Profile.htm

      enum service_type {
        GENERIC_ACCESS_PROFILE                                           = 0x1800,
        GENERIC_ATTRIBUTE_PROFILE                                        = 0x1801
      };

      enum attribute_type {
        PRIMARY_SERVICE                                                  = 0x2800,
        SECONDARY_SERVICE                                                = 0x2801,
        INCLUDE                                                          = 0x2801,
        CHARACTERISTIC                                                   = 0x2803
      };

      enum characteristic_descriptor {
        CHARACTERISTIC_EXTENDED_PROPERTIES                               = 0x2900,
        CHARACTERISTIC_USER_DESCRIPTION                                  = 0x2901,
        CLIENT_CHARACTERISTIC_DESCRIPTION                                = 0x2902,
        SERVER_CHARACTERISTIC_CONFIGURATION                              = 0x2903,
        CHARACTERISTIC_FORMAT                                            = 0x2904,
        CHARACTERISTIC_AGGREGATE_FORMAT                                  = 0x2905
      };

      enum characteristic_type {
        DEVICE_NAME                                                      = 0x2a00,
        APPEARANCE                                                       = 0x2a01,
        PERIPHERAL_PRIVACY_FLAG                                          = 0x2a02,
        RECONNECTION_ADDRESS                                             = 0x2a03,
        PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS                       = 0x2a04,
        SERVICE_CHANGED                                                  = 0x2a05
      };

      enum attribute_property_masks {
        BROADCAST                                                        = 0x01,
        READ                                                             = 0x02,
        WRITE_WITHOUT_RESPONSE                                           = 0x04,
        WRITE                                                            = 0x08,
        NOTIFY                                                           = 0x10,
        INDICATE                                                         = 0x20,
        AUTHENTICATED_SIGNED_WRITES                                      = 0x40,
        EXTENDED_PROPERTIES                                              = 0x80
      };

      enum characteristic_formats {
        BOOLEAN                                                          = 0x01,
        TWOBIT                                                           = 0x02,
        NIBBLE                                                           = 0x03,
        UINT8                                                            = 0x04,
        UINT12                                                           = 0x05,
        UINT16                                                           = 0x06,
        UINT24                                                           = 0x07,
        UINT32                                                           = 0x08,
        UINT48                                                           = 0x09,
        UINT64                                                           = 0x0a,
        UINT128                                                          = 0x0b,
        SINT8                                                            = 0x0c,
        SINT12                                                           = 0x0d,
        SINT16                                                           = 0x0e,
        SINT24                                                           = 0x0f,
        SINT32                                                           = 0x10,
        SINT48                                                           = 0x11,
        SINT64                                                           = 0x12,
        SINT128                                                          = 0x13,
        FLOAT32                                                          = 0x14,
        FLOAT64                                                          = 0x15,
        SFLOAT                                                           = 0x16,
        FLOAT                                                            = 0x17,
        DUINT16                                                          = 0x18,
        UTF8S                                                            = 0x19,
        UTF16S                                                           = 0x1a,
        STRUCT                                                           = 0x1b
      };
    };
  };
};
