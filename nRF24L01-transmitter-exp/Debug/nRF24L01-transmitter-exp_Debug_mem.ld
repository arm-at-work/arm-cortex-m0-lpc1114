/*
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-2015
 * (c) NXP Semiconductors 2013-2015
 * Linker script memory definitions
 * Created from LinkMemoryTemplate
 * By LPCXpresso v7.5.0 [Build 254] [2014-10-31]  on Tue Apr 07 17:35:59 BRT 2015)
*/

EMORY
{
  /* Define each memory region */
  MFlash32 (rx) : ORIGIN = 0x0, LENGTH = 0x8000 /* 32K bytes */
  RamLoc8 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x2000 /* 8K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_MFlash32 = 0x0 + 0x8000;
  __top_RamLoc8 = 0x10000000 + 0x2000;



