

#include "ace/OS_main.h"

// FUZZ: disable check_for_streams_include
#include "ace/streams.h"

#include "ace/Log_Msg.h"
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/Auto_Ptr.h"
#include "ace/OS_Memory.h"
#include "loguru.hpp"

#include <string>
#include <time.h>

#include "pkt.h"

// 字符串 "1234" 转换为 unsigned char[4] {0x01, 0x02, 0x03, 0x04}
void _Header::setid(string &id, int _type) {}

void _Header::getid(string &id, int _type) {}


void test_loguru(int argc, ACE_TCHAR *argv[]) {
    loguru::init(argc, argv);

    // Put every log message in "everything.log":
    loguru::add_file("everything.log", loguru::Append, loguru::Verbosity_MAX);

    // Only log INFO, WARNING, ERROR and FATAL to "latest_readable.log":
    loguru::add_file("latest_readable.log", loguru::Truncate, loguru::Verbosity_INFO);

    LOG_SCOPE_F(INFO, "Will indent all log messages within this scope.");
    LOG_F(INFO, "I'm hungry for some %.3f!", 3.14159);
    LOG_F(2, "Will only show if verbosity is 2 or higher");
    // VLOG_F(get_log_level(), "Use vlog for dynamic log level (integer in the range 0-9, inclusive)");

    int badness = 2;
    LOG_IF_F(ERROR, 0, "Will only show if badness happens");

    const char * filename = "pkt.cpp";
    auto fp = fopen(filename, "r");
    CHECK_F(fp != NULL, "Failed to open file '%s'", filename);
    int length = 2;
    CHECK_GT_F(length, 0); // Will print the value of `length` on failure.

    int a=1, b=2;
    CHECK_EQ_F(a, b, "You can also supply a custom message, like to print something: %d", a + b);
}

int test_ace_log(int argc, ACE_TCHAR *argv[]) {
    if(ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::OSTREAM) == -1 ) {
        ACE_ERROR( (LM_ERROR, "can not open logger!") );
        return 2;
    }

    ACE_LOG_MSG->set_flags (ACE_Log_Msg::OSTREAM);


    int counter  = 2;
    double f = 3.1416 * counter++;
      int i = 10000;
 
      ACE_DEBUG ((LM_INFO,
                  "%10f, %*s%s = %d\n",
                  f,
                  8,
                  "",
                  "hello",
                  i));

    static int array[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};

      // Print out the binary bytes of the array in hex form.
      ACE_LOG_MSG->log_hexdump (LM_DEBUG,
                                (char *) array,
                                sizeof array);
    
    _Header t;

    string id = "1234";
    t.setid(id, 1);

    t.getid(id, 1);
    cout << "id:" << id << endl;
    return 0;
}

ACE_CDR::Char * _str2bcd4(ACE_CDR::Char *dst, const char *src, int _l) {

    for(int i=0; i<_l; i++)
        dst[i] = ((src[i*2]-'0') << 4 ) | ((src[i*2+1]-'0') & 0x0f);
    return dst;
}

// int operator<< (ACE_OutputCDR &cdr, const _Header &header)
int _Header::encode(ACE_OutputCDR &cdr)
{
  cdr << ACE_CDR::Short (type);
  ACE_CDR::Char id[4];
  cdr.write_char_array(_str2bcd4(id, id_src), 4);
  cdr.write_char_array(_str2bcd4(id, id_dst), 4);
  cdr.write_char_array(_str2bcd4(id, id_trn), 4);
  cdr << ACE_CDR::Long (pkt_seq);
  cdr << ACE_CDR::Char (ack_flag);
  cdr << ACE_CDR::Char (ver); // 1
  cdr << ACE_CDR::Char (ack);

  return cdr.good_bit ();
}

void _bcd2str4(char *dst, const ACE_CDR::Char *src, int _l) {
    for(int i=0; i<_l; i++){
        dst[i*2] = (src[i] >> 4 & 0x0f ) + '0';
        dst[i*2+1] = (src[i] & 0x0f) + '0';
    }
    dst[_l*2] = 0;
}

//int operator>> (ACE_InputCDR &cdr, _Header &header)
int _Header::decode(ACE_InputCDR &cdr)
{
    ACE_CDR::Short type;
    ACE_CDR::Char id[5];
    ACE_CDR::Long pkt_seq;
    ACE_CDR::Char ack_flag;
    ACE_CDR::Char ver;
    ACE_CDR::Char ack;

    // ACE_LOG_MSG->log_hexdump(LM_DEBUG, cdr.rd_ptr(), cdr.length());

    cdr >> type;
    cdr.read_char_array(id, 4); _bcd2str4(id_src, id, 4);
    cdr.read_char_array(id, 4); _bcd2str4(id_dst, id, 4);
    cdr.read_char_array(id, 4); _bcd2str4(id_trn, id, 4);
    cdr >> pkt_seq;
    cdr >> ack_flag;
    cdr >> ver;
    cdr >> ack;
  
    // ACE_CDR::Char *log_msg;
    // ACE_NEW_RETURN (log_msg, ACE_CDR::Char[buffer_len + 1], -1);
    // ACE_Auto_Basic_Array_Ptr<ACE_TCHAR> log_msg_p (log_msg);
    this->type = type;
    this->pkt_seq = pkt_seq;
    this->ack_flag = ack_flag;
    this->ver = ver;
    this->ack = ack;

    return cdr.good_bit ();
}

/*
int P2001::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[5];
    cdr.read_char_array(id, 4); _bcd2str4(id_dst, id);
    return cdr.good_bit();
};

int P2001::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[5];
    cdr.write_char_array(_str2bcd4(id, id_dst), 4);
    return cdr.good_bit();
};


int P3001::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    cdr.write_char_array(_str2bcd4(id, timestamp, 7), 7);
    cdr.write_char_array(_str2bcd4(id, id_src), 4);
    cdr.write_char_array(_str2bcd4(id, id_dst), 4);
    cdr.write_char_array((const char *)oper_id, 10);
    cdr << ACE_OutputCDR::from_char(mode);

    int x = 1;
    cdr << (ACE_CDR::Long)x;
    return cdr.good_bit();
};

int P3001::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    cdr.read_char_array(id, 7); _bcd2str4(timestamp, id, 7);
    cdr.read_char_array(id, 4); _bcd2str4(id_src, id);
    cdr.read_char_array(id, 4); _bcd2str4(id_dst, id);
    cdr.read_char_array((char *)oper_id, 10); oper_id[10] = 0;
    ACE_CDR::Char _m;
    cdr >> _m;
    mode = _m;
    return cdr.good_bit();
};

void P3001::setTime(long long sec) {
    if(sec == 0)
        time((time_t *)&sec);
    char tt[14+1];
    struct tm * info = localtime( &sec );
 
    strftime(tt, sizeof(tt), "%Y-%m-%d %H:%M:%S", info);
    
}
*/

int _Pkt::encode(ACE_OutputCDR &cdr){
    cdr.reset_byte_order(__BYTE_ORDER__);
    ACE_CDR::Short length = header.len();
    if(body != NULL)
        length += body->len();
    length += sizeof(checksum);
    cout << " encode header len:" << length << " pkt ptr:" << cdr.total_length() << endl;

    cdr << length;
    LOG_F(INFO, "encode pkt =type 0x%x", header.type);
    header.encode(cdr);
    cout << " encode length after header:" << cdr.total_length() << endl;
    if (body != NULL) {
        body->encode(cdr);
    }
    cout << " encode length after body:" << cdr.total_length() << endl;

    md5((const uint8_t *)(cdr.begin()->rd_ptr() + 2), length - sizeof(checksum),
        checksum);

    cdr.write_char_array((const ACE_CDR::Char *)checksum, sizeof(checksum));
    cout << " encode length after checksum:" << cdr.total_length() << endl;

    return cdr.good_bit();
}

/*
int _Pkt::decode(ACE_InputCDR &cdr) {
    cdr.reset_byte_order(__BYTE_ORDER__);
    ACE_CDR::Short length;
    cdr >> length;

    // compute checksum
    uint8_t nchecksum[16];
    md5((const uint8_t *)cdr.rd_ptr(), length - sizeof(nchecksum), nchecksum);
    // ACE_LOG_MSG->log_hexdump(LM_DEBUG, in.rd_ptr(), len);

    // cout << "decode length in header:" << length << "  rd_ptr:" << cdr.length() << endl;
    cdr >> header;
    // cout << cdr.length() << endl;
    _BodyBase *p = NULL;
    if (header.type == 0x3001){
        p = (header.ack_flag == 0x00) ? (_BodyBase *)new P3001() : (_BodyBase *)new P3001A();
    }else if(header.type == 0x2001){
        p = (header.ack_flag == 0x00) ? (_BodyBase *)new P2001() : (_BodyBase *)new P2001A();
    }
    p->decode(cdr);
    body = p;
    // cout << cdr.length() << endl;

    // check md5
    cdr.read_char_array((char *)checksum, 16);

    if(memcmp(nchecksum, checksum, sizeof(nchecksum)) != 0){
        ACE_DEBUG ((LM_ERROR, "decode checksum failed\n") );
        return -1;
    }
    
    return 0;
}
*/

int _Pkt::encodeAck(ACE_OutputCDR &cdr,  uint8_t ack){
    _Header r = _Header();
    uint8_t checksum[16];
    header.genAck(r, ack);

    cdr.reset_byte_order(__BYTE_ORDER__);
    ACE_CDR::Short length = r.len();

    length += sizeof(checksum);

    cdr << length;
    r.encode(cdr);

    md5((const uint8_t *)(cdr.begin()->rd_ptr() + 2), length - sizeof(checksum),
        checksum);

    cdr.write_char_array((const ACE_CDR::Char *)checksum, sizeof(checksum));
    return cdr.good_bit();
}


void test_pkt() {
    /*
    P3001 *p1 = new P3001();
    memcpy(p1->id_src, "12345678", 8);
    memcpy(p1->id_dst, "87654321", 8);
    strcpy(p1->oper_id, "myoperid");
    p1->setTime();

    _Pkt p;
    p.setBody(p1);
    memcpy(p.header.id_dst, "23456789", 8);
    memcpy(p.header.id_src, "98765432", 8); 

    ACE_OutputCDR payload (MAX_PKT_SIZE);
    p.encode(payload);

    int len = payload.total_length();

    const char *s = (const char *)payload.begin()->rd_ptr();

    //ACE_OS::printf("length: %d\n", len);
    cout << "length:" << len << " " << endl;

    ACE_LOG_MSG->log_hexdump (LM_DEBUG, s, len);



    // decode test
    ACE_Message_Block *msg = new ACE_Message_Block (ACE_DEFAULT_CDR_BUFSIZE);
    memcpy(msg->wr_ptr(), s, len);
    msg->wr_ptr(len);
    ACE_InputCDR in(msg);

    _Pkt pr;
    int ret = pr.decode(in);

    cout << "decoded return:" << ret << endl;
    P3001 *body = (P3001 *)pr.body;
    cout << "id_dst:" << body ->id_dst  << "   oper_id:" << body->oper_id << endl;

    */

    // char bcd[4], bcds[9], tgt[9];
    // strcpy(bcds, "12345678");
    // _str2bcd4(bcd, (const uint8_t *)bcds, 4);

    // _bcd2str4((uint8_t *)tgt, bcd, 4);
    // cout << "-----:" << tgt << endl;


}

#ifdef __TEST__
int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
    ACE_LOG_MSG->open(argv[0], ACE_Log_Msg::OSTREAM);

    // test_loguru(argc, argv);

    // test_ace_log(argc, argv);

    test_pkt();

    return 0;
}
#endif

