/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _RX_MPDU_INFO_H_
#define _RX_MPDU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rxpt_classify_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct rxpt_classify_info rxpt_classify_info_details;
//	1	rx_reo_queue_desc_addr_31_0[31:0]
//	2	rx_reo_queue_desc_addr_39_32[7:0], receive_queue_number[23:8], pre_delim_err_warning[24], first_delim_err[25], reserved_2a[31:26]
//	3	pn_31_0[31:0]
//	4	pn_63_32[31:0]
//	5	pn_95_64[31:0]
//	6	pn_127_96[31:0]
//	7	epd_en[0], all_frames_shall_be_encrypted[1], encrypt_type[5:2], wep_key_width_for_variable_key[7:6], mesh_sta[9:8], bssid_hit[10], bssid_number[14:11], tid[18:15], reserved_7a[31:19]
//	8	peer_meta_data[31:0]
//	9	rxpcu_mpdu_filter_in_category[1:0], sw_frame_group_id[8:2], ndp_frame[9], phy_err[10], phy_err_during_mpdu_header[11], protocol_version_err[12], ast_based_lookup_valid[13], reserved_9a[15:14], phy_ppdu_id[31:16]
//	10	ast_index[15:0], sw_peer_id[31:16]
//	11	mpdu_frame_control_valid[0], mpdu_duration_valid[1], mac_addr_ad1_valid[2], mac_addr_ad2_valid[3], mac_addr_ad3_valid[4], mac_addr_ad4_valid[5], mpdu_sequence_control_valid[6], mpdu_qos_control_valid[7], mpdu_ht_control_valid[8], frame_encryption_info_valid[9], mpdu_fragment_number[13:10], more_fragment_flag[14], reserved_11a[15], fr_ds[16], to_ds[17], encrypted[18], mpdu_retry[19], mpdu_sequence_number[31:20]
//	12	key_id_octet[7:0], new_peer_entry[8], decrypt_needed[9], decap_type[11:10], rx_insert_vlan_c_tag_padding[12], rx_insert_vlan_s_tag_padding[13], strip_vlan_c_tag_decap[14], strip_vlan_s_tag_decap[15], pre_delim_count[27:16], ampdu_flag[28], bar_frame[29], raw_mpdu[30], reserved_12[31]
//	13	mpdu_length[13:0], first_mpdu[14], mcast_bcast[15], ast_index_not_found[16], ast_index_timeout[17], power_mgmt[18], non_qos[19], null_data[20], mgmt_type[21], ctrl_type[22], more_data[23], eosp[24], fragment_flag[25], order[26], u_apsd_trigger[27], encrypt_required[28], directed[29], amsdu_present[30], reserved_13[31]
//	14	mpdu_frame_control_field[15:0], mpdu_duration_field[31:16]
//	15	mac_addr_ad1_31_0[31:0]
//	16	mac_addr_ad1_47_32[15:0], mac_addr_ad2_15_0[31:16]
//	17	mac_addr_ad2_47_16[31:0]
//	18	mac_addr_ad3_31_0[31:0]
//	19	mac_addr_ad3_47_32[15:0], mpdu_sequence_control_field[31:16]
//	20	mac_addr_ad4_31_0[31:0]
//	21	mac_addr_ad4_47_32[15:0], mpdu_qos_control_field[31:16]
//	22	mpdu_ht_control_field[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_MPDU_INFO 23

struct rx_mpdu_info {
    struct            rxpt_classify_info                       rxpt_classify_info_details;
             uint32_t rx_reo_queue_desc_addr_31_0     : 32; //[31:0]
             uint32_t rx_reo_queue_desc_addr_39_32    :  8, //[7:0]
                      receive_queue_number            : 16, //[23:8]
                      pre_delim_err_warning           :  1, //[24]
                      first_delim_err                 :  1, //[25]
                      reserved_2a                     :  6; //[31:26]
             uint32_t pn_31_0                         : 32; //[31:0]
             uint32_t pn_63_32                        : 32; //[31:0]
             uint32_t pn_95_64                        : 32; //[31:0]
             uint32_t pn_127_96                       : 32; //[31:0]
             uint32_t epd_en                          :  1, //[0]
                      all_frames_shall_be_encrypted   :  1, //[1]
                      encrypt_type                    :  4, //[5:2]
                      wep_key_width_for_variable_key  :  2, //[7:6]
                      mesh_sta                        :  2, //[9:8]
                      bssid_hit                       :  1, //[10]
                      bssid_number                    :  4, //[14:11]
                      tid                             :  4, //[18:15]
                      reserved_7a                     : 13; //[31:19]
             uint32_t peer_meta_data                  : 32; //[31:0]
             uint32_t rxpcu_mpdu_filter_in_category   :  2, //[1:0]
                      sw_frame_group_id               :  7, //[8:2]
                      ndp_frame                       :  1, //[9]
                      phy_err                         :  1, //[10]
                      phy_err_during_mpdu_header      :  1, //[11]
                      protocol_version_err            :  1, //[12]
                      ast_based_lookup_valid          :  1, //[13]
                      reserved_9a                     :  2, //[15:14]
                      phy_ppdu_id                     : 16; //[31:16]
             uint32_t ast_index                       : 16, //[15:0]
                      sw_peer_id                      : 16; //[31:16]
             uint32_t mpdu_frame_control_valid        :  1, //[0]
                      mpdu_duration_valid             :  1, //[1]
                      mac_addr_ad1_valid              :  1, //[2]
                      mac_addr_ad2_valid              :  1, //[3]
                      mac_addr_ad3_valid              :  1, //[4]
                      mac_addr_ad4_valid              :  1, //[5]
                      mpdu_sequence_control_valid     :  1, //[6]
                      mpdu_qos_control_valid          :  1, //[7]
                      mpdu_ht_control_valid           :  1, //[8]
                      frame_encryption_info_valid     :  1, //[9]
                      mpdu_fragment_number            :  4, //[13:10]
                      more_fragment_flag              :  1, //[14]
                      reserved_11a                    :  1, //[15]
                      fr_ds                           :  1, //[16]
                      to_ds                           :  1, //[17]
                      encrypted                       :  1, //[18]
                      mpdu_retry                      :  1, //[19]
                      mpdu_sequence_number            : 12; //[31:20]
             uint32_t key_id_octet                    :  8, //[7:0]
                      new_peer_entry                  :  1, //[8]
                      decrypt_needed                  :  1, //[9]
                      decap_type                      :  2, //[11:10]
                      rx_insert_vlan_c_tag_padding    :  1, //[12]
                      rx_insert_vlan_s_tag_padding    :  1, //[13]
                      strip_vlan_c_tag_decap          :  1, //[14]
                      strip_vlan_s_tag_decap          :  1, //[15]
                      pre_delim_count                 : 12, //[27:16]
                      ampdu_flag                      :  1, //[28]
                      bar_frame                       :  1, //[29]
                      raw_mpdu                        :  1, //[30]
                      reserved_12                     :  1; //[31]
             uint32_t mpdu_length                     : 14, //[13:0]
                      first_mpdu                      :  1, //[14]
                      mcast_bcast                     :  1, //[15]
                      ast_index_not_found             :  1, //[16]
                      ast_index_timeout               :  1, //[17]
                      power_mgmt                      :  1, //[18]
                      non_qos                         :  1, //[19]
                      null_data                       :  1, //[20]
                      mgmt_type                       :  1, //[21]
                      ctrl_type                       :  1, //[22]
                      more_data                       :  1, //[23]
                      eosp                            :  1, //[24]
                      fragment_flag                   :  1, //[25]
                      order                           :  1, //[26]
                      u_apsd_trigger                  :  1, //[27]
                      encrypt_required                :  1, //[28]
                      directed                        :  1, //[29]
                      amsdu_present                   :  1, //[30]
                      reserved_13                     :  1; //[31]
             uint32_t mpdu_frame_control_field        : 16, //[15:0]
                      mpdu_duration_field             : 16; //[31:16]
             uint32_t mac_addr_ad1_31_0               : 32; //[31:0]
             uint32_t mac_addr_ad1_47_32              : 16, //[15:0]
                      mac_addr_ad2_15_0               : 16; //[31:16]
             uint32_t mac_addr_ad2_47_16              : 32; //[31:0]
             uint32_t mac_addr_ad3_31_0               : 32; //[31:0]
             uint32_t mac_addr_ad3_47_32              : 16, //[15:0]
                      mpdu_sequence_control_field     : 16; //[31:16]
             uint32_t mac_addr_ad4_31_0               : 32; //[31:0]
             uint32_t mac_addr_ad4_47_32              : 16, //[15:0]
                      mpdu_qos_control_field          : 16; //[31:16]
             uint32_t mpdu_ht_control_field           : 32; //[31:0]
};

/*

struct rxpt_classify_info rxpt_classify_info_details
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			RXOLE related classification info
			
			<legal all

rx_reo_queue_desc_addr_31_0
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Address (lower 32 bits) of the REO queue descriptor. 
			
			
			
			If no Peer entry lookup happened for this frame, the
			value wil be set to 0, and the frame shall never be pushed
			to REO entrance ring.
			
			<legal all>

rx_reo_queue_desc_addr_39_32
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Address (upper 8 bits) of the REO queue descriptor. 
			
			
			
			If no Peer entry lookup happened for this frame, the
			value wil be set to 0, and the frame shall never be pushed
			to REO entrance ring.
			
			<legal all>

receive_queue_number
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Indicates the MPDU queue ID to which this MPDU link
			descriptor belongs
			
			Used for tracking and debugging
			
			<legal all>

pre_delim_err_warning
			
			Indicates that a delimiter FCS error was found in
			between the Previous MPDU and this MPDU.
			
			
			
			Note that this is just a warning, and does not mean that
			this MPDU is corrupted in any way. If it is, there will be
			other errors indicated such as FCS or decrypt errors
			
			
			
			In case of ndp or phy_err, this field will indicate at
			least one of delimiters located after the last MPDU in the
			previous PPDU has been corrupted.

first_delim_err
			
			Indicates that the first delimiter had a FCS failure. 
			Only valid when first_mpdu and first_msdu are set.
			
			
			

reserved_2a
			
			<legal 0>

pn_31_0
			
			
			
			
			
			WEP: IV = {key_id_octet, pn2, pn1, pn0}.  Only pn[23:0]
			is valid.
			
			TKIP: IV = {pn5, pn4, pn3, pn2, key_id_octet, pn0,
			WEPSeed[1], pn1}.  Only pn[47:0] is valid.
			
			AES-CCM: IV = {pn5, pn4, pn3, pn2, key_id_octet, 0x0,
			pn1, pn0}.  Only pn[47:0] is valid.
			
			WAPI: IV = {key_id_octet, 0x0, pn15, pn14, pn13, pn12,
			pn11, pn10, pn9, pn8, pn7, pn6, pn5, pn4, pn3, pn2, pn1,
			pn0}.  pn[127:0] are valid.
			
			
			

pn_63_32
			
			
			
			
			Bits [63:32] of the PN number.   See description for
			pn_31_0.
			
			
			

pn_95_64
			
			
			
			
			Bits [95:64] of the PN number.  See description for
			pn_31_0.
			
			
			

pn_127_96
			
			
			
			
			Bits [127:96] of the PN number.  See description for
			pn_31_0.
			
			
			

epd_en
			
			Field only valid when AST_based_lookup_valid == 1.
			
			
			
			
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			If set to one use EPD instead of LPD
			
			
			
			
			<legal all>

all_frames_shall_be_encrypted
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			When set, all frames (data only ?) shall be encrypted.
			If not, RX CRYPTO shall set an error flag.
			
			<legal all>

encrypt_type
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Indicates type of decrypt cipher used (as defined in the
			peer entry)
			
			
			
			<enum 0 wep_40> WEP 40-bit
			
			<enum 1 wep_104> WEP 104-bit
			
			<enum 2 tkip_no_mic> TKIP without MIC
			
			<enum 3 wep_128> WEP 128-bit
			
			<enum 4 tkip_with_mic> TKIP with MIC
			
			<enum 5 wapi> WAPI
			
			<enum 6 aes_ccmp_128> AES CCMP 128
			
			<enum 7 no_cipher> No crypto
			
			<enum 8 aes_ccmp_256> AES CCMP 256
			
			<enum 9 aes_gcmp_128> AES CCMP 128
			
			<enum 10 aes_gcmp_256> AES CCMP 256
			
			<enum 11 wapi_gcm_sm4> WAPI GCM SM4
			
			
			
			<enum 12 wep_varied_width> WEP encryption. As for WEP
			per keyid the key bit width can vary, the key bit width for
			this MPDU will be indicated in field
			wep_key_width_for_variable key
			
			<legal 0-12>

wep_key_width_for_variable_key
			
			Field only valid when key_type is set to
			wep_varied_width. 
			
			
			
			This field indicates the size of the wep key for this
			MPDU.
			
			 
			
			<enum 0 wep_varied_width_40> WEP 40-bit
			
			<enum 1 wep_varied_width_104> WEP 104-bit
			
			<enum 2 wep_varied_width_128> WEP 128-bit
			
			
			
			<legal 0-2>

mesh_sta
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			When set, this is a Mesh (11s) STA.
			
			
			
			The interpretation of the A-MSDU 'Length' field in the
			MPDU (if any) is decided by the e-numerations below.
			
			
			
			<enum 0 MESH_DISABLE>
			
			<enum 1 MESH_Q2Q> A-MSDU 'Length' is big endian and
			includes the length of Mesh Control.
			
			<enum 2 MESH_11S_BE> A-MSDU 'Length' is big endian and
			excludes the length of Mesh Control.
			
			<enum 3 MESH_11S_LE> A-MSDU 'Length' is little endian
			and excludes the length of Mesh Control. This is
			802.11s-compliant.
			
			<legal all>

bssid_hit
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			When set, the BSSID of the incoming frame matched one of
			the 8 BSSID register values
			
			
			
			<legal all>

bssid_number
			
			Field only valid when bssid_hit is set.
			
			
			
			This number indicates which one out of the 8 BSSID
			register values matched the incoming frame
			
			<legal all>

tid
			
			Field only valid when mpdu_qos_control_valid is set
			
			
			
			The TID field in the QoS control field
			
			<legal all>

reserved_7a
			
			<legal 0>

peer_meta_data
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Meta data that SW has programmed in the Peer table entry
			of the transmitting STA.
			
			<legal all>

rxpcu_mpdu_filter_in_category
			
			Field indicates what the reason was that this MPDU frame
			was allowed to come into the receive path by RXPCU
			
			<enum 0 rxpcu_filter_pass> This MPDU passed the normal
			frame filter programming of rxpcu
			
			<enum 1 rxpcu_monitor_client> This MPDU did NOT pass the
			regular frame filter and would have been dropped, were it
			not for the frame fitting into the 'monitor_client'
			category.
			
			<enum 2 rxpcu_monitor_other> This MPDU did NOT pass the
			regular frame filter and also did not pass the
			rxpcu_monitor_client filter. It would have been dropped
			accept that it did pass the 'monitor_other' category.
			
			
			
			Note: for ndp frame, if it was expected because the
			preceding NDPA was filter_pass, the setting 
			rxpcu_filter_pass will be used. This setting will also be
			used for every ndp frame in case Promiscuous mode is
			enabled.
			
			
			
			In case promiscuous is not enabled, and an NDP is not
			preceded by a NPDA filter pass frame, the only other setting
			that could appear here for the NDP is rxpcu_monitor_other. 
			
			(rxpcu has a configuration bit specifically for this
			scenario)
			
			
			
			Note: for 
			
			<legal 0-2>

sw_frame_group_id
			
			SW processes frames based on certain classifications.
			This field indicates to what sw classification this MPDU is
			mapped.
			
			The classification is given in priority order
			
			
			
			<enum 0 sw_frame_group_NDP_frame> Note: The
			corresponding Rxpcu_Mpdu_filter_in_category can be
			rxpcu_filter_pass or rxpcu_monitor_other
			
			
			
			<enum 1 sw_frame_group_Multicast_data> 
			
			<enum 2 sw_frame_group_Unicast_data> 
			
			<enum 3 sw_frame_group_Null_data > This includes mpdus
			of type Data Null as well as QoS Data Null
			
			
			
			<enum 4 sw_frame_group_mgmt_0000 > 
			
			<enum 5 sw_frame_group_mgmt_0001 > 
			
			<enum 6 sw_frame_group_mgmt_0010 > 
			
			<enum 7 sw_frame_group_mgmt_0011 > 
			
			<enum 8 sw_frame_group_mgmt_0100 > 
			
			<enum 9 sw_frame_group_mgmt_0101 > 
			
			<enum 10 sw_frame_group_mgmt_0110 > 
			
			<enum 11 sw_frame_group_mgmt_0111 > 
			
			<enum 12 sw_frame_group_mgmt_1000 > 
			
			<enum 13 sw_frame_group_mgmt_1001 > 
			
			<enum 14 sw_frame_group_mgmt_1010 > 
			
			<enum 15 sw_frame_group_mgmt_1011 > 
			
			<enum 16 sw_frame_group_mgmt_1100 > 
			
			<enum 17 sw_frame_group_mgmt_1101 > 
			
			<enum 18 sw_frame_group_mgmt_1110 > 
			
			<enum 19 sw_frame_group_mgmt_1111 > 
			
			
			
			<enum 20 sw_frame_group_ctrl_0000 > 
			
			<enum 21 sw_frame_group_ctrl_0001 > 
			
			<enum 22 sw_frame_group_ctrl_0010 > 
			
			<enum 23 sw_frame_group_ctrl_0011 > 
			
			<enum 24 sw_frame_group_ctrl_0100 > 
			
			<enum 25 sw_frame_group_ctrl_0101 > 
			
			<enum 26 sw_frame_group_ctrl_0110 > 
			
			<enum 27 sw_frame_group_ctrl_0111 > 
			
			<enum 28 sw_frame_group_ctrl_1000 > 
			
			<enum 29 sw_frame_group_ctrl_1001 > 
			
			<enum 30 sw_frame_group_ctrl_1010 > 
			
			<enum 31 sw_frame_group_ctrl_1011 > 
			
			<enum 32 sw_frame_group_ctrl_1100 > 
			
			<enum 33 sw_frame_group_ctrl_1101 > 
			
			<enum 34 sw_frame_group_ctrl_1110 > 
			
			<enum 35 sw_frame_group_ctrl_1111 > 
			
			
			
			<enum 36 sw_frame_group_unsupported> This covers type 3
			and protocol version != 0 
			
			Note: The corresponding Rxpcu_Mpdu_filter_in_category
			can only be rxpcu_monitor_other
			
			
			
			
			Note: The corresponding Rxpcu_Mpdu_filter_in_category
			can be rxpcu_filter_pass
			
			
			
			<legal 0-37>

ndp_frame
			
			When set, the received frame was an NDP frame, and thus
			there will be no MPDU data.
			
			<legal all>

phy_err
			
			When set, a PHY error was received before MAC received
			any data, and thus there will be no MPDU data.
			
			<legal all>

phy_err_during_mpdu_header
			
			When set, a PHY error was received before MAC received
			the complete MPDU header which was needed for proper
			decoding
			
			<legal all>

protocol_version_err
			
			Set when RXPCU detected a version error in the Frame
			control field
			
			<legal all>

ast_based_lookup_valid
			
			When set, AST based lookup for this frame has found a
			valid result.
			
			
			
			Note that for NDP frame this will never be set
			
			<legal all>

reserved_9a
			
			<legal 0>

phy_ppdu_id
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>

ast_index
			
			This field indicates the index of the AST entry
			corresponding to this MPDU. It is provided by the GSE module
			instantiated in RXPCU.
			
			A value of 0xFFFF indicates an invalid AST index,
			meaning that No AST entry was found or NO AST search was
			performed
			
			
			
			In case of ndp or phy_err, this field will be set to
			0xFFFF
			
			<legal all>

sw_peer_id
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			This field indicates a unique peer identifier. It is set
			equal to field 'sw_peer_id' from the AST entry
			
			
			
			<legal all>

mpdu_frame_control_valid
			
			When set, the field Mpdu_Frame_control_field has valid
			information
			
			
			
			
			<legal all>

mpdu_duration_valid
			
			When set, the field Mpdu_duration_field has valid
			information
			
			
			
			
			<legal all>

mac_addr_ad1_valid
			
			When set, the fields mac_addr_ad1_..... have valid
			information
			
			
			
			
			<legal all>

mac_addr_ad2_valid
			
			When set, the fields mac_addr_ad2_..... have valid
			information
			
			
			
			
			
			
			
			<legal all>

mac_addr_ad3_valid
			
			When set, the fields mac_addr_ad3_..... have valid
			information
			
			
			
			
			
			
			
			<legal all>

mac_addr_ad4_valid
			
			When set, the fields mac_addr_ad4_..... have valid
			information
			
			
			
			
			
			
			
			<legal all>

mpdu_sequence_control_valid
			
			When set, the fields mpdu_sequence_control_field and
			mpdu_sequence_number have valid information as well as field
			
			
			
			For MPDUs without a sequence control field, this field
			will not be set.
			
			
			
			
			<legal all>

mpdu_qos_control_valid
			
			When set, the field mpdu_qos_control_field has valid
			information
			
			
			
			For MPDUs without a QoS control field, this field will
			not be set.
			
			
			
			
			<legal all>

mpdu_ht_control_valid
			
			When set, the field mpdu_HT_control_field has valid
			information
			
			
			
			For MPDUs without a HT control field, this field will
			not be set.
			
			
			
			
			<legal all>

frame_encryption_info_valid
			
			When set, the encryption related info fields, like IV
			and PN are valid
			
			
			
			For MPDUs that are not encrypted, this will not be set.
			
			
			
			
			<legal all>

mpdu_fragment_number
			
			Field only valid when Mpdu_sequence_control_valid is set
			AND Fragment_flag is set 
			
			
			
			The fragment number from the 802.11 header
			
			
			
			<legal all>

more_fragment_flag
			
			The More Fragment bit setting from the MPDU header of
			the received frame
			
			
			
			<legal all>

reserved_11a
			
			<legal 0>

fr_ds
			
			Field only valid when Mpdu_frame_control_valid is set
			
			
			
			Set if the from DS bit is set in the frame control.
			
			<legal all>

to_ds
			
			Field only valid when Mpdu_frame_control_valid is set 
			
			
			
			Set if the to DS bit is set in the frame control.
			
			<legal all>

encrypted
			
			Field only valid when Mpdu_frame_control_valid is set.
			
			
			
			Protected bit from the frame control.  
			
			<legal all>

mpdu_retry
			
			Field only valid when Mpdu_frame_control_valid is set.
			
			
			
			Retry bit from the frame control.  Only valid when
			first_msdu is set.
			
			<legal all>

mpdu_sequence_number
			
			Field only valid when Mpdu_sequence_control_valid is
			set.
			
			
			
			The sequence number from the 802.11 header.
			
			<legal all>

key_id_octet
			
			
			
			
			The key ID octet from the IV.
			
			
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			<legal all>

new_peer_entry
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Set if new RX_PEER_ENTRY TLV follows. If clear,
			RX_PEER_ENTRY doesn't follow so RX DECRYPTION module either
			uses old peer entry or not decrypt. 
			
			<legal all>

decrypt_needed
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Set if decryption is needed. 
			
			
			
			Note:
			
			When RXPCU sets bit 'ast_index_not_found' and/or
			ast_index_timeout', RXPCU will also ensure that this bit is
			NOT set
			
			CRYPTO for that reason only needs to evaluate this bit
			and non of the other ones.
			
			<legal all>

decap_type
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Used by the OLE during decapsulation.
			
			
			
			Indicates the decapsulation that HW will perform:
			
			
			
			<enum 0 RAW> No encapsulation
			
			<enum 1 Native_WiFi>
			
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses
			SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			
			
			<legal all>

rx_insert_vlan_c_tag_padding
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Insert 4 byte of all zeros as VLAN tag if the rx payload
			does not have VLAN. Used during decapsulation. 
			
			<legal all>

rx_insert_vlan_s_tag_padding
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Insert 4 byte of all zeros as double VLAN tag if the rx
			payload does not have VLAN. Used during 
			
			<legal all>

strip_vlan_c_tag_decap
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Strip the VLAN during decapsulation.  Used by the OLE.
			
			<legal all>

strip_vlan_s_tag_decap
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Strip the double VLAN during decapsulation.  Used by
			the OLE.
			
			<legal all>

pre_delim_count
			
			The number of delimiters before this MPDU.  
			
			
			
			Note that this number is cleared at PPDU start.
			
			
			
			If this MPDU is the first received MPDU in the PPDU and
			this MPDU gets filtered-in, this field will indicate the
			number of delimiters located after the last MPDU in the
			previous PPDU.
			
			
			
			If this MPDU is located after the first received MPDU in
			an PPDU, this field will indicate the number of delimiters
			located between the previous MPDU and this MPDU.
			
			
			
			In case of ndp or phy_err, this field will indicate the
			number of delimiters located after the last MPDU in the
			previous PPDU.
			
			<legal all>

ampdu_flag
			
			When set, received frame was part of an A-MPDU.
			
			
			
			
			<legal all>

bar_frame
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			When set, received frame is a BAR frame
			
			<legal all>

raw_mpdu
			
			Consumer: SW
			
			Producer: RXOLE
			
			
			
			RXPCU sets this field to 0 and RXOLE overwrites it.
			
			
			
			Set to 1 by RXOLE when it has not performed any 802.11
			to Ethernet/Natvie WiFi header conversion on this MPDU.
			
			<legal all>

reserved_12
			
			<legal 0>

mpdu_length
			
			In case of ndp or phy_err this field will be set to 0
			
			
			
			MPDU length before decapsulation.
			
			<legal all>

first_mpdu
			
			See definition in RX attention descriptor
			
			
			
			In case of ndp or phy_err, this field will be set. Note
			however that there will not actually be any data contents in
			the MPDU.
			
			<legal all>

mcast_bcast
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

ast_index_not_found
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

ast_index_timeout
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

power_mgmt
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

non_qos
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 1
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

null_data
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

mgmt_type
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

ctrl_type
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

more_data
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

eosp
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

fragment_flag
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

order
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			
			
			<legal all>

u_apsd_trigger
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

encrypt_required
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

directed
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>

amsdu_present
			
			Field only valid when Mpdu_qos_control_valid is set
			
			
			
			The 'amsdu_present' bit within the QoS control field of
			the MPDU
			
			<legal all>

reserved_13
			
			<legal 0>

mpdu_frame_control_field
			
			Field only valid when Mpdu_frame_control_valid is set
			
			
			
			The frame control field of this received MPDU.
			
			
			
			Field only valid when Ndp_frame and phy_err are NOT set
			
			
			
			Bytes 0 + 1 of the received MPDU
			
			<legal all>

mpdu_duration_field
			
			Field only valid when Mpdu_duration_valid is set
			
			
			
			The duration field of this received MPDU.
			
			<legal all>

mac_addr_ad1_31_0
			
			Field only valid when mac_addr_ad1_valid is set
			
			
			
			The Least Significant 4 bytes of the Received Frames MAC
			Address AD1
			
			<legal all>

mac_addr_ad1_47_32
			
			Field only valid when mac_addr_ad1_valid is set
			
			
			
			The 2 most significant bytes of the Received Frames MAC
			Address AD1
			
			<legal all>

mac_addr_ad2_15_0
			
			Field only valid when mac_addr_ad2_valid is set
			
			
			
			The Least Significant 2 bytes of the Received Frames MAC
			Address AD2
			
			<legal all>

mac_addr_ad2_47_16
			
			Field only valid when mac_addr_ad2_valid is set
			
			
			
			The 4 most significant bytes of the Received Frames MAC
			Address AD2
			
			<legal all>

mac_addr_ad3_31_0
			
			Field only valid when mac_addr_ad3_valid is set
			
			
			
			The Least Significant 4 bytes of the Received Frames MAC
			Address AD3
			
			<legal all>

mac_addr_ad3_47_32
			
			Field only valid when mac_addr_ad3_valid is set
			
			
			
			The 2 most significant bytes of the Received Frames MAC
			Address AD3
			
			<legal all>

mpdu_sequence_control_field
			
			
			
			
			The sequence control field of the MPDU
			
			<legal all>

mac_addr_ad4_31_0
			
			Field only valid when mac_addr_ad4_valid is set
			
			
			
			The Least Significant 4 bytes of the Received Frames MAC
			Address AD4
			
			<legal all>

mac_addr_ad4_47_32
			
			Field only valid when mac_addr_ad4_valid is set
			
			
			
			The 2 most significant bytes of the Received Frames MAC
			Address AD4
			
			<legal all>

mpdu_qos_control_field
			
			Field only valid when mpdu_qos_control_valid is set
			
			
			
			The sequence control field of the MPDU
			
			<legal all>

mpdu_ht_control_field
			
			Field only valid when mpdu_qos_control_valid is set
			
			
			
			The HT control field of the MPDU
			
			<legal all>
*/


 /* EXTERNAL REFERENCE : struct rxpt_classify_info rxpt_classify_info_details */ 


/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION
			
			The ID of the REO exit ring where the MSDU frame shall
			push after (MPDU level) reordering has finished.
			
			
			
			<enum 0 reo_destination_tcl> Reo will push the frame
			into the REO2TCL ring
			
			<enum 1 reo_destination_sw1> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 2 reo_destination_sw2> Reo will push the frame
			into the REO2SW2 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW3 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW4 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_sw5> Reo will push the frame
			into the REO2SW5 ring (REO remaps this in chips without
			REO2SW5 ring, e.g. Pine)
			
			<enum 8 reo_destination_sw6> Reo will push the frame
			into the REO2SW6 ring (REO remaps this in chips without
			REO2SW6 ring, e.g. Pine)
			
			<enum 9 reo_destination_9> REO remaps this <enum 10
			reo_destination_10> REO remaps this 
			
			<enum 11 reo_destination_11> REO remaps this 
			
			<enum 12 reo_destination_12> REO remaps this <enum 13
			reo_destination_13> REO remaps this 
			
			<enum 14 reo_destination_14> REO remaps this 
			
			<enum 15 reo_destination_15> REO remaps this 
			
			<enum 16 reo_destination_16> REO remaps this 
			
			<enum 17 reo_destination_17> REO remaps this 
			
			<enum 18 reo_destination_18> REO remaps this 
			
			<enum 19 reo_destination_19> REO remaps this 
			
			<enum 20 reo_destination_20> REO remaps this 
			
			<enum 21 reo_destination_21> REO remaps this 
			
			<enum 22 reo_destination_22> REO remaps this 
			
			<enum 23 reo_destination_23> REO remaps this 
			
			<enum 24 reo_destination_24> REO remaps this 
			
			<enum 25 reo_destination_25> REO remaps this 
			
			<enum 26 reo_destination_26> REO remaps this 
			
			<enum 27 reo_destination_27> REO remaps this 
			
			<enum 28 reo_destination_28> REO remaps this 
			
			<enum 29 reo_destination_29> REO remaps this 
			
			<enum 30 reo_destination_30> REO remaps this 
			
			<enum 31 reo_destination_31> REO remaps this 
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB 0
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK 0x0000001f

/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_LMAC_PEER_ID_MSB
			
			If use_flow_id_toeplitz_clfy is set and lmac_peer_id_'sb
			is 2'b00, Rx OLE uses a REO desination indicati'n of {1'b1,
			hash[3:0]} using the chosen Toeplitz hash from Common Parser
			if flow search fails.
			
			If use_flow_id_toeplitz_clfy is set and lmac_peer_id_msb
			's not 2'b00, Rx OLE uses a REO desination indication of
			{lmac_peer_id_msb, hash[2:0]} using the chosen Toeplitz hash
			from Common Parser if flow search fails.
			
			This LMAC/peer-based routing is not supported in
			Hastings80 and HastingsPrime.
			
			<legal all>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_LMAC_PEER_ID_MSB_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_LMAC_PEER_ID_MSB_LSB 5
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_LMAC_PEER_ID_MSB_MASK 0x00000060

/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_USE_FLOW_ID_TOEPLITZ_CLFY
			
			Indication to Rx OLE to enable REO destination routing
			based on the chosen Toeplitz hash from Common Parser, in
			case flow search fails
			
			<legal all>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_USE_FLOW_ID_TOEPLITZ_CLFY_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_USE_FLOW_ID_TOEPLITZ_CLFY_LSB 7
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_USE_FLOW_ID_TOEPLITZ_CLFY_MASK 0x00000080

/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_UCAST_DATA
			
			Filter pass Unicast data frame (matching
			rxpcu_filter_pass and sw_frame_group_Unicast_data) routing
			selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_UCAST_DATA_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_UCAST_DATA_LSB 8
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_UCAST_DATA_MASK 0x00000100

/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_MCAST_DATA
			
			Filter pass Multicast data frame (matching
			rxpcu_filter_pass and sw_frame_group_Multicast_data) routing
			selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_MCAST_DATA_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_MCAST_DATA_LSB 9
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_MCAST_DATA_MASK 0x00000200

/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_1000
			
			Filter pass BAR frame (matching rxpcu_filter_pass and
			sw_frame_group_ctrl_1000) routing selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_1000_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_1000_LSB 10
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_1000_MASK 0x00000400

/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_SOURCE_RING_SELECTION
			
			Field only valid when for the received frame type the
			corresponding pkt_selection_fp_... bit is set
			
			
			
			<enum 0 wbm2rxdma_buf_source_ring> The data buffer for
			
			<enum 1 fw2rxdma_buf_source_ring> The data buffer for
			this frame shall be sourced by fw2rxdma buffer source ring.
			
			<enum 2 sw2rxdma_buf_source_ring> The data buffer for
			this frame shall be sourced by sw2rxdma buffer source ring.
			
			<enum 3 no_buffer_ring> The frame shall not be written
			to any data buffer.
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_SOURCE_RING_SELECTION_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_SOURCE_RING_SELECTION_LSB 11
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_SOURCE_RING_SELECTION_MASK 0x00001800

/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_DESTINATION_RING_SELECTION
			
			Field only valid when for the received frame type the
			corresponding pkt_selection_fp_... bit is set
			
			
			
			<enum 0  rxdma_release_ring> RXDMA0 shall push the frame
			to the Release ring. Effectively this means the frame needs
			to be dropped.
			
			<enum 1  rxdma2fw_ring> RXDMA0 shall push the frame to
			the FW ring.
			
			<enum 2  rxdma2sw_ring> RXDMA0 shall push the frame to
			the SW ring.
			
			<enum 3  rxdma2reo_ring> RXDMA0 shall push the frame to
			the REO entrance ring.
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_DESTINATION_RING_SELECTION_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_DESTINATION_RING_SELECTION_LSB 13
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_DESTINATION_RING_SELECTION_MASK 0x00006000

/* Description		RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RESERVED_0B
			
			<legal 0>
*/
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RESERVED_0B_OFFSET 0x00000000
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RESERVED_0B_LSB    15
#define RX_MPDU_INFO_0_RXPT_CLASSIFY_INFO_DETAILS_RESERVED_0B_MASK   0xffff8000

/* Description		RX_MPDU_INFO_1_RX_REO_QUEUE_DESC_ADDR_31_0
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Address (lower 32 bits) of the REO queue descriptor. 
			
			
			
			If no Peer entry lookup happened for this frame, the
			value wil be set to 0, and the frame shall never be pushed
			to REO entrance ring.
			
			<legal all>
*/
#define RX_MPDU_INFO_1_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET            0x00000004
#define RX_MPDU_INFO_1_RX_REO_QUEUE_DESC_ADDR_31_0_LSB               0
#define RX_MPDU_INFO_1_RX_REO_QUEUE_DESC_ADDR_31_0_MASK              0xffffffff

/* Description		RX_MPDU_INFO_2_RX_REO_QUEUE_DESC_ADDR_39_32
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Address (upper 8 bits) of the REO queue descriptor. 
			
			
			
			If no Peer entry lookup happened for this frame, the
			value wil be set to 0, and the frame shall never be pushed
			to REO entrance ring.
			
			<legal all>
*/
#define RX_MPDU_INFO_2_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET           0x00000008
#define RX_MPDU_INFO_2_RX_REO_QUEUE_DESC_ADDR_39_32_LSB              0
#define RX_MPDU_INFO_2_RX_REO_QUEUE_DESC_ADDR_39_32_MASK             0x000000ff

/* Description		RX_MPDU_INFO_2_RECEIVE_QUEUE_NUMBER
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Indicates the MPDU queue ID to which this MPDU link
			descriptor belongs
			
			Used for tracking and debugging
			
			<legal all>
*/
#define RX_MPDU_INFO_2_RECEIVE_QUEUE_NUMBER_OFFSET                   0x00000008
#define RX_MPDU_INFO_2_RECEIVE_QUEUE_NUMBER_LSB                      8
#define RX_MPDU_INFO_2_RECEIVE_QUEUE_NUMBER_MASK                     0x00ffff00

/* Description		RX_MPDU_INFO_2_PRE_DELIM_ERR_WARNING
			
			Indicates that a delimiter FCS error was found in
			between the Previous MPDU and this MPDU.
			
			
			
			Note that this is just a warning, and does not mean that
			this MPDU is corrupted in any way. If it is, there will be
			other errors indicated such as FCS or decrypt errors
			
			
			
			In case of ndp or phy_err, this field will indicate at
			least one of delimiters located after the last MPDU in the
			previous PPDU has been corrupted.
*/
#define RX_MPDU_INFO_2_PRE_DELIM_ERR_WARNING_OFFSET                  0x00000008
#define RX_MPDU_INFO_2_PRE_DELIM_ERR_WARNING_LSB                     24
#define RX_MPDU_INFO_2_PRE_DELIM_ERR_WARNING_MASK                    0x01000000

/* Description		RX_MPDU_INFO_2_FIRST_DELIM_ERR
			
			Indicates that the first delimiter had a FCS failure. 
			Only valid when first_mpdu and first_msdu are set.
			
			
			
*/
#define RX_MPDU_INFO_2_FIRST_DELIM_ERR_OFFSET                        0x00000008
#define RX_MPDU_INFO_2_FIRST_DELIM_ERR_LSB                           25
#define RX_MPDU_INFO_2_FIRST_DELIM_ERR_MASK                          0x02000000

/* Description		RX_MPDU_INFO_2_RESERVED_2A
			
			<legal 0>
*/
#define RX_MPDU_INFO_2_RESERVED_2A_OFFSET                            0x00000008
#define RX_MPDU_INFO_2_RESERVED_2A_LSB                               26
#define RX_MPDU_INFO_2_RESERVED_2A_MASK                              0xfc000000

/* Description		RX_MPDU_INFO_3_PN_31_0
			
			
			
			
			
			WEP: IV = {key_id_octet, pn2, pn1, pn0}.  Only pn[23:0]
			is valid.
			
			TKIP: IV = {pn5, pn4, pn3, pn2, key_id_octet, pn0,
			WEPSeed[1], pn1}.  Only pn[47:0] is valid.
			
			AES-CCM: IV = {pn5, pn4, pn3, pn2, key_id_octet, 0x0,
			pn1, pn0}.  Only pn[47:0] is valid.
			
			WAPI: IV = {key_id_octet, 0x0, pn15, pn14, pn13, pn12,
			pn11, pn10, pn9, pn8, pn7, pn6, pn5, pn4, pn3, pn2, pn1,
			pn0}.  pn[127:0] are valid.
			
			
			
*/
#define RX_MPDU_INFO_3_PN_31_0_OFFSET                                0x0000000c
#define RX_MPDU_INFO_3_PN_31_0_LSB                                   0
#define RX_MPDU_INFO_3_PN_31_0_MASK                                  0xffffffff

/* Description		RX_MPDU_INFO_4_PN_63_32
			
			
			
			
			Bits [63:32] of the PN number.   See description for
			pn_31_0.
			
			
			
*/
#define RX_MPDU_INFO_4_PN_63_32_OFFSET                               0x00000010
#define RX_MPDU_INFO_4_PN_63_32_LSB                                  0
#define RX_MPDU_INFO_4_PN_63_32_MASK                                 0xffffffff

/* Description		RX_MPDU_INFO_5_PN_95_64
			
			
			
			
			Bits [95:64] of the PN number.  See description for
			pn_31_0.
			
			
			
*/
#define RX_MPDU_INFO_5_PN_95_64_OFFSET                               0x00000014
#define RX_MPDU_INFO_5_PN_95_64_LSB                                  0
#define RX_MPDU_INFO_5_PN_95_64_MASK                                 0xffffffff

/* Description		RX_MPDU_INFO_6_PN_127_96
			
			
			
			
			Bits [127:96] of the PN number.  See description for
			pn_31_0.
			
			
			
*/
#define RX_MPDU_INFO_6_PN_127_96_OFFSET                              0x00000018
#define RX_MPDU_INFO_6_PN_127_96_LSB                                 0
#define RX_MPDU_INFO_6_PN_127_96_MASK                                0xffffffff

/* Description		RX_MPDU_INFO_7_EPD_EN
			
			Field only valid when AST_based_lookup_valid == 1.
			
			
			
			
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			If set to one use EPD instead of LPD
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_7_EPD_EN_OFFSET                                 0x0000001c
#define RX_MPDU_INFO_7_EPD_EN_LSB                                    0
#define RX_MPDU_INFO_7_EPD_EN_MASK                                   0x00000001

/* Description		RX_MPDU_INFO_7_ALL_FRAMES_SHALL_BE_ENCRYPTED
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			When set, all frames (data only ?) shall be encrypted.
			If not, RX CRYPTO shall set an error flag.
			
			<legal all>
*/
#define RX_MPDU_INFO_7_ALL_FRAMES_SHALL_BE_ENCRYPTED_OFFSET          0x0000001c
#define RX_MPDU_INFO_7_ALL_FRAMES_SHALL_BE_ENCRYPTED_LSB             1
#define RX_MPDU_INFO_7_ALL_FRAMES_SHALL_BE_ENCRYPTED_MASK            0x00000002

/* Description		RX_MPDU_INFO_7_ENCRYPT_TYPE
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Indicates type of decrypt cipher used (as defined in the
			peer entry)
			
			
			
			<enum 0 wep_40> WEP 40-bit
			
			<enum 1 wep_104> WEP 104-bit
			
			<enum 2 tkip_no_mic> TKIP without MIC
			
			<enum 3 wep_128> WEP 128-bit
			
			<enum 4 tkip_with_mic> TKIP with MIC
			
			<enum 5 wapi> WAPI
			
			<enum 6 aes_ccmp_128> AES CCMP 128
			
			<enum 7 no_cipher> No crypto
			
			<enum 8 aes_ccmp_256> AES CCMP 256
			
			<enum 9 aes_gcmp_128> AES CCMP 128
			
			<enum 10 aes_gcmp_256> AES CCMP 256
			
			<enum 11 wapi_gcm_sm4> WAPI GCM SM4
			
			
			
			<enum 12 wep_varied_width> WEP encryption. As for WEP
			per keyid the key bit width can vary, the key bit width for
			this MPDU will be indicated in field
			wep_key_width_for_variable key
			
			<legal 0-12>
*/
#define RX_MPDU_INFO_7_ENCRYPT_TYPE_OFFSET                           0x0000001c
#define RX_MPDU_INFO_7_ENCRYPT_TYPE_LSB                              2
#define RX_MPDU_INFO_7_ENCRYPT_TYPE_MASK                             0x0000003c

/* Description		RX_MPDU_INFO_7_WEP_KEY_WIDTH_FOR_VARIABLE_KEY
			
			Field only valid when key_type is set to
			wep_varied_width. 
			
			
			
			This field indicates the size of the wep key for this
			MPDU.
			
			 
			
			<enum 0 wep_varied_width_40> WEP 40-bit
			
			<enum 1 wep_varied_width_104> WEP 104-bit
			
			<enum 2 wep_varied_width_128> WEP 128-bit
			
			
			
			<legal 0-2>
*/
#define RX_MPDU_INFO_7_WEP_KEY_WIDTH_FOR_VARIABLE_KEY_OFFSET         0x0000001c
#define RX_MPDU_INFO_7_WEP_KEY_WIDTH_FOR_VARIABLE_KEY_LSB            6
#define RX_MPDU_INFO_7_WEP_KEY_WIDTH_FOR_VARIABLE_KEY_MASK           0x000000c0

/* Description		RX_MPDU_INFO_7_MESH_STA
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			When set, this is a Mesh (11s) STA.
			
			
			
			The interpretation of the A-MSDU 'Length' field in the
			MPDU (if any) is decided by the e-numerations below.
			
			
			
			<enum 0 MESH_DISABLE>
			
			<enum 1 MESH_Q2Q> A-MSDU 'Length' is big endian and
			includes the length of Mesh Control.
			
			<enum 2 MESH_11S_BE> A-MSDU 'Length' is big endian and
			excludes the length of Mesh Control.
			
			<enum 3 MESH_11S_LE> A-MSDU 'Length' is little endian
			and excludes the length of Mesh Control. This is
			802.11s-compliant.
			
			<legal all>
*/
#define RX_MPDU_INFO_7_MESH_STA_OFFSET                               0x0000001c
#define RX_MPDU_INFO_7_MESH_STA_LSB                                  8
#define RX_MPDU_INFO_7_MESH_STA_MASK                                 0x00000300

/* Description		RX_MPDU_INFO_7_BSSID_HIT
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			When set, the BSSID of the incoming frame matched one of
			the 8 BSSID register values
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_7_BSSID_HIT_OFFSET                              0x0000001c
#define RX_MPDU_INFO_7_BSSID_HIT_LSB                                 10
#define RX_MPDU_INFO_7_BSSID_HIT_MASK                                0x00000400

/* Description		RX_MPDU_INFO_7_BSSID_NUMBER
			
			Field only valid when bssid_hit is set.
			
			
			
			This number indicates which one out of the 8 BSSID
			register values matched the incoming frame
			
			<legal all>
*/
#define RX_MPDU_INFO_7_BSSID_NUMBER_OFFSET                           0x0000001c
#define RX_MPDU_INFO_7_BSSID_NUMBER_LSB                              11
#define RX_MPDU_INFO_7_BSSID_NUMBER_MASK                             0x00007800

/* Description		RX_MPDU_INFO_7_TID
			
			Field only valid when mpdu_qos_control_valid is set
			
			
			
			The TID field in the QoS control field
			
			<legal all>
*/
#define RX_MPDU_INFO_7_TID_OFFSET                                    0x0000001c
#define RX_MPDU_INFO_7_TID_LSB                                       15
#define RX_MPDU_INFO_7_TID_MASK                                      0x00078000

/* Description		RX_MPDU_INFO_7_RESERVED_7A
			
			<legal 0>
*/
#define RX_MPDU_INFO_7_RESERVED_7A_OFFSET                            0x0000001c
#define RX_MPDU_INFO_7_RESERVED_7A_LSB                               19
#define RX_MPDU_INFO_7_RESERVED_7A_MASK                              0xfff80000

/* Description		RX_MPDU_INFO_8_PEER_META_DATA
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Meta data that SW has programmed in the Peer table entry
			of the transmitting STA.
			
			<legal all>
*/
#define RX_MPDU_INFO_8_PEER_META_DATA_OFFSET                         0x00000020
#define RX_MPDU_INFO_8_PEER_META_DATA_LSB                            0
#define RX_MPDU_INFO_8_PEER_META_DATA_MASK                           0xffffffff

/* Description		RX_MPDU_INFO_9_RXPCU_MPDU_FILTER_IN_CATEGORY
			
			Field indicates what the reason was that this MPDU frame
			was allowed to come into the receive path by RXPCU
			
			<enum 0 rxpcu_filter_pass> This MPDU passed the normal
			frame filter programming of rxpcu
			
			<enum 1 rxpcu_monitor_client> This MPDU did NOT pass the
			regular frame filter and would have been dropped, were it
			not for the frame fitting into the 'monitor_client'
			category.
			
			<enum 2 rxpcu_monitor_other> This MPDU did NOT pass the
			regular frame filter and also did not pass the
			rxpcu_monitor_client filter. It would have been dropped
			accept that it did pass the 'monitor_other' category.
			
			
			
			Note: for ndp frame, if it was expected because the
			preceding NDPA was filter_pass, the setting 
			rxpcu_filter_pass will be used. This setting will also be
			used for every ndp frame in case Promiscuous mode is
			enabled.
			
			
			
			In case promiscuous is not enabled, and an NDP is not
			preceded by a NPDA filter pass frame, the only other setting
			that could appear here for the NDP is rxpcu_monitor_other. 
			
			(rxpcu has a configuration bit specifically for this
			scenario)
			
			
			
			Note: for 
			
			<legal 0-2>
*/
#define RX_MPDU_INFO_9_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET          0x00000024
#define RX_MPDU_INFO_9_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB             0
#define RX_MPDU_INFO_9_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK            0x00000003

/* Description		RX_MPDU_INFO_9_SW_FRAME_GROUP_ID
			
			SW processes frames based on certain classifications.
			This field indicates to what sw classification this MPDU is
			mapped.
			
			The classification is given in priority order
			
			
			
			<enum 0 sw_frame_group_NDP_frame> Note: The
			corresponding Rxpcu_Mpdu_filter_in_category can be
			rxpcu_filter_pass or rxpcu_monitor_other
			
			
			
			<enum 1 sw_frame_group_Multicast_data> 
			
			<enum 2 sw_frame_group_Unicast_data> 
			
			<enum 3 sw_frame_group_Null_data > This includes mpdus
			of type Data Null as well as QoS Data Null
			
			
			
			<enum 4 sw_frame_group_mgmt_0000 > 
			
			<enum 5 sw_frame_group_mgmt_0001 > 
			
			<enum 6 sw_frame_group_mgmt_0010 > 
			
			<enum 7 sw_frame_group_mgmt_0011 > 
			
			<enum 8 sw_frame_group_mgmt_0100 > 
			
			<enum 9 sw_frame_group_mgmt_0101 > 
			
			<enum 10 sw_frame_group_mgmt_0110 > 
			
			<enum 11 sw_frame_group_mgmt_0111 > 
			
			<enum 12 sw_frame_group_mgmt_1000 > 
			
			<enum 13 sw_frame_group_mgmt_1001 > 
			
			<enum 14 sw_frame_group_mgmt_1010 > 
			
			<enum 15 sw_frame_group_mgmt_1011 > 
			
			<enum 16 sw_frame_group_mgmt_1100 > 
			
			<enum 17 sw_frame_group_mgmt_1101 > 
			
			<enum 18 sw_frame_group_mgmt_1110 > 
			
			<enum 19 sw_frame_group_mgmt_1111 > 
			
			
			
			<enum 20 sw_frame_group_ctrl_0000 > 
			
			<enum 21 sw_frame_group_ctrl_0001 > 
			
			<enum 22 sw_frame_group_ctrl_0010 > 
			
			<enum 23 sw_frame_group_ctrl_0011 > 
			
			<enum 24 sw_frame_group_ctrl_0100 > 
			
			<enum 25 sw_frame_group_ctrl_0101 > 
			
			<enum 26 sw_frame_group_ctrl_0110 > 
			
			<enum 27 sw_frame_group_ctrl_0111 > 
			
			<enum 28 sw_frame_group_ctrl_1000 > 
			
			<enum 29 sw_frame_group_ctrl_1001 > 
			
			<enum 30 sw_frame_group_ctrl_1010 > 
			
			<enum 31 sw_frame_group_ctrl_1011 > 
			
			<enum 32 sw_frame_group_ctrl_1100 > 
			
			<enum 33 sw_frame_group_ctrl_1101 > 
			
			<enum 34 sw_frame_group_ctrl_1110 > 
			
			<enum 35 sw_frame_group_ctrl_1111 > 
			
			
			
			<enum 36 sw_frame_group_unsupported> This covers type 3
			and protocol version != 0 
			
			Note: The corresponding Rxpcu_Mpdu_filter_in_category
			can only be rxpcu_monitor_other
			
			
			
			
			Note: The corresponding Rxpcu_Mpdu_filter_in_category
			can be rxpcu_filter_pass
			
			
			
			<legal 0-37>
*/
#define RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_OFFSET                      0x00000024
#define RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_LSB                         2
#define RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_MASK                        0x000001fc

/* Description		RX_MPDU_INFO_9_NDP_FRAME
			
			When set, the received frame was an NDP frame, and thus
			there will be no MPDU data.
			
			<legal all>
*/
#define RX_MPDU_INFO_9_NDP_FRAME_OFFSET                              0x00000024
#define RX_MPDU_INFO_9_NDP_FRAME_LSB                                 9
#define RX_MPDU_INFO_9_NDP_FRAME_MASK                                0x00000200

/* Description		RX_MPDU_INFO_9_PHY_ERR
			
			When set, a PHY error was received before MAC received
			any data, and thus there will be no MPDU data.
			
			<legal all>
*/
#define RX_MPDU_INFO_9_PHY_ERR_OFFSET                                0x00000024
#define RX_MPDU_INFO_9_PHY_ERR_LSB                                   10
#define RX_MPDU_INFO_9_PHY_ERR_MASK                                  0x00000400

/* Description		RX_MPDU_INFO_9_PHY_ERR_DURING_MPDU_HEADER
			
			When set, a PHY error was received before MAC received
			the complete MPDU header which was needed for proper
			decoding
			
			<legal all>
*/
#define RX_MPDU_INFO_9_PHY_ERR_DURING_MPDU_HEADER_OFFSET             0x00000024
#define RX_MPDU_INFO_9_PHY_ERR_DURING_MPDU_HEADER_LSB                11
#define RX_MPDU_INFO_9_PHY_ERR_DURING_MPDU_HEADER_MASK               0x00000800

/* Description		RX_MPDU_INFO_9_PROTOCOL_VERSION_ERR
			
			Set when RXPCU detected a version error in the Frame
			control field
			
			<legal all>
*/
#define RX_MPDU_INFO_9_PROTOCOL_VERSION_ERR_OFFSET                   0x00000024
#define RX_MPDU_INFO_9_PROTOCOL_VERSION_ERR_LSB                      12
#define RX_MPDU_INFO_9_PROTOCOL_VERSION_ERR_MASK                     0x00001000

/* Description		RX_MPDU_INFO_9_AST_BASED_LOOKUP_VALID
			
			When set, AST based lookup for this frame has found a
			valid result.
			
			
			
			Note that for NDP frame this will never be set
			
			<legal all>
*/
#define RX_MPDU_INFO_9_AST_BASED_LOOKUP_VALID_OFFSET                 0x00000024
#define RX_MPDU_INFO_9_AST_BASED_LOOKUP_VALID_LSB                    13
#define RX_MPDU_INFO_9_AST_BASED_LOOKUP_VALID_MASK                   0x00002000

/* Description		RX_MPDU_INFO_9_RESERVED_9A
			
			<legal 0>
*/
#define RX_MPDU_INFO_9_RESERVED_9A_OFFSET                            0x00000024
#define RX_MPDU_INFO_9_RESERVED_9A_LSB                               14
#define RX_MPDU_INFO_9_RESERVED_9A_MASK                              0x0000c000

/* Description		RX_MPDU_INFO_9_PHY_PPDU_ID
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>
*/
#define RX_MPDU_INFO_9_PHY_PPDU_ID_OFFSET                            0x00000024
#define RX_MPDU_INFO_9_PHY_PPDU_ID_LSB                               16
#define RX_MPDU_INFO_9_PHY_PPDU_ID_MASK                              0xffff0000

/* Description		RX_MPDU_INFO_10_AST_INDEX
			
			This field indicates the index of the AST entry
			corresponding to this MPDU. It is provided by the GSE module
			instantiated in RXPCU.
			
			A value of 0xFFFF indicates an invalid AST index,
			meaning that No AST entry was found or NO AST search was
			performed
			
			
			
			In case of ndp or phy_err, this field will be set to
			0xFFFF
			
			<legal all>
*/
#define RX_MPDU_INFO_10_AST_INDEX_OFFSET                             0x00000028
#define RX_MPDU_INFO_10_AST_INDEX_LSB                                0
#define RX_MPDU_INFO_10_AST_INDEX_MASK                               0x0000ffff

/* Description		RX_MPDU_INFO_10_SW_PEER_ID
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			This field indicates a unique peer identifier. It is set
			equal to field 'sw_peer_id' from the AST entry
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_10_SW_PEER_ID_OFFSET                            0x00000028
#define RX_MPDU_INFO_10_SW_PEER_ID_LSB                               16
#define RX_MPDU_INFO_10_SW_PEER_ID_MASK                              0xffff0000

/* Description		RX_MPDU_INFO_11_MPDU_FRAME_CONTROL_VALID
			
			When set, the field Mpdu_Frame_control_field has valid
			information
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MPDU_FRAME_CONTROL_VALID_OFFSET              0x0000002c
#define RX_MPDU_INFO_11_MPDU_FRAME_CONTROL_VALID_LSB                 0
#define RX_MPDU_INFO_11_MPDU_FRAME_CONTROL_VALID_MASK                0x00000001

/* Description		RX_MPDU_INFO_11_MPDU_DURATION_VALID
			
			When set, the field Mpdu_duration_field has valid
			information
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MPDU_DURATION_VALID_OFFSET                   0x0000002c
#define RX_MPDU_INFO_11_MPDU_DURATION_VALID_LSB                      1
#define RX_MPDU_INFO_11_MPDU_DURATION_VALID_MASK                     0x00000002

/* Description		RX_MPDU_INFO_11_MAC_ADDR_AD1_VALID
			
			When set, the fields mac_addr_ad1_..... have valid
			information
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MAC_ADDR_AD1_VALID_OFFSET                    0x0000002c
#define RX_MPDU_INFO_11_MAC_ADDR_AD1_VALID_LSB                       2
#define RX_MPDU_INFO_11_MAC_ADDR_AD1_VALID_MASK                      0x00000004

/* Description		RX_MPDU_INFO_11_MAC_ADDR_AD2_VALID
			
			When set, the fields mac_addr_ad2_..... have valid
			information
			
			
			
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MAC_ADDR_AD2_VALID_OFFSET                    0x0000002c
#define RX_MPDU_INFO_11_MAC_ADDR_AD2_VALID_LSB                       3
#define RX_MPDU_INFO_11_MAC_ADDR_AD2_VALID_MASK                      0x00000008

/* Description		RX_MPDU_INFO_11_MAC_ADDR_AD3_VALID
			
			When set, the fields mac_addr_ad3_..... have valid
			information
			
			
			
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MAC_ADDR_AD3_VALID_OFFSET                    0x0000002c
#define RX_MPDU_INFO_11_MAC_ADDR_AD3_VALID_LSB                       4
#define RX_MPDU_INFO_11_MAC_ADDR_AD3_VALID_MASK                      0x00000010

/* Description		RX_MPDU_INFO_11_MAC_ADDR_AD4_VALID
			
			When set, the fields mac_addr_ad4_..... have valid
			information
			
			
			
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MAC_ADDR_AD4_VALID_OFFSET                    0x0000002c
#define RX_MPDU_INFO_11_MAC_ADDR_AD4_VALID_LSB                       5
#define RX_MPDU_INFO_11_MAC_ADDR_AD4_VALID_MASK                      0x00000020

/* Description		RX_MPDU_INFO_11_MPDU_SEQUENCE_CONTROL_VALID
			
			When set, the fields mpdu_sequence_control_field and
			mpdu_sequence_number have valid information as well as field
			
			
			
			For MPDUs without a sequence control field, this field
			will not be set.
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MPDU_SEQUENCE_CONTROL_VALID_OFFSET           0x0000002c
#define RX_MPDU_INFO_11_MPDU_SEQUENCE_CONTROL_VALID_LSB              6
#define RX_MPDU_INFO_11_MPDU_SEQUENCE_CONTROL_VALID_MASK             0x00000040

/* Description		RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID
			
			When set, the field mpdu_qos_control_field has valid
			information
			
			
			
			For MPDUs without a QoS control field, this field will
			not be set.
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_OFFSET                0x0000002c
#define RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_LSB                   7
#define RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_MASK                  0x00000080

/* Description		RX_MPDU_INFO_11_MPDU_HT_CONTROL_VALID
			
			When set, the field mpdu_HT_control_field has valid
			information
			
			
			
			For MPDUs without a HT control field, this field will
			not be set.
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MPDU_HT_CONTROL_VALID_OFFSET                 0x0000002c
#define RX_MPDU_INFO_11_MPDU_HT_CONTROL_VALID_LSB                    8
#define RX_MPDU_INFO_11_MPDU_HT_CONTROL_VALID_MASK                   0x00000100

/* Description		RX_MPDU_INFO_11_FRAME_ENCRYPTION_INFO_VALID
			
			When set, the encryption related info fields, like IV
			and PN are valid
			
			
			
			For MPDUs that are not encrypted, this will not be set.
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_FRAME_ENCRYPTION_INFO_VALID_OFFSET           0x0000002c
#define RX_MPDU_INFO_11_FRAME_ENCRYPTION_INFO_VALID_LSB              9
#define RX_MPDU_INFO_11_FRAME_ENCRYPTION_INFO_VALID_MASK             0x00000200

/* Description		RX_MPDU_INFO_11_MPDU_FRAGMENT_NUMBER
			
			Field only valid when Mpdu_sequence_control_valid is set
			AND Fragment_flag is set 
			
			
			
			The fragment number from the 802.11 header
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MPDU_FRAGMENT_NUMBER_OFFSET                  0x0000002c
#define RX_MPDU_INFO_11_MPDU_FRAGMENT_NUMBER_LSB                     10
#define RX_MPDU_INFO_11_MPDU_FRAGMENT_NUMBER_MASK                    0x00003c00

/* Description		RX_MPDU_INFO_11_MORE_FRAGMENT_FLAG
			
			The More Fragment bit setting from the MPDU header of
			the received frame
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MORE_FRAGMENT_FLAG_OFFSET                    0x0000002c
#define RX_MPDU_INFO_11_MORE_FRAGMENT_FLAG_LSB                       14
#define RX_MPDU_INFO_11_MORE_FRAGMENT_FLAG_MASK                      0x00004000

/* Description		RX_MPDU_INFO_11_RESERVED_11A
			
			<legal 0>
*/
#define RX_MPDU_INFO_11_RESERVED_11A_OFFSET                          0x0000002c
#define RX_MPDU_INFO_11_RESERVED_11A_LSB                             15
#define RX_MPDU_INFO_11_RESERVED_11A_MASK                            0x00008000

/* Description		RX_MPDU_INFO_11_FR_DS
			
			Field only valid when Mpdu_frame_control_valid is set
			
			
			
			Set if the from DS bit is set in the frame control.
			
			<legal all>
*/
#define RX_MPDU_INFO_11_FR_DS_OFFSET                                 0x0000002c
#define RX_MPDU_INFO_11_FR_DS_LSB                                    16
#define RX_MPDU_INFO_11_FR_DS_MASK                                   0x00010000

/* Description		RX_MPDU_INFO_11_TO_DS
			
			Field only valid when Mpdu_frame_control_valid is set 
			
			
			
			Set if the to DS bit is set in the frame control.
			
			<legal all>
*/
#define RX_MPDU_INFO_11_TO_DS_OFFSET                                 0x0000002c
#define RX_MPDU_INFO_11_TO_DS_LSB                                    17
#define RX_MPDU_INFO_11_TO_DS_MASK                                   0x00020000

/* Description		RX_MPDU_INFO_11_ENCRYPTED
			
			Field only valid when Mpdu_frame_control_valid is set.
			
			
			
			Protected bit from the frame control.  
			
			<legal all>
*/
#define RX_MPDU_INFO_11_ENCRYPTED_OFFSET                             0x0000002c
#define RX_MPDU_INFO_11_ENCRYPTED_LSB                                18
#define RX_MPDU_INFO_11_ENCRYPTED_MASK                               0x00040000

/* Description		RX_MPDU_INFO_11_MPDU_RETRY
			
			Field only valid when Mpdu_frame_control_valid is set.
			
			
			
			Retry bit from the frame control.  Only valid when
			first_msdu is set.
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MPDU_RETRY_OFFSET                            0x0000002c
#define RX_MPDU_INFO_11_MPDU_RETRY_LSB                               19
#define RX_MPDU_INFO_11_MPDU_RETRY_MASK                              0x00080000

/* Description		RX_MPDU_INFO_11_MPDU_SEQUENCE_NUMBER
			
			Field only valid when Mpdu_sequence_control_valid is
			set.
			
			
			
			The sequence number from the 802.11 header.
			
			<legal all>
*/
#define RX_MPDU_INFO_11_MPDU_SEQUENCE_NUMBER_OFFSET                  0x0000002c
#define RX_MPDU_INFO_11_MPDU_SEQUENCE_NUMBER_LSB                     20
#define RX_MPDU_INFO_11_MPDU_SEQUENCE_NUMBER_MASK                    0xfff00000

/* Description		RX_MPDU_INFO_12_KEY_ID_OCTET
			
			
			
			
			The key ID octet from the IV.
			
			
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			<legal all>
*/
#define RX_MPDU_INFO_12_KEY_ID_OCTET_OFFSET                          0x00000030
#define RX_MPDU_INFO_12_KEY_ID_OCTET_LSB                             0
#define RX_MPDU_INFO_12_KEY_ID_OCTET_MASK                            0x000000ff

/* Description		RX_MPDU_INFO_12_NEW_PEER_ENTRY
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Set if new RX_PEER_ENTRY TLV follows. If clear,
			RX_PEER_ENTRY doesn't follow so RX DECRYPTION module either
			uses old peer entry or not decrypt. 
			
			<legal all>
*/
#define RX_MPDU_INFO_12_NEW_PEER_ENTRY_OFFSET                        0x00000030
#define RX_MPDU_INFO_12_NEW_PEER_ENTRY_LSB                           8
#define RX_MPDU_INFO_12_NEW_PEER_ENTRY_MASK                          0x00000100

/* Description		RX_MPDU_INFO_12_DECRYPT_NEEDED
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Set if decryption is needed. 
			
			
			
			Note:
			
			When RXPCU sets bit 'ast_index_not_found' and/or
			ast_index_timeout', RXPCU will also ensure that this bit is
			NOT set
			
			CRYPTO for that reason only needs to evaluate this bit
			and non of the other ones.
			
			<legal all>
*/
#define RX_MPDU_INFO_12_DECRYPT_NEEDED_OFFSET                        0x00000030
#define RX_MPDU_INFO_12_DECRYPT_NEEDED_LSB                           9
#define RX_MPDU_INFO_12_DECRYPT_NEEDED_MASK                          0x00000200

/* Description		RX_MPDU_INFO_12_DECAP_TYPE
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Used by the OLE during decapsulation.
			
			
			
			Indicates the decapsulation that HW will perform:
			
			
			
			<enum 0 RAW> No encapsulation
			
			<enum 1 Native_WiFi>
			
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses
			SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_12_DECAP_TYPE_OFFSET                            0x00000030
#define RX_MPDU_INFO_12_DECAP_TYPE_LSB                               10
#define RX_MPDU_INFO_12_DECAP_TYPE_MASK                              0x00000c00

/* Description		RX_MPDU_INFO_12_RX_INSERT_VLAN_C_TAG_PADDING
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Insert 4 byte of all zeros as VLAN tag if the rx payload
			does not have VLAN. Used during decapsulation. 
			
			<legal all>
*/
#define RX_MPDU_INFO_12_RX_INSERT_VLAN_C_TAG_PADDING_OFFSET          0x00000030
#define RX_MPDU_INFO_12_RX_INSERT_VLAN_C_TAG_PADDING_LSB             12
#define RX_MPDU_INFO_12_RX_INSERT_VLAN_C_TAG_PADDING_MASK            0x00001000

/* Description		RX_MPDU_INFO_12_RX_INSERT_VLAN_S_TAG_PADDING
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Insert 4 byte of all zeros as double VLAN tag if the rx
			payload does not have VLAN. Used during 
			
			<legal all>
*/
#define RX_MPDU_INFO_12_RX_INSERT_VLAN_S_TAG_PADDING_OFFSET          0x00000030
#define RX_MPDU_INFO_12_RX_INSERT_VLAN_S_TAG_PADDING_LSB             13
#define RX_MPDU_INFO_12_RX_INSERT_VLAN_S_TAG_PADDING_MASK            0x00002000

/* Description		RX_MPDU_INFO_12_STRIP_VLAN_C_TAG_DECAP
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Strip the VLAN during decapsulation.  Used by the OLE.
			
			<legal all>
*/
#define RX_MPDU_INFO_12_STRIP_VLAN_C_TAG_DECAP_OFFSET                0x00000030
#define RX_MPDU_INFO_12_STRIP_VLAN_C_TAG_DECAP_LSB                   14
#define RX_MPDU_INFO_12_STRIP_VLAN_C_TAG_DECAP_MASK                  0x00004000

/* Description		RX_MPDU_INFO_12_STRIP_VLAN_S_TAG_DECAP
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			Strip the double VLAN during decapsulation.  Used by
			the OLE.
			
			<legal all>
*/
#define RX_MPDU_INFO_12_STRIP_VLAN_S_TAG_DECAP_OFFSET                0x00000030
#define RX_MPDU_INFO_12_STRIP_VLAN_S_TAG_DECAP_LSB                   15
#define RX_MPDU_INFO_12_STRIP_VLAN_S_TAG_DECAP_MASK                  0x00008000

/* Description		RX_MPDU_INFO_12_PRE_DELIM_COUNT
			
			The number of delimiters before this MPDU.  
			
			
			
			Note that this number is cleared at PPDU start.
			
			
			
			If this MPDU is the first received MPDU in the PPDU and
			this MPDU gets filtered-in, this field will indicate the
			number of delimiters located after the last MPDU in the
			previous PPDU.
			
			
			
			If this MPDU is located after the first received MPDU in
			an PPDU, this field will indicate the number of delimiters
			located between the previous MPDU and this MPDU.
			
			
			
			In case of ndp or phy_err, this field will indicate the
			number of delimiters located after the last MPDU in the
			previous PPDU.
			
			<legal all>
*/
#define RX_MPDU_INFO_12_PRE_DELIM_COUNT_OFFSET                       0x00000030
#define RX_MPDU_INFO_12_PRE_DELIM_COUNT_LSB                          16
#define RX_MPDU_INFO_12_PRE_DELIM_COUNT_MASK                         0x0fff0000

/* Description		RX_MPDU_INFO_12_AMPDU_FLAG
			
			When set, received frame was part of an A-MPDU.
			
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_12_AMPDU_FLAG_OFFSET                            0x00000030
#define RX_MPDU_INFO_12_AMPDU_FLAG_LSB                               28
#define RX_MPDU_INFO_12_AMPDU_FLAG_MASK                              0x10000000

/* Description		RX_MPDU_INFO_12_BAR_FRAME
			
			In case of ndp or phy_err or AST_based_lookup_valid ==
			0, this field will be set to 0
			
			
			
			When set, received frame is a BAR frame
			
			<legal all>
*/
#define RX_MPDU_INFO_12_BAR_FRAME_OFFSET                             0x00000030
#define RX_MPDU_INFO_12_BAR_FRAME_LSB                                29
#define RX_MPDU_INFO_12_BAR_FRAME_MASK                               0x20000000

/* Description		RX_MPDU_INFO_12_RAW_MPDU
			
			Consumer: SW
			
			Producer: RXOLE
			
			
			
			RXPCU sets this field to 0 and RXOLE overwrites it.
			
			
			
			Set to 1 by RXOLE when it has not performed any 802.11
			to Ethernet/Natvie WiFi header conversion on this MPDU.
			
			<legal all>
*/
#define RX_MPDU_INFO_12_RAW_MPDU_OFFSET                              0x00000030
#define RX_MPDU_INFO_12_RAW_MPDU_LSB                                 30
#define RX_MPDU_INFO_12_RAW_MPDU_MASK                                0x40000000

/* Description		RX_MPDU_INFO_12_RESERVED_12
			
			<legal 0>
*/
#define RX_MPDU_INFO_12_RESERVED_12_OFFSET                           0x00000030
#define RX_MPDU_INFO_12_RESERVED_12_LSB                              31
#define RX_MPDU_INFO_12_RESERVED_12_MASK                             0x80000000

/* Description		RX_MPDU_INFO_13_MPDU_LENGTH
			
			In case of ndp or phy_err this field will be set to 0
			
			
			
			MPDU length before decapsulation.
			
			<legal all>
*/
#define RX_MPDU_INFO_13_MPDU_LENGTH_OFFSET                           0x00000034
#define RX_MPDU_INFO_13_MPDU_LENGTH_LSB                              0
#define RX_MPDU_INFO_13_MPDU_LENGTH_MASK                             0x00003fff

/* Description		RX_MPDU_INFO_13_FIRST_MPDU
			
			See definition in RX attention descriptor
			
			
			
			In case of ndp or phy_err, this field will be set. Note
			however that there will not actually be any data contents in
			the MPDU.
			
			<legal all>
*/
#define RX_MPDU_INFO_13_FIRST_MPDU_OFFSET                            0x00000034
#define RX_MPDU_INFO_13_FIRST_MPDU_LSB                               14
#define RX_MPDU_INFO_13_FIRST_MPDU_MASK                              0x00004000

/* Description		RX_MPDU_INFO_13_MCAST_BCAST
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_MCAST_BCAST_OFFSET                           0x00000034
#define RX_MPDU_INFO_13_MCAST_BCAST_LSB                              15
#define RX_MPDU_INFO_13_MCAST_BCAST_MASK                             0x00008000

/* Description		RX_MPDU_INFO_13_AST_INDEX_NOT_FOUND
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_AST_INDEX_NOT_FOUND_OFFSET                   0x00000034
#define RX_MPDU_INFO_13_AST_INDEX_NOT_FOUND_LSB                      16
#define RX_MPDU_INFO_13_AST_INDEX_NOT_FOUND_MASK                     0x00010000

/* Description		RX_MPDU_INFO_13_AST_INDEX_TIMEOUT
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_AST_INDEX_TIMEOUT_OFFSET                     0x00000034
#define RX_MPDU_INFO_13_AST_INDEX_TIMEOUT_LSB                        17
#define RX_MPDU_INFO_13_AST_INDEX_TIMEOUT_MASK                       0x00020000

/* Description		RX_MPDU_INFO_13_POWER_MGMT
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_POWER_MGMT_OFFSET                            0x00000034
#define RX_MPDU_INFO_13_POWER_MGMT_LSB                               18
#define RX_MPDU_INFO_13_POWER_MGMT_MASK                              0x00040000

/* Description		RX_MPDU_INFO_13_NON_QOS
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 1
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_NON_QOS_OFFSET                               0x00000034
#define RX_MPDU_INFO_13_NON_QOS_LSB                                  19
#define RX_MPDU_INFO_13_NON_QOS_MASK                                 0x00080000

/* Description		RX_MPDU_INFO_13_NULL_DATA
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_NULL_DATA_OFFSET                             0x00000034
#define RX_MPDU_INFO_13_NULL_DATA_LSB                                20
#define RX_MPDU_INFO_13_NULL_DATA_MASK                               0x00100000

/* Description		RX_MPDU_INFO_13_MGMT_TYPE
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_MGMT_TYPE_OFFSET                             0x00000034
#define RX_MPDU_INFO_13_MGMT_TYPE_LSB                                21
#define RX_MPDU_INFO_13_MGMT_TYPE_MASK                               0x00200000

/* Description		RX_MPDU_INFO_13_CTRL_TYPE
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_CTRL_TYPE_OFFSET                             0x00000034
#define RX_MPDU_INFO_13_CTRL_TYPE_LSB                                22
#define RX_MPDU_INFO_13_CTRL_TYPE_MASK                               0x00400000

/* Description		RX_MPDU_INFO_13_MORE_DATA
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_MORE_DATA_OFFSET                             0x00000034
#define RX_MPDU_INFO_13_MORE_DATA_LSB                                23
#define RX_MPDU_INFO_13_MORE_DATA_MASK                               0x00800000

/* Description		RX_MPDU_INFO_13_EOSP
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_EOSP_OFFSET                                  0x00000034
#define RX_MPDU_INFO_13_EOSP_LSB                                     24
#define RX_MPDU_INFO_13_EOSP_MASK                                    0x01000000

/* Description		RX_MPDU_INFO_13_FRAGMENT_FLAG
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_FRAGMENT_FLAG_OFFSET                         0x00000034
#define RX_MPDU_INFO_13_FRAGMENT_FLAG_LSB                            25
#define RX_MPDU_INFO_13_FRAGMENT_FLAG_MASK                           0x02000000

/* Description		RX_MPDU_INFO_13_ORDER
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			
			
			<legal all>
*/
#define RX_MPDU_INFO_13_ORDER_OFFSET                                 0x00000034
#define RX_MPDU_INFO_13_ORDER_LSB                                    26
#define RX_MPDU_INFO_13_ORDER_MASK                                   0x04000000

/* Description		RX_MPDU_INFO_13_U_APSD_TRIGGER
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_U_APSD_TRIGGER_OFFSET                        0x00000034
#define RX_MPDU_INFO_13_U_APSD_TRIGGER_LSB                           27
#define RX_MPDU_INFO_13_U_APSD_TRIGGER_MASK                          0x08000000

/* Description		RX_MPDU_INFO_13_ENCRYPT_REQUIRED
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_ENCRYPT_REQUIRED_OFFSET                      0x00000034
#define RX_MPDU_INFO_13_ENCRYPT_REQUIRED_LSB                         28
#define RX_MPDU_INFO_13_ENCRYPT_REQUIRED_MASK                        0x10000000

/* Description		RX_MPDU_INFO_13_DIRECTED
			
			In case of ndp or phy_err or Phy_err_during_mpdu_header
			this field will be set to 0
			
			
			
			See definition in RX attention descriptor
			
			<legal all>
*/
#define RX_MPDU_INFO_13_DIRECTED_OFFSET                              0x00000034
#define RX_MPDU_INFO_13_DIRECTED_LSB                                 29
#define RX_MPDU_INFO_13_DIRECTED_MASK                                0x20000000

/* Description		RX_MPDU_INFO_13_AMSDU_PRESENT
			
			Field only valid when Mpdu_qos_control_valid is set
			
			
			
			The 'amsdu_present' bit within the QoS control field of
			the MPDU
			
			<legal all>
*/
#define RX_MPDU_INFO_13_AMSDU_PRESENT_OFFSET                         0x00000034
#define RX_MPDU_INFO_13_AMSDU_PRESENT_LSB                            30
#define RX_MPDU_INFO_13_AMSDU_PRESENT_MASK                           0x40000000

/* Description		RX_MPDU_INFO_13_RESERVED_13
			
			<legal 0>
*/
#define RX_MPDU_INFO_13_RESERVED_13_OFFSET                           0x00000034
#define RX_MPDU_INFO_13_RESERVED_13_LSB                              31
#define RX_MPDU_INFO_13_RESERVED_13_MASK                             0x80000000

/* Description		RX_MPDU_INFO_14_MPDU_FRAME_CONTROL_FIELD
			
			Field only valid when Mpdu_frame_control_valid is set
			
			
			
			The frame control field of this received MPDU.
			
			
			
			Field only valid when Ndp_frame and phy_err are NOT set
			
			
			
			Bytes 0 + 1 of the received MPDU
			
			<legal all>
*/
#define RX_MPDU_INFO_14_MPDU_FRAME_CONTROL_FIELD_OFFSET              0x00000038
#define RX_MPDU_INFO_14_MPDU_FRAME_CONTROL_FIELD_LSB                 0
#define RX_MPDU_INFO_14_MPDU_FRAME_CONTROL_FIELD_MASK                0x0000ffff

/* Description		RX_MPDU_INFO_14_MPDU_DURATION_FIELD
			
			Field only valid when Mpdu_duration_valid is set
			
			
			
			The duration field of this received MPDU.
			
			<legal all>
*/
#define RX_MPDU_INFO_14_MPDU_DURATION_FIELD_OFFSET                   0x00000038
#define RX_MPDU_INFO_14_MPDU_DURATION_FIELD_LSB                      16
#define RX_MPDU_INFO_14_MPDU_DURATION_FIELD_MASK                     0xffff0000

/* Description		RX_MPDU_INFO_15_MAC_ADDR_AD1_31_0
			
			Field only valid when mac_addr_ad1_valid is set
			
			
			
			The Least Significant 4 bytes of the Received Frames MAC
			Address AD1
			
			<legal all>
*/
#define RX_MPDU_INFO_15_MAC_ADDR_AD1_31_0_OFFSET                     0x0000003c
#define RX_MPDU_INFO_15_MAC_ADDR_AD1_31_0_LSB                        0
#define RX_MPDU_INFO_15_MAC_ADDR_AD1_31_0_MASK                       0xffffffff

/* Description		RX_MPDU_INFO_16_MAC_ADDR_AD1_47_32
			
			Field only valid when mac_addr_ad1_valid is set
			
			
			
			The 2 most significant bytes of the Received Frames MAC
			Address AD1
			
			<legal all>
*/
#define RX_MPDU_INFO_16_MAC_ADDR_AD1_47_32_OFFSET                    0x00000040
#define RX_MPDU_INFO_16_MAC_ADDR_AD1_47_32_LSB                       0
#define RX_MPDU_INFO_16_MAC_ADDR_AD1_47_32_MASK                      0x0000ffff

/* Description		RX_MPDU_INFO_16_MAC_ADDR_AD2_15_0
			
			Field only valid when mac_addr_ad2_valid is set
			
			
			
			The Least Significant 2 bytes of the Received Frames MAC
			Address AD2
			
			<legal all>
*/
#define RX_MPDU_INFO_16_MAC_ADDR_AD2_15_0_OFFSET                     0x00000040
#define RX_MPDU_INFO_16_MAC_ADDR_AD2_15_0_LSB                        16
#define RX_MPDU_INFO_16_MAC_ADDR_AD2_15_0_MASK                       0xffff0000

/* Description		RX_MPDU_INFO_17_MAC_ADDR_AD2_47_16
			
			Field only valid when mac_addr_ad2_valid is set
			
			
			
			The 4 most significant bytes of the Received Frames MAC
			Address AD2
			
			<legal all>
*/
#define RX_MPDU_INFO_17_MAC_ADDR_AD2_47_16_OFFSET                    0x00000044
#define RX_MPDU_INFO_17_MAC_ADDR_AD2_47_16_LSB                       0
#define RX_MPDU_INFO_17_MAC_ADDR_AD2_47_16_MASK                      0xffffffff

/* Description		RX_MPDU_INFO_18_MAC_ADDR_AD3_31_0
			
			Field only valid when mac_addr_ad3_valid is set
			
			
			
			The Least Significant 4 bytes of the Received Frames MAC
			Address AD3
			
			<legal all>
*/
#define RX_MPDU_INFO_18_MAC_ADDR_AD3_31_0_OFFSET                     0x00000048
#define RX_MPDU_INFO_18_MAC_ADDR_AD3_31_0_LSB                        0
#define RX_MPDU_INFO_18_MAC_ADDR_AD3_31_0_MASK                       0xffffffff

/* Description		RX_MPDU_INFO_19_MAC_ADDR_AD3_47_32
			
			Field only valid when mac_addr_ad3_valid is set
			
			
			
			The 2 most significant bytes of the Received Frames MAC
			Address AD3
			
			<legal all>
*/
#define RX_MPDU_INFO_19_MAC_ADDR_AD3_47_32_OFFSET                    0x0000004c
#define RX_MPDU_INFO_19_MAC_ADDR_AD3_47_32_LSB                       0
#define RX_MPDU_INFO_19_MAC_ADDR_AD3_47_32_MASK                      0x0000ffff

/* Description		RX_MPDU_INFO_19_MPDU_SEQUENCE_CONTROL_FIELD
			
			
			
			
			The sequence control field of the MPDU
			
			<legal all>
*/
#define RX_MPDU_INFO_19_MPDU_SEQUENCE_CONTROL_FIELD_OFFSET           0x0000004c
#define RX_MPDU_INFO_19_MPDU_SEQUENCE_CONTROL_FIELD_LSB              16
#define RX_MPDU_INFO_19_MPDU_SEQUENCE_CONTROL_FIELD_MASK             0xffff0000

/* Description		RX_MPDU_INFO_20_MAC_ADDR_AD4_31_0
			
			Field only valid when mac_addr_ad4_valid is set
			
			
			
			The Least Significant 4 bytes of the Received Frames MAC
			Address AD4
			
			<legal all>
*/
#define RX_MPDU_INFO_20_MAC_ADDR_AD4_31_0_OFFSET                     0x00000050
#define RX_MPDU_INFO_20_MAC_ADDR_AD4_31_0_LSB                        0
#define RX_MPDU_INFO_20_MAC_ADDR_AD4_31_0_MASK                       0xffffffff

/* Description		RX_MPDU_INFO_21_MAC_ADDR_AD4_47_32
			
			Field only valid when mac_addr_ad4_valid is set
			
			
			
			The 2 most significant bytes of the Received Frames MAC
			Address AD4
			
			<legal all>
*/
#define RX_MPDU_INFO_21_MAC_ADDR_AD4_47_32_OFFSET                    0x00000054
#define RX_MPDU_INFO_21_MAC_ADDR_AD4_47_32_LSB                       0
#define RX_MPDU_INFO_21_MAC_ADDR_AD4_47_32_MASK                      0x0000ffff

/* Description		RX_MPDU_INFO_21_MPDU_QOS_CONTROL_FIELD
			
			Field only valid when mpdu_qos_control_valid is set
			
			
			
			The sequence control field of the MPDU
			
			<legal all>
*/
#define RX_MPDU_INFO_21_MPDU_QOS_CONTROL_FIELD_OFFSET                0x00000054
#define RX_MPDU_INFO_21_MPDU_QOS_CONTROL_FIELD_LSB                   16
#define RX_MPDU_INFO_21_MPDU_QOS_CONTROL_FIELD_MASK                  0xffff0000

/* Description		RX_MPDU_INFO_22_MPDU_HT_CONTROL_FIELD
			
			Field only valid when mpdu_qos_control_valid is set
			
			
			
			The HT control field of the MPDU
			
			<legal all>
*/
#define RX_MPDU_INFO_22_MPDU_HT_CONTROL_FIELD_OFFSET                 0x00000058
#define RX_MPDU_INFO_22_MPDU_HT_CONTROL_FIELD_LSB                    0
#define RX_MPDU_INFO_22_MPDU_HT_CONTROL_FIELD_MASK                   0xffffffff


#endif // _RX_MPDU_INFO_H_
