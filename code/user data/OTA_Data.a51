;/*****************************************************************************
;**               AMICCOM Electronics Corporation Document                   **
;**          Copyright (c) 2011-2014 AMICCOM Electronics Corporation         **
;**                                                                          **
;**      A3,1F,No.1, Li-Hsin 1th Road, Science_Based Industrid Park,         **
;**                       Hsin_Chu City, 300, Taiwan, ROC.                   **
;**                 Tel: 886-3-5785818   Fax: 886-3-5785819                  **
;**         E-mail:info@amiccom.com.tw  http: //www.amiccom.com.tw           **
;*****************************************************************************/

NAME    OTA_DATA

CSEG AT 0x7FF4

;================================================ User Modify =================================================

    ; Customer Version (4 Byte)
        DB  0x30, 0x31, 0x32, 0x33

    ; Product Version (4 Byte)
        DB  0x40, 0x41, 0x42, 0x43       

    ; Firmware Version (4 Byte)
        DB  0x00, 0x00, 0x00, 0x00 

;================================================= Modify END =================================================
       
END