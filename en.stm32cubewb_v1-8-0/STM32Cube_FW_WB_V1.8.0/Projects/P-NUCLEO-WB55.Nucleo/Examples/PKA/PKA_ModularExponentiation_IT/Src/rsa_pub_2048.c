/**
  ******************************************************************************
  * @file    PKA/PKA_ModularExponentiation_IT/Src/rsa_pub_2048.c
  * @author  MCD Application Team
  * @brief   This file contains buffers reflecting the content of the file
  *          rsa_pub_2048.pem provided as an example of public key to 
  *          demonstrate functionnality of PKA.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics. 
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the 
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* 
This file reflect the content of rsa_pub_2048.pem

rsa_pub_2048.pem has been created using openssl 1.0.2k  26 Jan 2017:
  openssl rsa -in rsa_priv_2048.pem -out rsa_pub_2048.pem -outform PEM -pubout
  
Buffer have been adapted from the output of the command: 
  openssl rsa -pubin -in rsa_public.pem -text
  
Public-Key: (2048 bit)
Modulus: (RSA modulus. Discard the first 00)
    00:d7:59:df:92:03:12:5a:8b:ea:06:e2:6d:20:cf:
    50:4f:45:e6:3d:04:7e:0b:ed:8b:d7:3d:74:a1:2e:
    a7:b6:2a:dc:a4:22:bb:ac:c8:e7:61:2c:5a:bb:01:
    0d:a0:66:94:53:e9:01:cf:3f:b3:03:05:31:00:77:
    bc:b8:1d:1e:45:e4:2b:95:f1:35:a5:80:c8:eb:3e:
    5d:5f:93:70:6e:5f:ad:2c:34:f0:b2:d5:ec:31:19:
    3e:5f:c7:c1:06:9f:91:30:ea:2b:be:23:ed:71:b7:
    3f:04:d2:b5:33:8d:5d:48:84:39:59:c9:d4:98:bb:
    7a:5c:d1:90:58:07:68:fa:ec:02:92:5a:e3:97:6d:
    ca:39:7c:80:34:a5:51:b8:3d:15:7a:82:85:02:54:
    ab:6d:55:6a:fd:06:f5:46:38:2e:70:3e:63:19:72:
    f1:a3:a4:8e:a0:f1:45:a1:6f:79:d8:ff:3c:5b:51:
    83:06:3b:11:2c:95:e3:12:4f:39:3a:c9:12:3c:39:
    7b:5c:af:34:58:c4:17:57:f1:7f:77:e0:94:6a:57:
    16:47:64:ea:7e:d8:d3:95:5b:e4:7e:93:9e:ef:47:
    8c:0b:2e:3a:7f:79:4e:c8:a7:5a:b8:41:d4:a8:9b:
    de:52:f7:53:d3:a3:6e:23:bb:c4:10:4f:32:9a:03:
    3c:31
Exponent: 65537 (0x10001)

Remark: 00 at beginning of buffers are removed to ease the usage with PKA 

*/

#include "main.h"

const uint8_t rsa_pub_2048_modulus[] = {
/*0x00,*/ 0xd7, 0x59, 0xdf, 0x92, 0x03, 0x12, 0x5a, 0x8b, 0xea, 0x06, 0xe2, 0x6d, 0x20, 0xcf,
    0x50, 0x4f, 0x45, 0xe6, 0x3d, 0x04, 0x7e, 0x0b, 0xed, 0x8b, 0xd7, 0x3d, 0x74, 0xa1, 0x2e,
    0xa7, 0xb6, 0x2a, 0xdc, 0xa4, 0x22, 0xbb, 0xac, 0xc8, 0xe7, 0x61, 0x2c, 0x5a, 0xbb, 0x01,
    0x0d, 0xa0, 0x66, 0x94, 0x53, 0xe9, 0x01, 0xcf, 0x3f, 0xb3, 0x03, 0x05, 0x31, 0x00, 0x77,
    0xbc, 0xb8, 0x1d, 0x1e, 0x45, 0xe4, 0x2b, 0x95, 0xf1, 0x35, 0xa5, 0x80, 0xc8, 0xeb, 0x3e,
    0x5d, 0x5f, 0x93, 0x70, 0x6e, 0x5f, 0xad, 0x2c, 0x34, 0xf0, 0xb2, 0xd5, 0xec, 0x31, 0x19,
    0x3e, 0x5f, 0xc7, 0xc1, 0x06, 0x9f, 0x91, 0x30, 0xea, 0x2b, 0xbe, 0x23, 0xed, 0x71, 0xb7,
    0x3f, 0x04, 0xd2, 0xb5, 0x33, 0x8d, 0x5d, 0x48, 0x84, 0x39, 0x59, 0xc9, 0xd4, 0x98, 0xbb,
    0x7a, 0x5c, 0xd1, 0x90, 0x58, 0x07, 0x68, 0xfa, 0xec, 0x02, 0x92, 0x5a, 0xe3, 0x97, 0x6d,
    0xca, 0x39, 0x7c, 0x80, 0x34, 0xa5, 0x51, 0xb8, 0x3d, 0x15, 0x7a, 0x82, 0x85, 0x02, 0x54,
    0xab, 0x6d, 0x55, 0x6a, 0xfd, 0x06, 0xf5, 0x46, 0x38, 0x2e, 0x70, 0x3e, 0x63, 0x19, 0x72,
    0xf1, 0xa3, 0xa4, 0x8e, 0xa0, 0xf1, 0x45, 0xa1, 0x6f, 0x79, 0xd8, 0xff, 0x3c, 0x5b, 0x51,
    0x83, 0x06, 0x3b, 0x11, 0x2c, 0x95, 0xe3, 0x12, 0x4f, 0x39, 0x3a, 0xc9, 0x12, 0x3c, 0x39,
    0x7b, 0x5c, 0xaf, 0x34, 0x58, 0xc4, 0x17, 0x57, 0xf1, 0x7f, 0x77, 0xe0, 0x94, 0x6a, 0x57,
    0x16, 0x47, 0x64, 0xea, 0x7e, 0xd8, 0xd3, 0x95, 0x5b, 0xe4, 0x7e, 0x93, 0x9e, 0xef, 0x47,
    0x8c, 0x0b, 0x2e, 0x3a, 0x7f, 0x79, 0x4e, 0xc8, 0xa7, 0x5a, 0xb8, 0x41, 0xd4, 0xa8, 0x9b,
    0xde, 0x52, 0xf7, 0x53, 0xd3, 0xa3, 0x6e, 0x23, 0xbb, 0xc4, 0x10, 0x4f, 0x32, 0x9a, 0x03,
    0x3c, 0x31
};
const uint32_t rsa_pub_2048_modulus_len = 256;

const uint8_t rsa_pub_2048_publicExponent[] = {0x00, 0x1, 0x00, 0x01};/* 65537; *//*(0x10001)*/

const uint32_t rsa_pub_2048_publicExponent_len = 4;
  

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
