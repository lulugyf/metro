
import sys

pkts = {
    "2001":[ # 链路测试
        ["id_dst",    "BCD", 4, "目的设备编码"],
    ],
    "2001A": [],
    "3001":[ # 模式命令
        ["timestamp", "BCD",  7, "起时间 时间戳"],
        ["id_src",    "BCD",  4, "发起方节点 ID"],
        ["id_dst",    "BCD",  4, " 目的节点 ID,  发生降级运营模式的车站"],
        ["oper_id",   "char", 10, " 操作员 ID"],
        ["mode",      "byte", 1,  "降级运营模式 ID"],
    ],
    "3001A":[],
    "3002":[ # 模式通知
        ["now",       "BCD",  7, " 当前时间 时间戳"],
        ["id_dst",    "BCD",  4, "模式节点 ID, 发生降级运营模式的车站或者终端设备 ID"],
        ["mode",      "byte", 1,  "降级运营模式 ID"],
        ["affect_time", "BCD",  7, "降级运营模式发生时间"],
    ],
    "3002A":[],
    # "3003":[ # 模式广播  【有变长记录， 得手工写】
    "3003A":[],
    "3004":[ # 模式查询请求
        ["id_dst",    "BCD", 4, "被查询的线路/车站/设备的节点编号"],
    ],
    "3004A":[],
    # "3005":[ # 模式查询应答  【有变长记录， 得手工写】
    "3005A":[],

}





tpl_cls = '''
class P%s : public _BodyBase{
public:
%s
    virtual uint16_t type() { return 0x%s; }
    virtual uint32_t len() { return %d; }
    virtual int encode(ACE_OutputCDR &cdr) ;
    virtual int decode(ACE_InputCDR  &cdr) ;
};'''

def gen_header(fout):
    print('''
////// !! Do not modify this file, It was generated, Directly modify will be overwritten.
#ifndef __PKT_IMPL_H
 #define __PKT_IMPL_H
#include "pkt.h"

    ''', file=fout)
    cls = []
    for pid, fld in pkts.items():
        flds = []
        bodylen = 0
        for fname, ftype, flen, fcomment in fld:
            bodylen += flen
            if ftype == "BCD":
                f_def = f"char {fname}[{flen*2}+1]; // {fcomment}"
            elif ftype == "char":
                f_def = f"char {fname}[{flen}+1]; // {fcomment}"
            elif ftype == "byte":
                f_def = f"u_char {fname}; // {fcomment}"
            elif ftype == "word":
                f_def = f"u_short {fname}; // {fcomment}"
            elif ftype == "long":
                f_def = f"u_int {fname}; // {fcomment}"
            elif ftype == "block":
                f_def = f"char {fname}[{flen}+1]; // {fcomment}"
            else:
                print("inalid ftype:", ftype, pid)
                sys.exit()
            flds.append(f"    {f_def}")
        print(tpl_cls % (pid, "\n".join(flds), pid[:4], bodylen), file=fout)
    print('#endif // __PKT_IMPL_H', file=fout)

def gen_src(fout):
    print('''
////// !! Do not modify this file, It was generated, Directly modify will be overwritten.
#include "pkt_def.h"
#include "loguru.hpp"

''',
     file=fout)
    for pid, fld in pkts.items():
        print('''int P%s::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];''' % pid, file=fout)
        for fname, ftype, flen, fcomment in fld:
            if ftype == "BCD":
                f_def = f"cdr.write_char_array(_str2bcd4(id, {fname}, {flen}), {flen});"
            elif ftype == "char":
                f_def = f"cdr.write_char_array((const char *){fname}, {flen}); {fname}[{flen}]=0;"
            elif ftype == "block":
                f_def = f"cdr.write_char_array((const char *){fname}, {flen});"
            elif ftype == "byte":
                f_def = f"cdr << ACE_CDR::Char({fname});"
            elif ftype == "word":
                f_def = f"cdr << ACE_CDR::UShort({fname});"
            elif ftype == "long":
                f_def = f"cdr << ACE_CDR::Long({fname});"
            else:
                print("inalid ftype:", ftype, pid)
                sys.exit()
            print(f"    {f_def}", file=fout)
        print("    return cdr.good_bit();\n};", file=fout)

    for pid, fld in pkts.items():
        print('''int P%s::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];''' % pid, file=fout)
        i = 0
        for fname, ftype, flen, fcomment in fld:
            if ftype == "BCD":
                f_def = f"cdr.read_char_array(id, {flen}); _bcd2str4({fname}, id, {flen});"
            elif ftype == "char":
                f_def = f"cdr.read_char_array((char *){fname}, {flen}); {fname}[{flen}] = 0;"
            elif ftype == "block":
                f_def = f"cdr.write_char_array((const char *){fname}, {flen});"
            elif ftype == "byte":
                i += 1
                f_def = f"ACE_CDR::Char _m_{i}; cdr >> _m_{i}; {fname} = _m_{i};"
            elif ftype == "word":
                i += 1
                f_def = f"ACE_CDR::UShort _m_{i}; cdr >> _m_{i}; {fname} = _m_{i};"
            elif ftype == "long":
                i += 1
                f_def = f"ACE_CDR::Long _m_{i}; cdr >> _m_{i}; {fname} = _m_{i};"
            else:
                print("inalid ftype:", ftype, pid)
                sys.exit()
            print(f"    {f_def}", file=fout)
        print("    return cdr.good_bit();\n};", file=fout)

    print('''
int _Pkt::decode(ACE_InputCDR &cdr) {
    cdr.reset_byte_order(__BYTE_ORDER__);
    ACE_CDR::Short length;
    cdr >> length;
    uint8_t nchecksum[16];
    md5((const uint8_t *)cdr.rd_ptr(), length - sizeof(nchecksum), nchecksum);
    header.decode(cdr);
    _BodyBase *p = NULL;

    if(false){
''', file=fout)

    for pid in pkts.keys():
        if pid[-1] == 'A': continue
        print('''    }else if(header.type == 0x%s){
        p = (header.ack_flag == 0x00) ? (_BodyBase *)new P%s() : (_BodyBase *)new P%sA();''' %(
                    pid, pid, pid),
         file=fout)

    # 手工编写的报文class， 需要在这里添加记录
    print('''    }else if(header.type == 0x1999){
    p = (header.ack_flag == 0x00) ? (_BodyBase *)new P1999() : (_BodyBase *)new P1999A();''',
        file=fout)

    print('''    }
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
    ''', file=fout)

'''
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
'''

tpl_mthd = '''
int P3001::encode(ACE_OutputCDR &cdr) {
    ACE_CDR::Char id[7];
    cdr.write_char_array(_str2bcd4(id, timestamp, 7), 7);
    cdr.write_char_array(_str2bcd4(id, id_src), 4);
    cdr.write_char_array(_str2bcd4(id, id_dst), 4);
    cdr.write_char_array((const char *)oper_id, 10);
    cdr << ACE_OutputCDR::from_char(mode);
    return cdr.good_bit();
};

int P3001::decode(ACE_InputCDR  &cdr) {
    ACE_CDR::Char id[7];
    cdr.read_char_array(id, 7); _bcd2str4(timestamp, id, 7);
    cdr.read_char_array(id, 4); _bcd2str4(id_src, id);
    cdr.read_char_array(id, 4); _bcd2str4(id_dst, id);
    cdr.read_char_array((char *)oper_id, 10); oper_id[10] = 0;
    ACE_CDR::Char _m; cdr >> _m; mode = _m;
    return cdr.good_bit();
};
'''

        

with open("../include/pkt_def.h", "w", encoding="utf8") as fo:
    gen_header(fo)
with open("../src/pkt_def.cpp", "w", encoding="utf8") as fo:
    gen_src(fo)
