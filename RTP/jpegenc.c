/*
 * jpegenc.c
 *
 *  Created on: Aug 29, 2014
 *      Author: jason
 */
#include "JpegRtpApi.h"

struct BIT
{
	unsigned char size;
	unsigned short int code;
};

static struct BIT s_lumin_dc[12] = {
         { 2, 0x0000}, /*               00 */
         { 3, 0x0002}, /*              010 */
         { 3, 0x0003}, /*              011 */
         { 3, 0x0004}, /*              100 */
         { 3, 0x0005}, /*              101 */
         { 3, 0x0006}, /*              110 */
         { 4, 0x000e}, /*             1110 */
         { 5, 0x001e}, /*            11110 */
         { 6, 0x003e}, /*           111110 */
         { 7, 0x007e}, /*          1111110 */
         { 8, 0x00fe}, /*         11111110 */
         { 9, 0x01fe}  /*        111111110 */
};

static struct BIT s_chrom_dc[12] = {
         { 2, 0x0000}, /*               00 */
         { 2, 0x0001}, /*               01 */
         { 2, 0x0002}, /*               10 */
         { 3, 0x0006}, /*              110 */
         { 4, 0x000e}, /*             1110 */
         { 5, 0x001e}, /*            11110 */
         { 6, 0x003e}, /*           111110 */
         { 7, 0x007e}, /*          1111110 */
         { 8, 0x00fe}, /*         11111110 */
         { 9, 0x01fe}, /*        111111110 */
         {10, 0x03fe}, /*       1111111110 */
         {11, 0x07fe}  /*      11111111110 */
};

/* 162 valid entries */
static struct BIT s_lumin_ac[256] = {

         { 4, 0x000a}, /* 0x00:              1010 */
         { 2, 0x0000}, /* 0x01:                00 */
         { 2, 0x0001}, /* 0x02:                01 */
         { 3, 0x0004}, /* 0x03:               100 */
         { 4, 0x000b}, /* 0x04:              1011 */
         { 5, 0x001a}, /* 0x05:             11010 */
         { 7, 0x0078}, /* 0x06:           1111000 */
         { 8, 0x00f8}, /* 0x07:          11111000 */
         {10, 0x03f6}, /* 0x08:        1111110110 */
         {16, 0xff82}, /* 0x09:  1111111110000010 */
         {16, 0xff83}, /* 0x0a:  1111111110000011 */
         { 0, 0x0000}, /* 0x0b:                   */
         { 0, 0x0000}, /* 0x0c:                   */
         { 0, 0x0000}, /* 0x0d:                   */
         { 0, 0x0000}, /* 0x0e:                   */
         { 0, 0x0000}, /* 0x0f:                   */
         { 0, 0x0000}, /* 0x10:                   */
         { 4, 0x000c}, /* 0x11:              1100 */
         { 5, 0x001b}, /* 0x12:             11011 */
         { 7, 0x0079}, /* 0x13:           1111001 */
         { 9, 0x01f6}, /* 0x14:         111110110 */
         {11, 0x07f6}, /* 0x15:       11111110110 */
         {16, 0xff84}, /* 0x16:  1111111110000100 */
         {16, 0xff85}, /* 0x17:  1111111110000101 */
         {16, 0xff86}, /* 0x18:  1111111110000110 */
         {16, 0xff87}, /* 0x19:  1111111110000111 */
         {16, 0xff88}, /* 0x1a:  1111111110001000 */
         { 0, 0x0000}, /* 0x1b:                   */
         { 0, 0x0000}, /* 0x1c:                   */
         { 0, 0x0000}, /* 0x1d:                   */
         { 0, 0x0000}, /* 0x1e:                   */
         { 0, 0x0000}, /* 0x1f:                   */
         { 0, 0x0000}, /* 0x20:                   */
         { 5, 0x001c}, /* 0x21:             11100 */
         { 8, 0x00f9}, /* 0x22:          11111001 */
         {10, 0x03f7}, /* 0x23:        1111110111 */
         {12, 0x0ff4}, /* 0x24:      111111110100 */
         {16, 0xff89}, /* 0x25:  1111111110001001 */
         {16, 0xff8a}, /* 0x26:  1111111110001010 */
         {16, 0xff8b}, /* 0x27:  1111111110001011 */
         {16, 0xff8c}, /* 0x28:  1111111110001100 */
         {16, 0xff8d}, /* 0x29:  1111111110001101 */
         {16, 0xff8e}, /* 0x2a:  1111111110001110 */
         { 0, 0x0000}, /* 0x2b:                   */
         { 0, 0x0000}, /* 0x2c:                   */
         { 0, 0x0000}, /* 0x2d:                   */
         { 0, 0x0000}, /* 0x2e:                   */
         { 0, 0x0000}, /* 0x2f:                   */
         { 0, 0x0000}, /* 0x30:                   */
         { 6, 0x003a}, /* 0x31:            111010 */
         { 9, 0x01f7}, /* 0x32:         111110111 */
         {12, 0x0ff5}, /* 0x33:      111111110101 */
         {16, 0xff8f}, /* 0x34:  1111111110001111 */
         {16, 0xff90}, /* 0x35:  1111111110010000 */
         {16, 0xff91}, /* 0x36:  1111111110010001 */
         {16, 0xff92}, /* 0x37:  1111111110010010 */
         {16, 0xff93}, /* 0x38:  1111111110010011 */
         {16, 0xff94}, /* 0x39:  1111111110010100 */
         {16, 0xff95}, /* 0x3a:  1111111110010101 */
         { 0, 0x0000}, /* 0x3b:                   */
         { 0, 0x0000}, /* 0x3c:                   */
         { 0, 0x0000}, /* 0x3d:                   */
         { 0, 0x0000}, /* 0x3e:                   */
         { 0, 0x0000}, /* 0x3f:                   */
         { 0, 0x0000}, /* 0x40:                   */
         { 6, 0x003b}, /* 0x41:            111011 */
         {10, 0x03f8}, /* 0x42:        1111111000 */
         {16, 0xff96}, /* 0x43:  1111111110010110 */
         {16, 0xff97}, /* 0x44:  1111111110010111 */
         {16, 0xff98}, /* 0x45:  1111111110011000 */
         {16, 0xff99}, /* 0x46:  1111111110011001 */
         {16, 0xff9a}, /* 0x47:  1111111110011010 */
         {16, 0xff9b}, /* 0x48:  1111111110011011 */
         {16, 0xff9c}, /* 0x49:  1111111110011100 */
         {16, 0xff9d}, /* 0x4a:  1111111110011101 */
         { 0, 0x0000}, /* 0x4b:                   */
         { 0, 0x0000}, /* 0x4c:                   */
         { 0, 0x0000}, /* 0x4d:                   */
         { 0, 0x0000}, /* 0x4e:                   */
         { 0, 0x0000}, /* 0x4f:                   */
         { 0, 0x0000}, /* 0x50:                   */
         { 7, 0x007a}, /* 0x51:           1111010 */
         {11, 0x07f7}, /* 0x52:       11111110111 */
         {16, 0xff9e}, /* 0x53:  1111111110011110 */
         {16, 0xff9f}, /* 0x54:  1111111110011111 */
         {16, 0xffa0}, /* 0x55:  1111111110100000 */
         {16, 0xffa1}, /* 0x56:  1111111110100001 */
         {16, 0xffa2}, /* 0x57:  1111111110100010 */
         {16, 0xffa3}, /* 0x58:  1111111110100011 */
         {16, 0xffa4}, /* 0x59:  1111111110100100 */
         {16, 0xffa5}, /* 0x5a:  1111111110100101 */
         { 0, 0x0000}, /* 0x5b:                   */
         { 0, 0x0000}, /* 0x5c:                   */
         { 0, 0x0000}, /* 0x5d:                   */
         { 0, 0x0000}, /* 0x5e:                   */
         { 0, 0x0000}, /* 0x5f:                   */
         { 0, 0x0000}, /* 0x60:                   */
         { 7, 0x007b}, /* 0x61:           1111011 */
         {12, 0x0ff6}, /* 0x62:      111111110110 */
         {16, 0xffa6}, /* 0x63:  1111111110100110 */
         {16, 0xffa7}, /* 0x64:  1111111110100111 */
         {16, 0xffa8}, /* 0x65:  1111111110101000 */
         {16, 0xffa9}, /* 0x66:  1111111110101001 */
         {16, 0xffaa}, /* 0x67:  1111111110101010 */
         {16, 0xffab}, /* 0x68:  1111111110101011 */
         {16, 0xffac}, /* 0x69:  1111111110101100 */
         {16, 0xffad}, /* 0x6a:  1111111110101101 */
         { 0, 0x0000}, /* 0x6b:                   */
         { 0, 0x0000}, /* 0x6c:                   */
         { 0, 0x0000}, /* 0x6d:                   */
         { 0, 0x0000}, /* 0x6e:                   */
         { 0, 0x0000}, /* 0x6f:                   */
         { 0, 0x0000}, /* 0x70:                   */
         { 8, 0x00fa}, /* 0x71:          11111010 */
         {12, 0x0ff7}, /* 0x72:      111111110111 */
         {16, 0xffae}, /* 0x73:  1111111110101110 */
         {16, 0xffaf}, /* 0x74:  1111111110101111 */
         {16, 0xffb0}, /* 0x75:  1111111110110000 */
         {16, 0xffb1}, /* 0x76:  1111111110110001 */
         {16, 0xffb2}, /* 0x77:  1111111110110010 */
         {16, 0xffb3}, /* 0x78:  1111111110110011 */
         {16, 0xffb4}, /* 0x79:  1111111110110100 */
         {16, 0xffb5}, /* 0x7a:  1111111110110101 */
         { 0, 0x0000}, /* 0x7b:                   */
         { 0, 0x0000}, /* 0x7c:                   */
         { 0, 0x0000}, /* 0x7d:                   */
         { 0, 0x0000}, /* 0x7e:                   */
         { 0, 0x0000}, /* 0x7f:                   */
         { 0, 0x0000}, /* 0x80:                   */
         { 9, 0x01f8}, /* 0x81:         111111000 */
         {15, 0x7fc0}, /* 0x82:   111111111000000 */
         {16, 0xffb6}, /* 0x83:  1111111110110110 */
         {16, 0xffb7}, /* 0x84:  1111111110110111 */
         {16, 0xffb8}, /* 0x85:  1111111110111000 */
         {16, 0xffb9}, /* 0x86:  1111111110111001 */
         {16, 0xffba}, /* 0x87:  1111111110111010 */
         {16, 0xffbb}, /* 0x88:  1111111110111011 */
         {16, 0xffbc}, /* 0x89:  1111111110111100 */
         {16, 0xffbd}, /* 0x8a:  1111111110111101 */
         { 0, 0x0000}, /* 0x8b:                   */
         { 0, 0x0000}, /* 0x8c:                   */
         { 0, 0x0000}, /* 0x8d:                   */
         { 0, 0x0000}, /* 0x8e:                   */
         { 0, 0x0000}, /* 0x8f:                   */
         { 0, 0x0000}, /* 0x90:                   */
         { 9, 0x01f9}, /* 0x91:         111111001 */
         {16, 0xffbe}, /* 0x92:  1111111110111110 */
         {16, 0xffbf}, /* 0x93:  1111111110111111 */
         {16, 0xffc0}, /* 0x94:  1111111111000000 */
         {16, 0xffc1}, /* 0x95:  1111111111000001 */
         {16, 0xffc2}, /* 0x96:  1111111111000010 */
         {16, 0xffc3}, /* 0x97:  1111111111000011 */
         {16, 0xffc4}, /* 0x98:  1111111111000100 */
         {16, 0xffc5}, /* 0x99:  1111111111000101 */
         {16, 0xffc6}, /* 0x9a:  1111111111000110 */
         { 0, 0x0000}, /* 0x9b:                   */
         { 0, 0x0000}, /* 0x9c:                   */
         { 0, 0x0000}, /* 0x9d:                   */
         { 0, 0x0000}, /* 0x9e:                   */
         { 0, 0x0000}, /* 0x9f:                   */
         { 0, 0x0000}, /* 0xa0:                   */
         { 9, 0x01fa}, /* 0xa1:         111111010 */
         {16, 0xffc7}, /* 0xa2:  1111111111000111 */
         {16, 0xffc8}, /* 0xa3:  1111111111001000 */
         {16, 0xffc9}, /* 0xa4:  1111111111001001 */
         {16, 0xffca}, /* 0xa5:  1111111111001010 */
         {16, 0xffcb}, /* 0xa6:  1111111111001011 */
         {16, 0xffcc}, /* 0xa7:  1111111111001100 */
         {16, 0xffcd}, /* 0xa8:  1111111111001101 */
         {16, 0xffce}, /* 0xa9:  1111111111001110 */
         {16, 0xffcf}, /* 0xaa:  1111111111001111 */
         { 0, 0x0000}, /* 0xab:                   */
         { 0, 0x0000}, /* 0xac:                   */
         { 0, 0x0000}, /* 0xad:                   */
         { 0, 0x0000}, /* 0xae:                   */
         { 0, 0x0000}, /* 0xaf:                   */
         { 0, 0x0000}, /* 0xb0:                   */
         {10, 0x03f9}, /* 0xb1:        1111111001 */
         {16, 0xffd0}, /* 0xb2:  1111111111010000 */
         {16, 0xffd1}, /* 0xb3:  1111111111010001 */
         {16, 0xffd2}, /* 0xb4:  1111111111010010 */
         {16, 0xffd3}, /* 0xb5:  1111111111010011 */
         {16, 0xffd4}, /* 0xb6:  1111111111010100 */
         {16, 0xffd5}, /* 0xb7:  1111111111010101 */
         {16, 0xffd6}, /* 0xb8:  1111111111010110 */
         {16, 0xffd7}, /* 0xb9:  1111111111010111 */
         {16, 0xffd8}, /* 0xba:  1111111111011000 */
         { 0, 0x0000}, /* 0xbb:                   */
         { 0, 0x0000}, /* 0xbc:                   */
         { 0, 0x0000}, /* 0xbd:                   */
         { 0, 0x0000}, /* 0xbe:                   */
         { 0, 0x0000}, /* 0xbf:                   */
         { 0, 0x0000}, /* 0xc0:                   */
         {10, 0x03fa}, /* 0xc1:        1111111010 */
         {16, 0xffd9}, /* 0xc2:  1111111111011001 */
         {16, 0xffda}, /* 0xc3:  1111111111011010 */
         {16, 0xffdb}, /* 0xc4:  1111111111011011 */
         {16, 0xffdc}, /* 0xc5:  1111111111011100 */
         {16, 0xffdd}, /* 0xc6:  1111111111011101 */
         {16, 0xffde}, /* 0xc7:  1111111111011110 */
         {16, 0xffdf}, /* 0xc8:  1111111111011111 */
         {16, 0xffe0}, /* 0xc9:  1111111111100000 */
         {16, 0xffe1}, /* 0xca:  1111111111100001 */
         { 0, 0x0000}, /* 0xcb:                   */
         { 0, 0x0000}, /* 0xcc:                   */
         { 0, 0x0000}, /* 0xcd:                   */
         { 0, 0x0000}, /* 0xce:                   */
         { 0, 0x0000}, /* 0xcf:                   */
         { 0, 0x0000}, /* 0xd0:                   */
         {11, 0x07f8}, /* 0xd1:       11111111000 */
         {16, 0xffe2}, /* 0xd2:  1111111111100010 */
         {16, 0xffe3}, /* 0xd3:  1111111111100011 */
         {16, 0xffe4}, /* 0xd4:  1111111111100100 */
         {16, 0xffe5}, /* 0xd5:  1111111111100101 */
         {16, 0xffe6}, /* 0xd6:  1111111111100110 */
         {16, 0xffe7}, /* 0xd7:  1111111111100111 */
         {16, 0xffe8}, /* 0xd8:  1111111111101000 */
         {16, 0xffe9}, /* 0xd9:  1111111111101001 */
         {16, 0xffea}, /* 0xda:  1111111111101010 */
         { 0, 0x0000}, /* 0xdb:                   */
         { 0, 0x0000}, /* 0xdc:                   */
         { 0, 0x0000}, /* 0xdd:                   */
         { 0, 0x0000}, /* 0xde:                   */
         { 0, 0x0000}, /* 0xdf:                   */
         { 0, 0x0000}, /* 0xe0:                   */
         {16, 0xffeb}, /* 0xe1:  1111111111101011 */
         {16, 0xffec}, /* 0xe2:  1111111111101100 */
         {16, 0xffed}, /* 0xe3:  1111111111101101 */
         {16, 0xffee}, /* 0xe4:  1111111111101110 */
         {16, 0xffef}, /* 0xe5:  1111111111101111 */
         {16, 0xfff0}, /* 0xe6:  1111111111110000 */
         {16, 0xfff1}, /* 0xe7:  1111111111110001 */
         {16, 0xfff2}, /* 0xe8:  1111111111110010 */
         {16, 0xfff3}, /* 0xe9:  1111111111110011 */
         {16, 0xfff4}, /* 0xea:  1111111111110100 */
         { 0, 0x0000}, /* 0xeb:                   */
         { 0, 0x0000}, /* 0xec:                   */
         { 0, 0x0000}, /* 0xed:                   */
         { 0, 0x0000}, /* 0xee:                   */
         { 0, 0x0000}, /* 0xef:                   */
         {11, 0x07f9}, /* 0xf0:       11111111001 */
         {16, 0xfff5}, /* 0xf1:  1111111111110101 */
         {16, 0xfff6}, /* 0xf2:  1111111111110110 */
         {16, 0xfff7}, /* 0xf3:  1111111111110111 */
         {16, 0xfff8}, /* 0xf4:  1111111111111000 */
         {16, 0xfff9}, /* 0xf5:  1111111111111001 */
         {16, 0xfffa}, /* 0xf6:  1111111111111010 */
         {16, 0xfffb}, /* 0xf7:  1111111111111011 */
         {16, 0xfffc}, /* 0xf8:  1111111111111100 */
         {16, 0xfffd}, /* 0xf9:  1111111111111101 */
         {16, 0xfffe}, /* 0xfa:  1111111111111110 */
         { 0, 0x0000}, /* 0xfb:                   */
         { 0, 0x0000}, /* 0xfc:                   */
         { 0, 0x0000}, /* 0xfd:                   */
         { 0, 0x0000}, /* 0xfe:                   */
         { 0, 0x0000}, /* 0xff:                   */
};

/* 162 valid entries */
static struct BIT s_chrom_ac[256] = {

         { 2, 0x0000}, /* 0x00:                00 */
         { 2, 0x0001}, /* 0x01:                01 */
         { 3, 0x0004}, /* 0x02:               100 */
         { 4, 0x000a}, /* 0x03:              1010 */
         { 5, 0x0018}, /* 0x04:             11000 */
         { 5, 0x0019}, /* 0x05:             11001 */
         { 6, 0x0038}, /* 0x06:            111000 */
         { 7, 0x0078}, /* 0x07:           1111000 */
         { 9, 0x01f4}, /* 0x08:         111110100 */
         {10, 0x03f6}, /* 0x09:        1111110110 */
         {12, 0x0ff4}, /* 0x0a:      111111110100 */
         { 0, 0x0000}, /* 0x0b:                   */
         { 0, 0x0000}, /* 0x0c:                   */
         { 0, 0x0000}, /* 0x0d:                   */
         { 0, 0x0000}, /* 0x0e:                   */
         { 0, 0x0000}, /* 0x0f:                   */
         { 0, 0x0000}, /* 0x10:                   */
         { 4, 0x000b}, /* 0x11:              1011 */
         { 6, 0x0039}, /* 0x12:            111001 */
         { 8, 0x00f6}, /* 0x13:          11110110 */
         { 9, 0x01f5}, /* 0x14:         111110101 */
         {11, 0x07f6}, /* 0x15:       11111110110 */
         {12, 0x0ff5}, /* 0x16:      111111110101 */
         {16, 0xff88}, /* 0x17:  1111111110001000 */
         {16, 0xff89}, /* 0x18:  1111111110001001 */
         {16, 0xff8a}, /* 0x19:  1111111110001010 */
         {16, 0xff8b}, /* 0x1a:  1111111110001011 */
         { 0, 0x0000}, /* 0x1b:                   */
         { 0, 0x0000}, /* 0x1c:                   */
         { 0, 0x0000}, /* 0x1d:                   */
         { 0, 0x0000}, /* 0x1e:                   */
         { 0, 0x0000}, /* 0x1f:                   */
         { 0, 0x0000}, /* 0x20:                   */
         { 5, 0x001a}, /* 0x21:             11010 */
         { 8, 0x00f7}, /* 0x22:          11110111 */
         {10, 0x03f7}, /* 0x23:        1111110111 */
         {12, 0x0ff6}, /* 0x24:      111111110110 */
         {15, 0x7fc2}, /* 0x25:   111111111000010 */
         {16, 0xff8c}, /* 0x26:  1111111110001100 */
         {16, 0xff8d}, /* 0x27:  1111111110001101 */
         {16, 0xff8e}, /* 0x28:  1111111110001110 */
         {16, 0xff8f}, /* 0x29:  1111111110001111 */
         {16, 0xff90}, /* 0x2a:  1111111110010000 */
         { 0, 0x0000}, /* 0x2b:                   */
         { 0, 0x0000}, /* 0x2c:                   */
         { 0, 0x0000}, /* 0x2d:                   */
         { 0, 0x0000}, /* 0x2e:                   */
         { 0, 0x0000}, /* 0x2f:                   */
         { 0, 0x0000}, /* 0x30:                   */
         { 5, 0x001b}, /* 0x31:             11011 */
         { 8, 0x00f8}, /* 0x32:          11111000 */
         {10, 0x03f8}, /* 0x33:        1111111000 */
         {12, 0x0ff7}, /* 0x34:      111111110111 */
         {16, 0xff91}, /* 0x35:  1111111110010001 */
         {16, 0xff92}, /* 0x36:  1111111110010010 */
         {16, 0xff93}, /* 0x37:  1111111110010011 */
         {16, 0xff94}, /* 0x38:  1111111110010100 */
         {16, 0xff95}, /* 0x39:  1111111110010101 */
         {16, 0xff96}, /* 0x3a:  1111111110010110 */
         { 0, 0x0000}, /* 0x3b:                   */
         { 0, 0x0000}, /* 0x3c:                   */
         { 0, 0x0000}, /* 0x3d:                   */
         { 0, 0x0000}, /* 0x3e:                   */
         { 0, 0x0000}, /* 0x3f:                   */
         { 0, 0x0000}, /* 0x40:                   */
         { 6, 0x003a}, /* 0x41:            111010 */
         { 9, 0x01f6}, /* 0x42:         111110110 */
         {16, 0xff97}, /* 0x43:  1111111110010111 */
         {16, 0xff98}, /* 0x44:  1111111110011000 */
         {16, 0xff99}, /* 0x45:  1111111110011001 */
         {16, 0xff9a}, /* 0x46:  1111111110011010 */
         {16, 0xff9b}, /* 0x47:  1111111110011011 */
         {16, 0xff9c}, /* 0x48:  1111111110011100 */
         {16, 0xff9d}, /* 0x49:  1111111110011101 */
         {16, 0xff9e}, /* 0x4a:  1111111110011110 */
         { 0, 0x0000}, /* 0x4b:                   */
         { 0, 0x0000}, /* 0x4c:                   */
         { 0, 0x0000}, /* 0x4d:                   */
         { 0, 0x0000}, /* 0x4e:                   */
         { 0, 0x0000}, /* 0x4f:                   */
         { 0, 0x0000}, /* 0x50:                   */
         { 6, 0x003b}, /* 0x51:            111011 */
         {10, 0x03f9}, /* 0x52:        1111111001 */
         {16, 0xff9f}, /* 0x53:  1111111110011111 */
         {16, 0xffa0}, /* 0x54:  1111111110100000 */
         {16, 0xffa1}, /* 0x55:  1111111110100001 */
         {16, 0xffa2}, /* 0x56:  1111111110100010 */
         {16, 0xffa3}, /* 0x57:  1111111110100011 */
         {16, 0xffa4}, /* 0x58:  1111111110100100 */
         {16, 0xffa5}, /* 0x59:  1111111110100101 */
         {16, 0xffa6}, /* 0x5a:  1111111110100110 */
         { 0, 0x0000}, /* 0x5b:                   */
         { 0, 0x0000}, /* 0x5c:                   */
         { 0, 0x0000}, /* 0x5d:                   */
         { 0, 0x0000}, /* 0x5e:                   */
         { 0, 0x0000}, /* 0x5f:                   */
         { 0, 0x0000}, /* 0x60:                   */
         { 7, 0x0079}, /* 0x61:           1111001 */
         {11, 0x07f7}, /* 0x62:       11111110111 */
         {16, 0xffa7}, /* 0x63:  1111111110100111 */
         {16, 0xffa8}, /* 0x64:  1111111110101000 */
         {16, 0xffa9}, /* 0x65:  1111111110101001 */
         {16, 0xffaa}, /* 0x66:  1111111110101010 */
         {16, 0xffab}, /* 0x67:  1111111110101011 */
         {16, 0xffac}, /* 0x68:  1111111110101100 */
         {16, 0xffad}, /* 0x69:  1111111110101101 */
         {16, 0xffae}, /* 0x6a:  1111111110101110 */
         { 0, 0x0000}, /* 0x6b:                   */
         { 0, 0x0000}, /* 0x6c:                   */
         { 0, 0x0000}, /* 0x6d:                   */
         { 0, 0x0000}, /* 0x6e:                   */
         { 0, 0x0000}, /* 0x6f:                   */
         { 0, 0x0000}, /* 0x70:                   */
         { 7, 0x007a}, /* 0x71:           1111010 */
         {11, 0x07f8}, /* 0x72:       11111111000 */
         {16, 0xffaf}, /* 0x73:  1111111110101111 */
         {16, 0xffb0}, /* 0x74:  1111111110110000 */
         {16, 0xffb1}, /* 0x75:  1111111110110001 */
         {16, 0xffb2}, /* 0x76:  1111111110110010 */
         {16, 0xffb3}, /* 0x77:  1111111110110011 */
         {16, 0xffb4}, /* 0x78:  1111111110110100 */
         {16, 0xffb5}, /* 0x79:  1111111110110101 */
         {16, 0xffb6}, /* 0x7a:  1111111110110110 */
         { 0, 0x0000}, /* 0x7b:                   */
         { 0, 0x0000}, /* 0x7c:                   */
         { 0, 0x0000}, /* 0x7d:                   */
         { 0, 0x0000}, /* 0x7e:                   */
         { 0, 0x0000}, /* 0x7f:                   */
         { 0, 0x0000}, /* 0x80:                   */
         { 8, 0x00f9}, /* 0x81:          11111001 */
         {16, 0xffb7}, /* 0x82:  1111111110110111 */
         {16, 0xffb8}, /* 0x83:  1111111110111000 */
         {16, 0xffb9}, /* 0x84:  1111111110111001 */
         {16, 0xffba}, /* 0x85:  1111111110111010 */
         {16, 0xffbb}, /* 0x86:  1111111110111011 */
         {16, 0xffbc}, /* 0x87:  1111111110111100 */
         {16, 0xffbd}, /* 0x88:  1111111110111101 */
         {16, 0xffbe}, /* 0x89:  1111111110111110 */
         {16, 0xffbf}, /* 0x8a:  1111111110111111 */
         { 0, 0x0000}, /* 0x8b:                   */
         { 0, 0x0000}, /* 0x8c:                   */
         { 0, 0x0000}, /* 0x8d:                   */
         { 0, 0x0000}, /* 0x8e:                   */
         { 0, 0x0000}, /* 0x8f:                   */
         { 0, 0x0000}, /* 0x90:                   */
         { 9, 0x01f7}, /* 0x91:         111110111 */
         {16, 0xffc0}, /* 0x92:  1111111111000000 */
         {16, 0xffc1}, /* 0x93:  1111111111000001 */
         {16, 0xffc2}, /* 0x94:  1111111111000010 */
         {16, 0xffc3}, /* 0x95:  1111111111000011 */
         {16, 0xffc4}, /* 0x96:  1111111111000100 */
         {16, 0xffc5}, /* 0x97:  1111111111000101 */
         {16, 0xffc6}, /* 0x98:  1111111111000110 */
         {16, 0xffc7}, /* 0x99:  1111111111000111 */
         {16, 0xffc8}, /* 0x9a:  1111111111001000 */
         { 0, 0x0000}, /* 0x9b:                   */
         { 0, 0x0000}, /* 0x9c:                   */
         { 0, 0x0000}, /* 0x9d:                   */
         { 0, 0x0000}, /* 0x9e:                   */
         { 0, 0x0000}, /* 0x9f:                   */
         { 0, 0x0000}, /* 0xa0:                   */
         { 9, 0x01f8}, /* 0xa1:         111111000 */
         {16, 0xffc9}, /* 0xa2:  1111111111001001 */
         {16, 0xffca}, /* 0xa3:  1111111111001010 */
         {16, 0xffcb}, /* 0xa4:  1111111111001011 */
         {16, 0xffcc}, /* 0xa5:  1111111111001100 */
         {16, 0xffcd}, /* 0xa6:  1111111111001101 */
         {16, 0xffce}, /* 0xa7:  1111111111001110 */
         {16, 0xffcf}, /* 0xa8:  1111111111001111 */
         {16, 0xffd0}, /* 0xa9:  1111111111010000 */
         {16, 0xffd1}, /* 0xaa:  1111111111010001 */
         { 0, 0x0000}, /* 0xab:                   */
         { 0, 0x0000}, /* 0xac:                   */
         { 0, 0x0000}, /* 0xad:                   */
         { 0, 0x0000}, /* 0xae:                   */
         { 0, 0x0000}, /* 0xaf:                   */
         { 0, 0x0000}, /* 0xb0:                   */
         { 9, 0x01f9}, /* 0xb1:         111111001 */
         {16, 0xffd2}, /* 0xb2:  1111111111010010 */
         {16, 0xffd3}, /* 0xb3:  1111111111010011 */
         {16, 0xffd4}, /* 0xb4:  1111111111010100 */
         {16, 0xffd5}, /* 0xb5:  1111111111010101 */
         {16, 0xffd6}, /* 0xb6:  1111111111010110 */
         {16, 0xffd7}, /* 0xb7:  1111111111010111 */
         {16, 0xffd8}, /* 0xb8:  1111111111011000 */
         {16, 0xffd9}, /* 0xb9:  1111111111011001 */
         {16, 0xffda}, /* 0xba:  1111111111011010 */
         { 0, 0x0000}, /* 0xbb:                   */
         { 0, 0x0000}, /* 0xbc:                   */
         { 0, 0x0000}, /* 0xbd:                   */
         { 0, 0x0000}, /* 0xbe:                   */
         { 0, 0x0000}, /* 0xbf:                   */
         { 0, 0x0000}, /* 0xc0:                   */
         { 9, 0x01fa}, /* 0xc1:         111111010 */
         {16, 0xffdb}, /* 0xc2:  1111111111011011 */
         {16, 0xffdc}, /* 0xc3:  1111111111011100 */
         {16, 0xffdd}, /* 0xc4:  1111111111011101 */
         {16, 0xffde}, /* 0xc5:  1111111111011110 */
         {16, 0xffdf}, /* 0xc6:  1111111111011111 */
         {16, 0xffe0}, /* 0xc7:  1111111111100000 */
         {16, 0xffe1}, /* 0xc8:  1111111111100001 */
         {16, 0xffe2}, /* 0xc9:  1111111111100010 */
         {16, 0xffe3}, /* 0xca:  1111111111100011 */
         { 0, 0x0000}, /* 0xcb:                   */
         { 0, 0x0000}, /* 0xcc:                   */
         { 0, 0x0000}, /* 0xcd:                   */
         { 0, 0x0000}, /* 0xce:                   */
         { 0, 0x0000}, /* 0xcf:                   */
         { 0, 0x0000}, /* 0xd0:                   */
         {11, 0x07f9}, /* 0xd1:       11111111001 */
         {16, 0xffe4}, /* 0xd2:  1111111111100100 */
         {16, 0xffe5}, /* 0xd3:  1111111111100101 */
         {16, 0xffe6}, /* 0xd4:  1111111111100110 */
         {16, 0xffe7}, /* 0xd5:  1111111111100111 */
         {16, 0xffe8}, /* 0xd6:  1111111111101000 */
         {16, 0xffe9}, /* 0xd7:  1111111111101001 */
         {16, 0xffea}, /* 0xd8:  1111111111101010 */
         {16, 0xffeb}, /* 0xd9:  1111111111101011 */
         {16, 0xffec}, /* 0xda:  1111111111101100 */
         { 0, 0x0000}, /* 0xdb:                   */
         { 0, 0x0000}, /* 0xdc:                   */
         { 0, 0x0000}, /* 0xdd:                   */
         { 0, 0x0000}, /* 0xde:                   */
         { 0, 0x0000}, /* 0xdf:                   */
         { 0, 0x0000}, /* 0xe0:                   */
         {14, 0x3fe0}, /* 0xe1:    11111111100000 */
         {16, 0xffed}, /* 0xe2:  1111111111101101 */
         {16, 0xffee}, /* 0xe3:  1111111111101110 */
         {16, 0xffef}, /* 0xe4:  1111111111101111 */
         {16, 0xfff0}, /* 0xe5:  1111111111110000 */
         {16, 0xfff1}, /* 0xe6:  1111111111110001 */
         {16, 0xfff2}, /* 0xe7:  1111111111110010 */
         {16, 0xfff3}, /* 0xe8:  1111111111110011 */
         {16, 0xfff4}, /* 0xe9:  1111111111110100 */
         {16, 0xfff5}, /* 0xea:  1111111111110101 */
         { 0, 0x0000}, /* 0xeb:                   */
         { 0, 0x0000}, /* 0xec:                   */
         { 0, 0x0000}, /* 0xed:                   */
         { 0, 0x0000}, /* 0xee:                   */
         { 0, 0x0000}, /* 0xef:                   */
         {10, 0x03fa}, /* 0xf0:        1111111010 */
         {15, 0x7fc3}, /* 0xf1:   111111111000011 */
         {16, 0xfff6}, /* 0xf2:  1111111111110110 */
         {16, 0xfff7}, /* 0xf3:  1111111111110111 */
         {16, 0xfff8}, /* 0xf4:  1111111111111000 */
         {16, 0xfff9}, /* 0xf5:  1111111111111001 */
         {16, 0xfffa}, /* 0xf6:  1111111111111010 */
         {16, 0xfffb}, /* 0xf7:  1111111111111011 */
         {16, 0xfffc}, /* 0xf8:  1111111111111100 */
         {16, 0xfffd}, /* 0xf9:  1111111111111101 */
         {16, 0xfffe}, /* 0xfa:  1111111111111110 */
         { 0, 0x0000}, /* 0xfb:                   */
         { 0, 0x0000}, /* 0xfc:                   */
         { 0, 0x0000}, /* 0xfd:                   */
         { 0, 0x0000}, /* 0xfe:                   */
         { 0, 0x0000}  /* 0xff:                   */
};
unsigned char jpeg_luma_quantizer[64] = {
        16, 11, 10, 16, 24, 40, 51, 61,
        12, 12, 14, 19, 26, 58, 60, 55,
        14, 13, 16, 24, 40, 57, 69, 56,
        14, 17, 22, 29, 51, 87, 80, 62,
        18, 22, 37, 56, 68, 109, 103, 77,
        24, 35, 55, 64, 81, 104, 113, 92,
        49, 64, 78, 87, 103, 121, 120, 101,
        72, 92, 95, 98, 112, 100, 103, 99
};

unsigned char jpeg_chroma_quantizer[64] = {
        17, 18, 24, 47, 99, 99, 99, 99,
        18, 21, 26, 66, 99, 99, 99, 99,
        24, 26, 56, 99, 99, 99, 99, 99,
        47, 66, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99
};

#ifdef SPEEDFIRST
static struct BIT VLITABLE[0xFFFF];
static struct BIT *vliptr=VLITABLE+32768;
#endif

static unsigned short int YQT[64];
static unsigned short int CbCrQT[64];
static unsigned char zigzag[64]={ 0,   1,  5,  6, 14, 15, 27, 28,
		                          2,   4,  7, 13, 16, 26, 29, 42,
		                          3,   8, 12, 17, 25, 30, 41, 43,
		                          9,  11, 18, 24, 31, 40, 44, 53,
		                          10, 19, 23, 32, 39, 45, 52, 54,
		                          20, 22, 33, 38, 46, 51, 55, 60,
		                          21, 34, 37, 47, 50, 56, 59, 61,
		                          35, 36, 48, 49, 57, 58, 62, 63};



static void QuantizeTableInit(unsigned char Q, unsigned char *lqt, unsigned char *cqt)
{
  int i;

  if (Q < 1)	Q = 1;
  if (Q > 99)	Q = 99;
  if (Q < 50)
	  Q = 5000 /Q;
  else
	  Q = 200-2*Q;

  for (i=0; i < 64; i++)
  {
	  int lq = (jpeg_luma_quantizer[i] * Q + 50) / 100;
	  int cq = (jpeg_chroma_quantizer[i] * Q + 50) / 100;
/* Limit the quantizers to 1 <= q <= 255 */
	  if (lq < 1) lq = 1;
	  else if (lq > 255) lq = 255;
	  lqt[zigzag[i]] = lq;
	  YQT[i]=0x8000/lq;
	  if (cq < 1) cq = 1;
	  else if (cq > 255) cq = 255;
	  cqt[zigzag[i]] = cq;
	  CbCrQT[i]=0x8000/cq;
  }
}


static void FDCT (short int * data)
{
  unsigned short int i;
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

/* All values are shifted left by 10
 and rounded off to nearest integer */
  static const unsigned short int c1 = 1420; /* cos PI/16 * root(2)  */
  static const unsigned short int c2 = 1338; /* cos PI/8 * root(2)   */
  static const unsigned short int c3 = 1204; /* cos 3PI/16 * root(2) */
  static const unsigned short int c5 = 805; /* cos 5PI/16 * root(2) */
  static const unsigned short int c6 = 554; /* cos 3PI/8 * root(2)  */
  static const unsigned short int c7 = 283; /* cos 7PI/16 * root(2) */
  static const unsigned short int s1 = 3;
  static const unsigned short int s2 = 10;
  static const unsigned short int s3 = 13;
  for (i = 8; i > 0; i--)

    {
      x8 = data[0] + data[7];
      x0 = data[0] - data[7];
      x7 = data[1] + data[6];
      x1 = data[1] - data[6];
      x6 = data[2] + data[5];
      x2 = data[2] - data[5];
      x5 = data[3] + data[4];
      x3 = data[3] - data[4];
      x4 = x8 + x5;
      x8 -= x5;
      x5 = x7 + x6;
      x7 -= x6;
      data[0] = (short int) (x4 + x5);
      data[4] = (short int) (x4 - x5);
      data[2] = (short int) ((x8 * c2 + x7 * c6) >> s2);
      data[6] = (short int) ((x8 * c6 - x7 * c2) >> s2);
      data[7] = (short int) ((x0 * c7 - x1 * c5 + x2 * c3 - x3 * c1) >> s2);
      data[5] = (short int) ((x0 * c5 - x1 * c1 + x2 * c7 + x3 * c3) >> s2);
      data[3] = (short int) ((x0 * c3 - x1 * c7 - x2 * c1 - x3 * c5) >> s2);
      data[1] = (short int) ((x0 * c1 + x1 * c3 + x2 * c5 + x3 * c7) >> s2);
      data += 8;
    }
  data -= 64;
  for (i = 8; i > 0; i--)

    {
      x8 = data[0] + data[56];
      x0 = data[0] - data[56];
      x7 = data[8] + data[48];
      x1 = data[8] - data[48];
      x6 = data[16] + data[40];
      x2 = data[16] - data[40];
      x5 = data[24] + data[32];
      x3 = data[24] - data[32];
      x4 = x8 + x5;
      x8 -= x5;
      x5 = x7 + x6;
      x7 -= x6;
      data[0] = (short int) ((x4 + x5) >> s1);
      data[32] = (short int) ((x4 - x5) >> s1);
      data[16] = (short int) ((x8 * c2 + x7 * c6) >> s3);
      data[48] = (short int) ((x8 * c6 - x7 * c2) >> s3);
      data[56] = (short int) ((x0 * c7 - x1 * c5 + x2 * c3 - x3 * c1) >> s3);
      data[40] = (short int) ((x0 * c5 - x1 * c1 + x2 * c7 + x3 * c3) >> s3);
      data[24] = (short int) ((x0 * c3 - x1 * c7 - x2 * c1 - x3 * c5) >> s3);
      data[8] = (short int) ((x0 * c1 + x1 * c3 + x2 * c5 + x3 * c7) >> s3);
      data++;
    }
}

static void Quantize(short int *data , unsigned short int *table)
{
	int i,temp;
	for(i=0;i<64;i++)
	{
	    temp = data[i] * table[i];
	    temp = (temp + 0x4000) >> 15;
	    data[i] = (short int) temp;
	}
}

static void ZigzagScan(short int *data)
{
	short int temp[64];
	int i;
	for(i=0;i<64;i++)
		temp[zigzag[i]]=data[i];
	for(i=0;i<64;i++)
		data[i]=temp[i];

}

static void GetMCU(struct JpegEncBuffer *input, unsigned int xpos, unsigned int ypos, short int *data, unsigned char YCbCr)
{
	unsigned char x,y;
	unsigned int temp,i;
	unsigned short width;
	i=0;

	width=input->width;

	if((input->pixfmt==YUV422)&&(YCbCr>0))
	{
		width=input->width>>1;
		xpos=xpos>>1;
	}
	if((input->pixfmt==YUV420)&&(YCbCr>0))
	{
		width=input->width>>1;
		xpos=xpos>>1;
		ypos=ypos>>1;
	}
	if(YCbCr==0)
	{
		for(y=0;y<8;y++)
		{
			temp=(y+ypos)*width+xpos;
			for(x=0;x<8;x++)
			{
				data[i]=(short int)(input->Ybuf[temp+x]-128);
				i++;
			}
		}
	}
	else if(YCbCr==1)
	{
		for(y=0;y<8;y++)
		{
			temp=(y+ypos)*width+xpos;
			for(x=0;x<8;x++)
			{
				data[i]=(short int)(input->Cbbuf[temp+x]-128);
				i++;
			}
		}
	}
	else
	{
		for(y=0;y<8;y++)
		{
			temp=(y+ypos)*width+xpos;
			for(x=0;x<8;x++)
			{
				data[i]=(short int)(input->Crbuf[temp+x]-128);
				i++;
			}
		}
	}
}

static void JpegEncMCU(short int*data,unsigned short int *dqtable)
{
	FDCT(data);
	Quantize(data,dqtable);
	ZigzagScan (data);
}

unsigned int JpegEncPutBitsOffset=0;
static void JpegEncPutBits(struct JPEG *jpeg,struct BIT *bit)
{
	unsigned int index;
	unsigned char offset;
	unsigned char val;
	unsigned char l;
	unsigned char size=bit->size;
	unsigned short int code=bit->code;

	while(size>0)
	{
		index=JpegEncPutBitsOffset/8;
		offset=JpegEncPutBitsOffset%8;
		if(offset==0)
			val=0;
		else
			val=jpeg->ScanData[index];
		if(size<(8-offset))
			l=size;
		else
			l=8-offset;
		val|=(((code>>(size-l))&((1<<l)-1))<<(8-offset-l));
		jpeg->ScanData[index]=val;
		if(val==0xFF)
		{
			index++;
			JpegEncPutBitsOffset=JpegEncPutBitsOffset+8;
			jpeg->ScanData[index]=0;
		}
		size=size-l;
		JpegEncPutBitsOffset=JpegEncPutBitsOffset+l;
	}
}

static void JpegEncStopBits(struct JPEG *jpeg)
{
	unsigned int index;
	unsigned char offset;
	unsigned char val;
	index=JpegEncPutBitsOffset/8;
	offset=JpegEncPutBitsOffset%8;
	if(offset>0)
	{
		val=jpeg->ScanData[index];
		val=val|((1<<(8-offset))-1);
		jpeg->ScanData[index]=val;
		index=index+1;
	}
	jpeg->ScanSize=index;
	printf("Jpeg Enc Size = %d\r\n", jpeg->ScanSize);
}

static void JpegEncDataToVLI(struct BIT *bit,short int data)
{
	short int temp;
	char size;

	size=16;
	temp=data;
	if(temp==0)
	{
		bit->code=0;
		bit->size=0;
		return ;
	}
	if(temp<0) temp=-temp;
	while(size>=0)
	{
		if(temp&(1<<size))
			break;
		size--;
	}
	bit->size=(unsigned char)size+1;
	if(data>0)
		bit->code=data;
	else
	{
		bit->code=(~temp)&((1<<bit->size)-1);
	}
}

#ifdef SPEEDFIRST
static void JpegEncHuffman(struct JPEG *jpeg,short int *data, short int diff, struct BIT *DC,struct BIT *AC)
{
	struct BIT *bit;
	unsigned char i,runlength;
	unsigned char zero;
	bit=vliptr+diff;
	JpegEncPutBits(jpeg,&DC[bit->size]);
	JpegEncPutBits(jpeg,bit);

	for(i=63;((i>0)&&(data[i]==0));i--);
	zero=63-i;
	if(zero==63)
	{
		JpegEncPutBits(jpeg,&AC[0]);
		return ;
	}
	runlength=0;
	for(i=1;i<64-zero;i++)
	{
		if(data[i])
		{
			bit=vliptr+data[i];
			JpegEncPutBits(jpeg,&AC[((runlength<<4)|(bit->size))]);
			JpegEncPutBits(jpeg,bit);
			runlength=0;
		}
		else
		{
			runlength++;
			if(runlength==16)
			{
				JpegEncPutBits(jpeg,&AC[0xF0]);
				runlength=0;
			}
		}
	}
	if(i<64)
		JpegEncPutBits(jpeg,&AC[0]);
}
#else
static void JpegEncHuffman(struct JPEG *jpeg,short int *data, short int diff, struct BIT *DC,struct BIT *AC)
{
	struct BIT bit;
	unsigned char i,runlength;
	unsigned char zero;
	JpegEncDataToVLI(&bit,diff);
	JpegEncPutBits(jpeg,&DC[bit.size]);
	JpegEncPutBits(jpeg,&bit);

	for(i=63;((i>0)&&(data[i]==0));i--);
	zero=63-i;
	if(zero==63)
	{
		JpegEncPutBits(jpeg,&AC[0]);
		return ;
	}
	runlength=0;
	for(i=1;i<64-zero;i++)
	{
		if(data[i])
		{
			JpegEncDataToVLI(&bit,data[i]);
			JpegEncPutBits(jpeg,&AC[((runlength<<4)|(bit.size))]);
			JpegEncPutBits(jpeg,&bit);
			runlength=0;
		}
		else
		{
			runlength++;
			if(runlength==16)
			{
				JpegEncPutBits(jpeg,&AC[0xF0]);
				runlength=0;
			}
		}
	}
	if(i<64)
		JpegEncPutBits(jpeg,&AC[0]);
}
#endif


static enum JPEGRTPSTATUS JpegEncJpegHeadInit(struct JPEG *jpeg, struct JpegEncBuffer *input)
{
	if((input->width%16)||(input->height%16))		return IMAGESIZEERROR;
	QuantizeTableInit(input->Q, (unsigned char *)jpeg->dqtable.table[0],(unsigned char *)jpeg->dqtable.table[1]);
	if(0){
		int i;
		for(i = 0; i < 64; i++){
			((unsigned char *)jpeg->dqtable.table[0])[i] = 1;
			((unsigned char *)jpeg->dqtable.table[1])[i] = 1;
		}
	}
	jpeg->Q=input->Q;
	jpeg->dqtable.count=2;

	jpeg->width=input->width;
	jpeg->height=input->height;
	jpeg->pixfmt=input->pixfmt;
	jpeg->RSTCount=0;

	jpeg->ScanData=(unsigned char*)malloc(MAXSCANSIZE);
	if(jpeg->ScanData==NULL)
		return MALLOCERROR;
	jpeg->ScanSize=0;
	return JPEGRTPSTATUSOK;
}

//初始化jpec编码器，使用编码器前必须调用
void JpegEncInit(void)
{
	JpegEncPutBitsOffset=0;
#ifdef SPEEDFIRST
	short int i;
	for(i=-32768;i<32767;i++)
	{
		JpegEncDataToVLI(vliptr+i,i);
	}
#endif
}

//开始编码
//jpeg为编码结果存放位置，不需要配置
//input为编码所需数据，所有结构都需要配置好
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegEncCode(struct JPEG *jpeg, struct JpegEncBuffer *input)
{
	unsigned int x,y;
	short int data[64];
	short int DY, DCb, DCr;
	enum JPEGRTPSTATUS ret;
	JpegEncPutBitsOffset=0;
	DY=0;DCb=0;DCr=0;

	ret=JpegEncJpegHeadInit(jpeg, input);
	if(ret!=JPEGRTPSTATUSOK)
	{
		return ret;
	}


	for(y=0;y<input->height;y=y+8)
	{
		for(x=0;x<input->width;x=x+8)
		{
			if(input->pixfmt==YUV444)
			{
				//Y
				GetMCU(input,x,y,data,0);
				JpegEncMCU(data,YQT);
				JpegEncHuffman(jpeg,data,data[0]-DY,s_lumin_dc,s_lumin_ac);
				DY=data[0];
				//Cb
				GetMCU(input,x,y,data,1);
				JpegEncMCU(data,CbCrQT);
				JpegEncHuffman(jpeg,data,data[0]-DCb,s_chrom_dc,s_chrom_ac);
				DCb=data[0];
				//Cr
				GetMCU(input,x,y,data,2);
				JpegEncMCU(data,CbCrQT);
				JpegEncHuffman(jpeg,data,data[0]-DCr,s_chrom_dc,s_chrom_ac);
				DCr=data[0];
			}
			else if(input->pixfmt==YUV422)
			{
				//Y
				GetMCU(input,x,y,data,0);
				JpegEncMCU(data,YQT);
				JpegEncHuffman(jpeg,data,data[0]-DY,s_lumin_dc,s_lumin_ac);
				DY=data[0];
				//Y
				GetMCU(input,x+8,y,data,0);
				JpegEncMCU(data,YQT);
				JpegEncHuffman(jpeg,data,data[0]-DY,s_lumin_dc,s_lumin_ac);
				DY=data[0];
				//Cb
				GetMCU(input,x,y,data,1);
				JpegEncMCU(data,CbCrQT);
				JpegEncHuffman(jpeg,data,data[0]-DCb,s_chrom_dc,s_chrom_ac);
				DCb=data[0];
				//Cr
				GetMCU(input,x,y,data,2);
				JpegEncMCU(data,CbCrQT);
				JpegEncHuffman(jpeg,data,data[0]-DCr,s_chrom_dc,s_chrom_ac);
				DCr=data[0];
				x+=8;
			}
			else
			{
				//Y
				GetMCU(input,x,y,data,0);
				JpegEncMCU(data,YQT);
				JpegEncHuffman(jpeg,data,data[0]-DY,s_lumin_dc,s_lumin_ac);
				DY=data[0];
				//Y
				GetMCU(input,x+8,y,data,0);
				JpegEncMCU(data,YQT);
				JpegEncHuffman(jpeg,data,data[0]-DY,s_lumin_dc,s_lumin_ac);
				DY=data[0];
				//Y
				GetMCU(input,x,y+8,data,0);
				JpegEncMCU(data,YQT);
				JpegEncHuffman(jpeg,data,data[0]-DY,s_lumin_dc,s_lumin_ac);
				DY=data[0];
				//Y
				GetMCU(input,x+8,y+8,data,0);
				JpegEncMCU(data,YQT);
				JpegEncHuffman(jpeg,data,data[0]-DY,s_lumin_dc,s_lumin_ac);
				DY=data[0];
				//Cb
				GetMCU(input,x,y,data,1);
				JpegEncMCU(data,CbCrQT);
				JpegEncHuffman(jpeg,data,data[0]-DCb,s_chrom_dc,s_chrom_ac);
				DCb=data[0];
				//Cr
				GetMCU(input,x,y,data,2);
				JpegEncMCU(data,CbCrQT);
				JpegEncHuffman(jpeg,data,data[0]-DCr,s_chrom_dc,s_chrom_ac);
				DCr=data[0];
				x+=8;
			}
		}
		if(input->pixfmt==YUV420) y+=8;
	}
	JpegEncStopBits(jpeg);
	return JPEGRTPSTATUSOK;
}

//初始化input
//input为需要初始化的
//Q量化系数 1~99
//width ,height 宽高，必须为16的倍数
//pixfmt 颜色格式
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegEncBufferInit(struct JpegEncBuffer *input, unsigned char Q,unsigned short int width,unsigned short int height, enum PixFmt pixfmt)
{
	if((width%16)||(height%16))		return IMAGESIZEERROR;
	input->Q=Q;
	input->width=width;
	input->height=height;
	input->pixfmt=pixfmt;
	return JPEGRTPSTATUSOK;
}

//同JpegEncBufferInit，但将为YCbCr各分量申请内存，用完使用JpegEncBufferDestroy释放
//input为需要初始化的
//Q量化系数 1~99
//width ,height 宽高，必须为16的倍数
//pixfmt 颜色格式
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegEncBufferCreate(struct JpegEncBuffer *input, unsigned char Q,unsigned short int width,unsigned short int height, enum PixFmt pixfmt)
{
	if((width%16)||(height%16))		return IMAGESIZEERROR;
	input->Q=Q;
	input->width=width;
	input->height=height;
	input->pixfmt=pixfmt;

	if(pixfmt==YUV444)
	{
		input->Ybuf=(unsigned char*)malloc(width*height*3);
		if(input->Ybuf==NULL) return MALLOCERROR;
		input->Cbbuf=input->Ybuf+width*height;
		input->Crbuf=input->Cbbuf+width*height;
	}
	else if(pixfmt==YUV422)
	{
		input->Ybuf=(unsigned char*)malloc(width*height<<1);
		if(input->Ybuf==NULL) return MALLOCERROR;
		input->Cbbuf=input->Ybuf+width*height;
		input->Crbuf=input->Cbbuf+((width*height)>>1);
	}
	else if(pixfmt==YUV420)
	{
		input->Ybuf=(unsigned char*)malloc(width*height*3>>1);
		if(input->Ybuf==NULL) return MALLOCERROR;
		input->Cbbuf=input->Ybuf+width*height;
		input->Crbuf=input->Cbbuf+((width*height)>>2);
	}
	else
	{
		return PIXFMTERROR;
	}
	return JPEGRTPSTATUSOK;
}

//释放JpegEncBufferCreate申请的内存
//input为需要释放的内存所在结构体
//返回JPEGRTPSTATUSOK成功
enum JPEGRTPSTATUS JpegEncBufferDestroy(struct JpegEncBuffer *input)
{
	if(input->Ybuf!=NULL)
	{
		free(input->Ybuf);
		input->Ybuf=NULL;
	}
	return JPEGRTPSTATUSOK;
}

//测试jpeg编码器
//file保存的文件名
int JpegEncTest(const char *file)
{
	int ret;
	struct JPEG jpeg;
	struct JpegEncBuffer input;

	unsigned char image[3*640*480];

	int i;

	for(i=0;i<3*640*480;i=i+3)
		image[i]=0xFF;

	JpegEncInit();
	ret=JpegEncBufferCreate(&input,99,640,480,YUV444);

	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegEncBufferInitError: %d\n",ret);
		goto out;
	}
	switch(input.pixfmt)
	{
	case YUV444:
		RGB24toYUV444(image, &input);break;
	case YUV422:
		RGB24toYUV422(image, &input);break;
	case YUV420:
		RGB24toYUV420(image, &input);break;
	default:goto out;break;
	}
	ret=JpegEncCode(&jpeg,&input);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegEncCodeError: %d\n",ret);
		goto out;
	}

	ret=JpegInfoPutToFile(&jpeg, file);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegInfoPutToFileError: %d\n",ret);
		goto out;
	}
out:
	JpegEncBufferDestroy(&input);
	JpegInfoDestroy(&jpeg);
	return ret;
}
