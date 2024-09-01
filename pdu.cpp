#include <iostream>
#include <string>
#include <stdint.h>
#include <bitset>
#include <vector>
std::string convert_semi_octet(const std::string& s) {
    std::string o;
    for (uint i = 0; i < s.size(); i+=2)
    {
        o += s[i+1];
        o += s[i];
    }

    return o;
}

std::string decodeGSM7bit(const std::string& encoded) {
    std::string decoded;
    int length = encoded.length() / 2;
    int bitOffset = 0;

    for (int i = 0; i < length; ++i) {
        int byteIndex = (i * 7) / 8;
        int shift = bitOffset % 8;

        uint8_t currentByte = std::stoi(encoded.substr(byteIndex * 2, 2), nullptr, 16);
        uint8_t nextByte = (byteIndex + 1 < encoded.length() / 2) ? std::stoi(encoded.substr((byteIndex + 1) * 2, 2), nullptr, 16) : 0;

        uint8_t septet = ((currentByte >> shift) | (nextByte << (8 - shift))) & 0x7F;

        decoded += static_cast<char>(septet);
        bitOffset += 7;
    }

    return decoded;
}

int hex_to_int(const std::string& s) {
    int result = 0;
    for (char c : s) {
        result *= 16;
        if (c >= '0' && c <= '9') {
            result += c - '0';
        } else if (c >= 'A' && c <= 'F') {
            result += c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            result += c - 'a' + 10;
        }
    }
    return result;
}

std::string hex_to_text(const std::string& s) {
    std::string result;
    for (size_t i = 0; i < s.length(); i += 2) {
        char c = static_cast<char>(hex_to_int(s.substr(i, 2)));
        if (c >= 32 && c < 127) {
            result += c;
        } else {
            result += '~';
        }
    }
    return result;
}

bool getBit(uint8_t byte, uint8_t bit) {
    return (byte & (1 << bit)) != 0;
}



int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <pdu>" << std::endl;
        return 1;
    }

    std::string pdu(argv[1]);

    std::string number;
    std::string text;
    std::string url;

    int i = 0; // index in pdu

    int SMSC_length = hex_to_int(pdu.substr(i, 2)) - 2; i += 2;
    int SMSC_number_type = hex_to_int(pdu.substr(i, 2)); i += 2;
    std::string SMSC = convert_semi_octet(pdu.substr(i, SMSC_length*2)); i += SMSC_length*2;
    i += 2; // skip F0

    // std::cout << "SMSC_length: " << SMSC_length << std::endl;
    // std::cout << "SMSC_number_type: " << SMSC_number_type << std::endl;
    // std::cout << "SMSC: " << SMSC << std::endl;

    // std::cout << "PDU_type: " << pdu.substr(i, 2) << std::endl;
    char PDU_type = hex_to_int(pdu.substr(i, 2)) - 2; i += 2;

    // std::cout << "PDU_type: " << int(PDU_type) << " " << std::bitset<8>(PDU_type).to_string() << std::endl;
    
    /*if(getBit(PDU_type, 7))
        std::cout << "TP-RP enabled" << std::endl;

    if(getBit(PDU_type, 6))
        std::cout << "TP-UDHI enabled" << std::endl;

    if(getBit(PDU_type, 5))
        std::cout << "TP-SRI enabled" << std::endl;
    
    if(getBit(PDU_type, 2))
        std::cout << "TP-MMS enabled" << std::endl;*/

    
    // std::cout << "TP-MTI: " << (int) getBit(PDU_type, 0) << (int) getBit(PDU_type, 1) << std::endl;


    int Adress_length = hex_to_int(pdu.substr(i, 2)) + 1; i += 2;
    int Adress_number_type = hex_to_int(pdu.substr(i, 2)); i += 2;
    std::string Adress = convert_semi_octet(pdu.substr(i, Adress_length)); i += Adress_length;
    Adress = Adress.substr(0, Adress.find('F'));

    // std::cout << "Adress_length: " << Adress_length << std::endl;
    // std::cout << "Adress_number_type: " << Adress_number_type << std::endl;
    // std::cout << "Adress: " << Adress << std::endl;

    char PID = hex_to_int(pdu.substr(i, 2)); i += 2;
    char DSC = hex_to_int(pdu.substr(i, 2)); i += 2;

    enum PDU_MODE {SMS, MMS, UNKNOWN};

    PDU_MODE mode = UNKNOWN;

    if(getBit(DSC, 3) == 0 && getBit(DSC, 2) == 0)
    {
        std::cout << "SMS mode" << std::endl;
        mode = SMS;
    }
    else if(getBit(DSC, 3) == 0 && getBit(DSC, 1) == 0)
    {
        std::cout << "MMS mode" << std::endl;
        mode = MMS;
    }

    // std::cout << "DSC: " << (int) getBit(DSC, 3) << (int) getBit(DSC, 2) << "  " << std::bitset<8>(DSC).to_string() << std::endl;

    i+= 7*2; // timestamp

    int Message_length = hex_to_int(pdu.substr(i, 2)) + 1; i += 2;
    std::string Message = pdu.substr(i, Message_length*2); i += Message_length*2;

    if(mode == SMS)
    {
        number = "+" + Adress;
        text = decodeGSM7bit(Message);
    }
    else
    {
        Message = hex_to_text(Message);
        number = Message.substr(Message.find("+"), Message.find("/") - Message.find("+"));
        url = Message.substr(Message.find("http"), Message.find("~", Message.find("http")) - Message.find("http"));
    }

    std::cout << "Number: " << number << std::endl;
    std::cout << "Text: " << text << std::endl;
    std::cout << "URL: " << url << std::endl;
}