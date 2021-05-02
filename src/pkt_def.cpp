
#include "pkt_def.h"
#include "loguru.hpp"


int P2001::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    cdr.write_char_array(_str2bcd4(id, id_dst, 4), 4);
    return cdr.good_bit();
};
int P2001A::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3001::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    cdr.write_char_array(_str2bcd4(id, timestamp, 7), 7);
    cdr.write_char_array(_str2bcd4(id, id_src, 4), 4);
    cdr.write_char_array(_str2bcd4(id, id_dst, 4), 4);
    cdr.write_char_array((const char *)oper_id, 10); oper_id[10]=0;
    cdr << ACE_CDR::Char(mode);
    return cdr.good_bit();
};
int P3001A::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3002::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    cdr.write_char_array(_str2bcd4(id, now, 7), 7);
    cdr.write_char_array(_str2bcd4(id, id_dst, 4), 4);
    cdr << ACE_CDR::Char(mode);
    cdr.write_char_array(_str2bcd4(id, affect_time, 7), 7);
    return cdr.good_bit();
};
int P3002A::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3003A::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3004::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    cdr.write_char_array(_str2bcd4(id, id_dst, 4), 4);
    return cdr.good_bit();
};
int P3004A::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3005A::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P2001::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    cdr.read_char_array(id, 4); _bcd2str4(id_dst, id, 4);
    return cdr.good_bit();
};
int P2001A::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3001::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    cdr.read_char_array(id, 7); _bcd2str4(timestamp, id, 7);
    cdr.read_char_array(id, 4); _bcd2str4(id_src, id, 4);
    cdr.read_char_array(id, 4); _bcd2str4(id_dst, id, 4);
    cdr.read_char_array((char *)oper_id, 10); oper_id[10] = 0;
    ACE_CDR::Char _m_1; cdr >> _m_1; mode = _m_1;
    return cdr.good_bit();
};
int P3001A::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3002::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    cdr.read_char_array(id, 7); _bcd2str4(now, id, 7);
    cdr.read_char_array(id, 4); _bcd2str4(id_dst, id, 4);
    ACE_CDR::Char _m_1; cdr >> _m_1; mode = _m_1;
    cdr.read_char_array(id, 7); _bcd2str4(affect_time, id, 7);
    return cdr.good_bit();
};
int P3002A::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3003A::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3004::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    cdr.read_char_array(id, 4); _bcd2str4(id_dst, id, 4);
    return cdr.good_bit();
};
int P3004A::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};
int P3005A::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    return cdr.good_bit();
};

int _Pkt::decode(ACE_InputCDR &cdr) {
    cdr.reset_byte_order(__BYTE_ORDER__);
    ACE_CDR::Short length;
    cdr >> length;
    uint8_t nchecksum[16];
    md5((const uint8_t *)cdr.rd_ptr(), length - sizeof(nchecksum), nchecksum);
    header.decode(cdr);
    _BodyBase *p = NULL;

    if(false){

    }else if(header.type == 0x2001){
        p = (header.ack_flag == 0x00) ? (_BodyBase *)new P2001() : (_BodyBase *)new P2001A();
    }else if(header.type == 0x3001){
        p = (header.ack_flag == 0x00) ? (_BodyBase *)new P3001() : (_BodyBase *)new P3001A();
    }else if(header.type == 0x3002){
        p = (header.ack_flag == 0x00) ? (_BodyBase *)new P3002() : (_BodyBase *)new P3002A();
    }else if(header.type == 0x3004){
        p = (header.ack_flag == 0x00) ? (_BodyBase *)new P3004() : (_BodyBase *)new P3004A();
    }
    if(p == NULL){
        LOG_F(ERROR, "invalid received pkt type: 0x%x", header.type);
        return -1;
    }
    p->decode(cdr);
    body = p;
    cdr.read_char_array((char *)checksum, 16);
    if(memcmp(nchecksum, checksum, sizeof(nchecksum)) != 0){
        LOG_F (ERROR, "decode checksum failed");
        return -1;
    }
    return 0;
}
    
