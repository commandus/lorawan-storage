#ifndef LORAWAN_ERROR_H_
#define LORAWAN_ERROR_H_	1

// syslog
#if defined(_MSC_VER) or defined(ESP_PLATFORM)
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug(-level messages */
#else
#include <syslog.h>
#endif

// Error codes
#define CODE_OK            					0
#define ERR_CODE_COMMAND_LINE		    	(-5000)
#define ERR_CODE_OPEN_DEVICE		    	(-5001)
#define ERR_CODE_CLOSE_DEVICE		    	(-5002)
#define ERR_CODE_BAD_STATUS		        	(-5003)
#define ERR_CODE_INVALID_PAR_LOG_FILE   	(-5004)
#define ERR_CODE_INVALID_SERVICE			(-5005)
#define ERR_CODE_INVALID_GATEWAY_ID			(-5006)
#define ERR_CODE_INVALID_DEVICE_EUI			(-5007)
#define ERR_CODE_INVALID_BUFFER_SIZE		(-5008)
#define ERR_CODE_GRPC_NETWORK_SERVER_FAIL	(-5009)
#define ERR_CODE_INVALID_RFM_HEADER			(-5010)
#define ERR_CODE_INVALID_ADDRESS			(-5011)
#define ERR_CODE_INVALID_FAMILY				(-5012)
#define ERR_CODE_SOCKET_CREATE		    	(-5013)
#define ERR_CODE_SOCKET_BIND		    	(-5014)
#define ERR_CODE_SOCKET_OPEN		    	(-5015)
#define ERR_CODE_SOCKET_CLOSE		    	(-5016)
#define ERR_CODE_SOCKET_READ		    	(-5017)
#define ERR_CODE_SOCKET_WRITE		    	(-5018)
#define ERR_CODE_SOCKET_NO_ONE				(-5019)
#define ERR_CODE_SOCKET_CONNECT		    	(-5020)
#define ERR_CODE_SOCKET_ADDRESS		    	(-5021)
#define ERR_CODE_SOCKET_LISTEN		    	(-5022)
#define ERR_CODE SOCKET_SET                 (-5023)
#define ERR_CODE_SELECT						(-5024)
#define ERR_CODE_INVALID_PACKET				(-5025)
#define ERR_CODE_INVALID_JSON				(-5026)
#define ERR_CODE_DEVICE_ADDRESS_NOTFOUND	(-5027)
#define ERR_CODE_FAIL_IDENTITY_SERVICE		(-5028)
#define ERR_CODE_LMDB_TXN_BEGIN				(-5029)
#define ERR_CODE_LMDB_TXN_COMMIT			(-5030)
#define ERR_CODE_LMDB_OPEN					(-5031)
#define ERR_CODE_LMDB_CLOSE					(-5032)
#define ERR_CODE_LMDB_PUT					(-5033)
#define ERR_CODE_LMDB_PUT_PROBE				(-5034)
#define ERR_CODE_LMDB_GET					(-5035)
#define ERR_CODE_WRONG_PARAM				(-5036)
#define ERR_CODE_INSUFFICIENT_MEMORY		(-5037)
#define ERR_CODE_NO_CONFIG					(-5038)
#define ERR_CODE_SEND_ACK					(-5039)
#define ERR_CODE_NO_GATEWAY_STAT			(-5040)
#define ERR_CODE_INVALID_PROTOCOL_VERSION	(-5041)
#define ERR_CODE_PACKET_TOO_SHORT			(-5042)
#define ERR_CODE_PARAM_NO_INTERFACE			(-5043)
#define ERR_CODE_MAC_TOO_SHORT				(-5044)
#define ERR_CODE_MAC_INVALID				(-5045)
#define ERR_CODE_MAC_UNKNOWN_EXTENSION		(-5046)
#define ERR_CODE_PARAM_INVALID				(-5047)
#define ERR_CODE_INSUFFICIENT_PARAMS		(-5048)
#define ERR_CODE_NO_MAC_NO_PAYLOAD          (-5049)
#define ERR_CODE_INVALID_REGEX              (-5050)
#define ERR_CODE_NO_DATABASE               	(-5051)
#define ERR_CODE_LOAD_PROTO					(-5052)
#define ERR_CODE_LOAD_DATABASE_CONFIG		(-5053)
#define ERR_CODE_DB_SELECT					(-5054)
#define ERR_CODE_DB_DATABASE_NOT_FOUND		(-5055)
#define ERR_CODE_DB_DATABASE_OPEN			(-5056)
#define ERR_CODE_DB_DATABASE_CLOSE			(-5057)
#define ERR_CODE_DB_CREATE					(-5058)
#define ERR_CODE_DB_INSERT					(-5059)
#define ERR_CODE_DB_START_TRANSACTION		(-5060)
#define ERR_CODE_DB_COMMIT_TRANSACTION		(-5061)
#define ERR_CODE_DB_EXEC					(-5062)
#define ERR_CODE_PING						(-5063)
#define ERR_CODE_PULLOUT					(-5064)
#define ERR_CODE_INVALID_STAT				(-5065)
#define ERR_CODE_NO_PAYLOAD          		(-5066)
#define ERR_CODE_NO_MESSAGE_TYPE			(-5067)
#define ERR_CODE_QUEUE_EMPTY				(-5068)
#define ERR_CODE_RM_FILE					(-5069)
#define ERR_CODE_INVALID_BASE64				(-5070)
#define ERR_CODE_MISSED_DEVICE				(-5071)
#define ERR_CODE_MISSED_GATEWAY				(-5072)
#define ERR_CODE_INVALID_FPORT				(-5073)
#define ERR_CODE_INVALID_MIC				(-5074)
#define ERR_CODE_SEGMENTATION_FAULT			(-5075)
#define ERR_CODE_ABRT           			(-5076)
#define ERR_CODE_BEST_GATEWAY_NOT_FOUND		(-5077)
#define ERR_CODE_REPLY_MAC					(-5078)
#define ERR_CODE_NO_MAC			           	(-5079)
#define ERR_CODE_NO_DEVICE_STAT				(-5080)
#define ERR_CODE_INIT_DEVICE_STAT			(-5081)
#define ERR_CODE_INIT_IDENTITY				(-5082)
#define ERR_CODE_INIT_QUEUE					(-5083)
#define ERR_CODE_HANGUP_DETECTED			(-5084)
#define ERR_CODE_NO_FCNT_DOWN				(-5085)
#define ERR_CODE_CONTROL_NOT_AUTHORIZED		(-5086)
#define ERR_CODE_GATEWAY_NOT_FOUND			(-5087)
#define ERR_CODE_CONTROL_DEVICE_NOT_FOUND	(-5088)
#define ERR_CODE_INVALID_CONTROL_PACKET		(-5089)
#define ERR_CODE_DUPLICATED_PACKET			(-5090)
#define ERR_CODE_INIT_GW_STAT   			(-5091)
#define ERR_CODE_DEVICE_NAME_NOT_FOUND      (-5092)
#define ERR_CODE_DEVICE_EUI_NOT_FOUND       (-5093)
#define ERR_CODE_JOIN_EUI_NOT_MATCHED       (-5094)
#define ERR_CODE_GATEWAY_NO_YET_PULL_DATA   (-5095)
#define ERR_CODE_REGION_BAND_EMPTY          (-5096)
#define ERR_CODE_INIT_REGION_BANDS          (-5097)
#define ERR_CODE_INIT_REGION_NO_DEFAULT     (-5098)
#define ERR_CODE_NO_REGION_BAND             (-5099)
#define ERR_CODE_REGION_BAND_NO_DEFAULT     (-5100)
#define ERR_CODE_IS_JOIN					(-5101)
#define ERR_CODE_BAD_JOIN_REQUEST			(-5102)
#define ERR_CODE_NETID_OR_NETTYPE_MISSED    (-5103)
#define ERR_CODE_NETTYPE_OUT_OF_RANGE       (-5104)
#define ERR_CODE_NETID_OUT_OF_RANGE         (-5105)
#define ERR_CODE_TYPE_OUT_OF_RANGE          (-5106)
#define ERR_CODE_NWK_OUT_OF_RANGE           (-5107)
#define ERR_CODE_ADDR_OUT_OF_RANGE          (-5108)
#define ERR_CODE_ADDR_SPACE_FULL            (-5109)
#define ERR_CODE_INIT_LOGGER_HUFFMAN_PARSER (-5110)
#define ERR_CODE_WS_START_FAILED			(-5111)
#define ERR_CODE_NO_DEFAULT_WS_DATABASE		(-5112)
#define ERR_CODE_INIT_LOGGER_HUFFMAN_DB     (-5113)
#define ERR_CODE_NO_PACKET_PARSER           (-5114)
#define ERR_CODE_LOAD_WS_PASSWD_NOT_FOUND   (-5115)

// embedded gateway config)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED       (-5116)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_TIME_STAMP         (-5117)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_SX1261_RADIO       (-5118)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT        (-5119)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_INVALID_RADIO      (-5120)

#define ERR_CODE_LORA_GATEWAY_CONFIGURE_DEMODULATION       (-5121)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL   (-5122)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_STD_CHANNEL        (-5123)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL        (-5124)
#define ERR_CODE_LORA_GATEWAY_CONFIGURE_DEBUG              (-5125)

#define ERR_CODE_LORA_GATEWAY_CONFIGURE_GPS_FAILED         (-5126)
#define ERR_CODE_LORA_GATEWAY_START_FAILED                 (-5127)
#define ERR_CODE_LORA_GATEWAY_GET_EUI                      (-5128)
#define ERR_CODE_LORA_GATEWAY_GPS_GET_TIME                 (-5129)
#define ERR_CODE_LORA_GATEWAY_GPS_SYNC_TIME                (-5130)
#define ERR_CODE_LORA_GATEWAY_GPS_DISABLED                 (-5131)

#define ERR_CODE_LORA_GATEWAY_GPS_GET_COORDS               (-5132)
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_START_FAILED   (-5133)
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_TIMEOUT        (-5134)
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_FAILED         (-5135)
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_ABORTED        (-5136)
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_UNEXPECTED_STATUS (-5137)
#define ERR_CODE_LORA_GATEWAY_GET_TX_STATUS                (-5138)
#define ERR_CODE_LORA_GATEWAY_SKIP_SPECTRAL_SCAN            (-5139)
#define ERR_CODE_LORA_GATEWAY_STATUS_FAILED                 (-5140)
#define ERR_CODE_LORA_GATEWAY_EMIT_ALLREADY                 (-5141)
#define ERR_CODE_LORA_GATEWAY_SCHEDULED_ALLREADY            (-5142)
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_ABORT_FAILED    (-5143)
#define ERR_CODE_LORA_GATEWAY_SEND_FAILED                   (-5144)
#define ERR_CODE_LORA_GATEWAY_SENT                          (-5145)
#define ERR_CODE_LORA_GATEWAY_JIT_DEQUEUE_FAILED            (-5146)
#define ERR_CODE_LORA_GATEWAY_JIT_PEEK_FAILED               (-5147)
#define ERR_CODE_LORA_GATEWAY_JIT_ENQUEUE_FAILED            (-5148)
#define ERR_CODE_LORA_GATEWAY_FETCH                         (-5149)
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_STATUS                (-5150)
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_DATARATE              (-5151)
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_BANDWIDTH             (-5152)
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_CODERATE              (-5153)
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_MODULATION            (-5154)
#define ERR_CODE_LORA_GATEWAY_RECEIVED                      (-5155)
#define ERR_CODE_LORA_GATEWAY_AUTOQUIT_THRESHOLD            (-5156)
#define ERR_CODE_LORA_GATEWAY_BEACON_FAILED                 (-5157)
#define ERR_CODE_LORA_GATEWAY_UNKNOWN_TX_MODE               (-5158)
#define ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME              (-5159)
#define ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME_DISABLED     (-5160)
#define ERR_CODE_LORA_GATEWAY_SEND_AT_GPS_TIME_INVALID      (-5161)
#define ERR_CODE_LORA_GATEWAY_TX_CHAIN_DISABLED             (-5162)
#define ERR_CODE_LORA_GATEWAY_TX_UNSUPPORTED_FREQUENCY      (-5163)
#define ERR_CODE_LORA_GATEWAY_TX_UNSUPPORTED_POWER          (-5164)
#define ERR_CODE_LORA_GATEWAY_USB_NOT_FOUND                 (-5165)
#define ERR_CODE_LORA_GATEWAY_SHUTDOWN_TIMEOUT              (-5166)
#define ERR_CODE_LORA_GATEWAY_STOP_FAILED                   (-5167)
#define ERR_CODE_INIT_PLUGINS_FAILED                        (-5168)
#define ERR_CODE_LOAD_PLUGINS_FAILED                        (-5169)
#define ERR_CODE_PLUGIN_MQTT_CONNECT                        (-5170)
#define ERR_CODE_PLUGIN_MQTT_DISCONNECT                     (-5171)
#define ERR_CODE_PLUGIN_MQTT_SEND                           (-5172)
#define ERR_CODE_UNIDENTIFIED_MESSAGE                       (-5173)
#define ERR_CODE_LORA_GATEWAY_SPECTRAL_SCAN_RESULT          (-5174)
#define ERR_CODE_STOPPED                                    (-5175)
#define ERR_CODE_ACCESS_DENIED                              (-5176)

#endif
