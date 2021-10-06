#include <string>

using namespace std;

// convert hex string to 32-bit binary string, padding with leading zeros if needed
string hex2bin(string address)
{
    string new_address = "";

    if (address.length() < 8)
    {
        int offset = 8 - address.length();

        for (int i = 0; i < offset; i++)
        {
            new_address.append("0");
        }

        new_address.append(address);
    }
    else
    {
        new_address = address;
    }

    string binary;

    for (int i = 0; i < new_address.length(); i++)
    {
        switch (new_address[i])
        {
        case '0':
            binary.append("0000");
            break;

        case '1':
            binary.append("0001");
            break;

        case '2':
            binary.append("0010");
            break;

        case '3':
            binary.append("0011");
            break;

        case '4':
            binary.append("0100");
            break;

        case '5':
            binary.append("0101");
            break;

        case '6':
            binary.append("0110");
            break;

        case '7':
            binary.append("0111");
            break;

        case '8':
            binary.append("1000");
            break;

        case '9':
            binary.append("1001");
            break;

        case 'a':
            binary.append("1010");
            break;

        case 'b':
            binary.append("1011");
            break;

        case 'c':
            binary.append("1100");
            break;

        case 'd':
            binary.append("1101");
            break;

        case 'e':
            binary.append("1110");
            break;

        case 'f':
            binary.append("1111");
        
        default:
            break;
        }
    }

    return binary;
}