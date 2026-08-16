#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

// HLE for Sega 837-13551 JVS I/O board and the Naomi 315-6149 JVS/Maple bridge.
// Board identity is verified directly from the user's 315-6215 firmware ROM.
// Protocol behavior follows the matching Flycast 2.6 Naomi JVS implementation.
class NaomiJvs13551 {
public:
    static constexpr const char* kBoardId =
        "SEGA ENTERPRISES,LTD.;I/O BD JVS;837-13551 ;Ver1.00;98/10";

    std::array<uint8_t, 0x80> eeprom{};
    std::array<std::vector<uint8_t>, 32> rx{};
    std::array<std::vector<uint8_t>, 32> repeat{};
    uint8_t nodeId = 1;
    uint16_t steering = 0x8000;
    uint16_t accel = 0x0000;
    uint16_t brake = 0x0000;
    uint16_t p1Buttons = 0;
    uint16_t p2Buttons = 0;
    uint8_t systemBits = 0;
    uint16_t coinCount[2]{};
    uint8_t digitalOutputs = 0;

    NaomiJvs13551() { eeprom.fill(0xFF); }

    static void append(std::vector<uint8_t>& o, uint8_t v) { o.push_back(v); }
    static void append32le(std::vector<uint8_t>& o, uint32_t v) {
        append(o, v); append(o, v >> 8); append(o, v >> 16); append(o, v >> 24);
    }

    std::vector<uint8_t> jvsMessage(const uint8_t* in, size_t n) {
        if (!in || n == 0) return {};
        const uint8_t c = in[0];
        if (c == 0xF0) return {}; // reset: no reply
        if (c == 0xF1 && (n < 2 || in[1] != nodeId)) return {};

        std::vector<uint8_t> o{0xE0, 0x00, 0x00};
        auto status = [&]{ append(o, 1); };
        switch (c) {
        case 0xF1:
            status(); status(); append(o, 5); break;
        case 0x10:
            status(); status();
            for (const char* p = kBoardId; *p; ++p) append(o, (uint8_t)*p);
            append(o, 0); break;
        case 0x11: status(); status(); append(o, 0x11); break; // command format 1.1
        case 0x12: status(); status(); append(o, 0x20); break; // JVS 2.0
        case 0x13: status(); status(); append(o, 0x10); break; // comm 1.0
        case 0x14: // features: exact 837-13551 profile
            status(); status();
            append(o,1); append(o,2); append(o,13); append(o,0); // digital: 2p, 13 bits
            append(o,2); append(o,2); append(o,0);  append(o,0); // 2 coin slots
            append(o,3); append(o,8); append(o,0x10); append(o,0); // 8x 16-bit analog
            append(o,0x12); append(o,6); append(o,0); append(o,0); // 6 outputs
            append(o,0); break;
        default:
            if (c >= 0x20 && c <= 0x38) {
                status();
                size_t i = 0;
                while (i < n) {
                    uint8_t q = in[i];
                    if (q == 0x20 && i + 2 < n) {
                        status();
                        append(o, systemBits);
                        unsigned players = in[i+1], bytes = in[i+2];
                        for (unsigned p=0; p<players; ++p) {
                            uint16_t b = p == 0 ? p1Buttons : p2Buttons;
                            append(o, (uint8_t)(b >> 8));
                            if (bytes == 2) append(o, (uint8_t)b);
                        }
                        i += 3;
                    } else if (q == 0x21 && i + 1 < n) {
                        status(); unsigned slots=in[i+1];
                        for (unsigned s=0;s<slots;++s) {
                            uint16_t cc = s < 2 ? coinCount[s] : 0;
                            append(o,(cc>>8)&0x3F); append(o,cc);
                        }
                        i += 2;
                    } else if (q == 0x22 && i + 1 < n) {
                        status(); unsigned chans=in[i+1];
                        for (unsigned a=0;a<chans;++a) {
                            uint16_t v = a==0?steering:(a==1?accel:(a==2?brake:0x8000));
                            v = std::min<uint16_t>(0xff7f, v);
                            if (v & 0x80) v = (uint16_t)(v + 0x100);
                            append(o,v>>8); append(o,v);
                        }
                        i += 2;
                    } else if (q == 0x30 && i + 3 < n) {
                        unsigned slot=in[i+1]; uint16_t dec=(in[i+2]<<8)|in[i+3];
                        if (slot>=1 && slot<=2) coinCount[slot-1] = coinCount[slot-1] > dec ? coinCount[slot-1]-dec : 0;
                        status(); i += 4;
                    } else if (q == 0x32 && i + 1 < n) {
                        unsigned cnt=in[i+1];
                        if (cnt && i+2<n) digitalOutputs=in[i+2];
                        status(); i += std::min<size_t>(n-i, cnt+2);
                    } else {
                        append(o,2); break;
                    }
                }
            } else {
                append(o,2);
            }
            break;
        }
        o[2] = (uint8_t)(o.size() - 2);
        uint8_t crc=0; for (size_t i=1;i<o.size();++i) crc=(uint8_t)(crc+o[i]);
        append(o,crc);
        return o;
    }

    uint8_t senseLine(uint8_t node) const { return node == 1 ? 0x8E : 0x8F; }

    void queueJvs(uint8_t node, uint8_t channel, const uint8_t* cmd, size_t len, bool useRepeat, bool repeatFirst=false) {
        if (channel >= rx.size()) return;
        if (node == 0xFF) {
            // JVS broadcast: fan out to every attached board. This model has one 837-13551 at node 1.
            queueJvs(1, channel, cmd, len, useRepeat, repeatFirst);
            return;
        }
        if (node < 1 || node > 32) return;
        std::vector<uint8_t> req;
        if (useRepeat && !repeat[node-1].empty() && repeatFirst) req.insert(req.end(), repeat[node-1].begin(), repeat[node-1].end());
        if (cmd && len) req.insert(req.end(), cmd, cmd+len);
        if (useRepeat && !repeat[node-1].empty() && !repeatFirst) req.insert(req.end(), repeat[node-1].begin(), repeat[node-1].end());
        auto body=jvsMessage(req.data(),req.size());
        if (!body.empty()) {
            auto& r=rx[channel];
            r.clear(); r.push_back(node); r.push_back(0); r.push_back((uint8_t)body.size());
            r.insert(r.end(),body.begin(),body.end());
        }
    }

    std::vector<uint8_t> mapleCommand(uint8_t cmd, const uint8_t* p, size_t n) {
        std::vector<uint8_t> o;
        auto hdr=[&](uint8_t rc,uint8_t words){ append(o,rc); append(o,0); append(o,0x20); append(o,words); };
        switch(cmd) {
        case 0x01: hdr(0x05,0); break; // DeviceRequest
        case 0x02: hdr(0x06,0); break;
        case 0x03: case 0x04: hdr(0x07,0); break;
        case 0x80: { // 315-6149 bridge firmware upload
            // Mirror the Naomi bridge handshake. Payload checksum reply is sufficient for IDAS3;
            // firmware bytes are not executed by the static-recomp runtime.
            if (p && n >= 2 && p[1] == 0xFF) { hdr(0x07,0); break; }
            uint8_t sum=0; if (p) for (size_t i=0;i<std::min<size_t>(n,0x1C);++i) sum=(uint8_t)(sum+p[i]);
            hdr(0x80,1); append(o,sum); append(o,0); append(o,0); append(o,0);
            hdr(0x07,0);
            break;
        }
        case 0x84: // 315-6149 self test
            hdr(0x85,1); append(o,0); append(o,0); append(o,0); append(o,0); break;
        case 0x82: { // 315-6149 bridge ID, exactly as Flycast/Naomi bridge reports
            static const char a[]="315-6149    COPYRIGHT SEGA E";
            static const char b[]="NTERPRISES CO,LTD.  1998    ";
            hdr(0x83,7); o.insert(o.end(),a,a+28);
            hdr(0x83,5); o.insert(o.end(),b,b+28);
            break;
        }
        case 0x86:
            if (!p || !n) { hdr(0x87,0); break; }
            handle86(p,n,o); break;
        default: hdr(0xFD,0); break;
        }
        return o;
    }

private:
    void handle86(const uint8_t* p,size_t n,std::vector<uint8_t>& o) {
        uint8_t sub=p[0];
        auto hdr=[&](uint8_t rc,uint8_t words){ append(o,rc); append(o,0); append(o,0x20); append(o,words); };
        if (sub==0x01) {
            hdr(0x87,1); append(o,2); append(o,0); append(o,0); append(o,0);
        } else if (sub==0x03) {
            size_t addr=n>1?p[1]%eeprom.size():0;
            hdr(0x87,0x20); o.insert(o.end(),eeprom.begin()+addr,eeprom.end());
        } else if (sub==0x0B && n>=4) {
            size_t addr=p[1]%eeprom.size(), sz=std::min<size_t>(p[2],eeprom.size()-addr);
            if (4+sz<=n) std::memcpy(eeprom.data()+addr,p+4,sz);
            hdr(0x87,1); o.insert(o.end(),eeprom.begin(),eeprom.begin()+4);
        } else if (sub==0x31) {
            hdr(0x87,5);
            const uint8_t dip[]={0x32,0xff,0xff,0xff,0x00,0xff,0xf9,0xff};
            o.insert(o.end(),std::begin(dip),std::end(dip)); append32le(o,0); append32le(o,0); append32le(o,0);
        } else if (sub==0x13 && n>=3) {
            uint8_t node=p[1], len=p[2]; if(node>=1&&node<=32&&3+len<=n) repeat[node-1].assign(p+3,p+3+len);
            hdr(0x87,1); append(o,sub+1); append(o,0); append(o,len+1); append(o,0);
        } else if (sub==0x15 && n>=2) {
            uint8_t ch=p[1]&31; auto &r=rx[ch];
            const size_t headerLen = 23; // 5 dwords + 3 bytes, matching Naomi 315-6149 framing
            uint8_t words = r.empty()?0x05:(uint8_t)((r.size()+headerLen-1)/4+1);
            hdr(0x87, words);
            append(o, r.empty()?0x32:0x16); append(o,0xff); append(o,0xff); append(o,0xff);
            append32le(o,0xffffff00); append32le(o,0); append32le(o,0);
            if (r.empty()) {
                append32le(o,0);
            } else {
                append(o,0); append(o,ch); append(o,senseLine(r[0]));
                o.insert(o.end(),r.begin(),r.end());
                while ((o.size() & 3u) != 0) append(o,0);
                r.clear();
            }
        } else if ((sub==0x17||sub==0x19||sub==0x21||sub==0x33) && n>=8) {
            uint8_t ch=p[5]&31,node=p[6],len=p[7]; if(8+len<=n) queueJvs(node,ch,p+8,len,sub!=0x17,sub==0x19);
            hdr(0x87,1); append(o,0x18); append(o,ch); append(o, sub==0x17?0x8E:senseLine(node==0xFF?1:node)); append(o,0);
        } else {
            hdr(0xFD,0);
        }
    }
};

inline NaomiJvs13551& g_naomiJvs13551() { static NaomiJvs13551 b; return b; }
